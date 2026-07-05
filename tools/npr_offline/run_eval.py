"""Neural NPR オフライン検証ハーネス（評価優先・学習なし）のエントリポイント。

ゲームがダンプした連番 PNG（dump/color_%04d.png）を読み、各フレームをスタイル変換し、
「素のフレーム単位スタイル化のちらつき」と「warp+時間ブレンドで安定化した結果のちらつき」を
比較して、neural NPR がこのコンテンツで時間的に成立しうるかの一次判断を出す。

使い方:
  スモーク:  python run_eval.py --frames <dump> --stylizer classic --out out
  本番:      python run_eval.py --frames <dump> --stylizer neural --style style.jpg --out out
"""

import argparse
import json
import os
import statistics
import time

import numpy as np

import io_utils
import temporal as T
import gbuffer as G


def build_stylizer(name: str, args):
    if name == "classic":
        from stylizers.classic_npr import ClassicNprStylizer
        return ClassicNprStylizer()
    if name == "pencil":
        from stylizers.pencil_sketch import PencilSketchStylizer
        return PencilSketchStylizer()
    if name == "neural":
        if not args.style:
            raise SystemExit("--style <image> は --stylizer neural に必須です")
        from stylizers.neural_style import NeuralStyleStylizer
        return NeuralStyleStylizer(args.style, steps=args.steps, max_size=args.max_size,
                                   temporal_weight=args.temporal_weight,
                                   style_weight=args.style_weight,
                                   content_weight=args.content_weight,
                                   tv_weight=args.tv_weight)
    raise SystemExit(f"unknown stylizer: {name}")


def main():
    ap = argparse.ArgumentParser(description="Neural NPR offline temporal-stability validation")
    ap.add_argument("--frames", required=True, help="ダンプ連番 PNG のあるディレクトリ")
    ap.add_argument("--stylizer", default="classic", choices=["classic", "pencil", "neural"])
    ap.add_argument("--style", default=None, help="neural 用のスタイル画像")
    ap.add_argument("--alpha", type=float, default=0.6, help="時間ブレンド係数 (0..1)")
    ap.add_argument("--max-frames", type=int, default=None)
    ap.add_argument("--steps", type=int, default=150, help="neural の最適化反復数")
    ap.add_argument("--max-size", type=int, default=384, help="neural の処理解像度上限")
    ap.add_argument("--warm-start", action="store_true",
                    help="neural: 前フレーム出力を現フレームへ warp して最適化の初期値にする（旧案A・ボケやすい）")
    ap.add_argument("--temporal-weight", type=float, default=0.0,
                    help="neural: 損失に前フレーム整合項を加える重み（案A改・鮮明さを保つ）。0 で無効")
    ap.add_argument("--style-weight", type=float, default=1e6,
                    help="neural: 画風（スタイル画像）に寄せる重み。大きいほど画風が強く出る（既定 1e6）")
    ap.add_argument("--content-weight", type=float, default=1.0,
                    help="neural: 内容（元レンダ）に寄せる重み。大きいほど元の形が残る（既定 1.0）")
    ap.add_argument("--tv-weight", type=float, default=0.0,
                    help="neural: Total Variation 正則化の重み。隣接ピクセル差を罰し高周波ノイズ（崩壊）を抑える。"
                         "高い style-weight でザラつくとき有効（例 1.0〜30）。0 で無効")
    ap.add_argument("--boil", type=float, default=0.0,
                    help="neural: ボイリング量。各フレーム初期値にフレーム固有ノイズを注入し画面全体（背景含む）を"
                         "毎フレーム揺らす。手描きアニメ風。大きいほど揺れる（例 0.05〜0.3）。0 で無効")
    ap.add_argument("--reject-sigma", type=float, default=0.0,
                    help="gbuffer 安定化: 残像リジェクションの感度。warp した前フレームと現フレームが"
                         "食い違う画素ほど α を自動で下げ、大きく変化した領域の残像を消す（例 0.05〜0.2）。0 で無効")
    ap.add_argument("--reject-power", type=float, default=2.0,
                    help="案1: 残像リジェクションの減衰カーブ指数。2=なだらか, 4〜6=崖型"
                         "（中程度の変化は守り残像級だけ弾く）")
    ap.add_argument("--warp-reject", type=float, default=0.0,
                    help="案2: warp 整合マスク。warp 前フレームと現フレームの食い違いがこの閾値を超える"
                         "画素を安定化対象から外し今フレームで描き直す（残像の真因＝対応無し領域を直接落とす）。0 で無効")
    ap.add_argument("--occ-feather", type=float, default=0.0,
                    help="gbuffer 遮蔽マスクをぼかす量(px)。安定化の「明滅する境界帯」と静かな領域の"
                         "硬い継ぎ目（境界ノイズ）をなめらかにする（例 5〜15）。0 で無効")
    ap.add_argument("--coherent-boil", type=float, default=0.0,
                    help="コヒーレント・ボイリングの振幅。最適化初期値に空間/時間的になめらかな揺れ場を注入し、"
                         "筆致のかたまりがゆっくりウネる手描き感を出す（白色ノイズと違いノイズに見えにくい）。0 で無効（例 0.02〜0.08）")
    ap.add_argument("--coherent-space", type=float, default=12.0,
                    help="コヒーレント・ボイリングの空間スケール(px)。大きいほど大きなかたまり単位で揺れる")
    ap.add_argument("--coherent-time", type=int, default=6,
                    help="コヒーレント・ボイリングの時間キーフレーム間隔(フレーム)。大きいほどゆっくり変化")
    ap.add_argument("--post-boil", type=float, default=0.0,
                    help="安定化後に乗せる筆致ゆらぎの振幅(px)。各フレームを微小ワープして手描きの揺れを"
                         "一律に出す（warp と干渉せず残像を生まない）。0 で無効（例 0.5〜3）")
    ap.add_argument("--post-boil-scale", type=float, default=24.0,
                    help="post-boil の変位場スケール(px)。大きいほどゆったり、小さいほど細かく震える")
    ap.add_argument("--no-stabilize", action="store_true",
                    help="warp+ブレンド安定化を行わず「入力｜スタイル化」の2列だけ出す。"
                         "ボイリング路線（安定化は不要・むしろ境界ゴーストが有害）向け")
    ap.add_argument("--flow", default="farneback", choices=["farneback", "gbuffer"],
                    help="フロー源: farneback(推定) / gbuffer(エンジンのモーションベクトル, 案B/段階0b)")
    ap.add_argument("--proc-size", type=int, default=720,
                    help="読み込み時の最長辺上限（メモリ/フロー計算量の制御）")
    ap.add_argument("--fps", type=int, default=24)
    ap.add_argument("--out", default="tools/npr_offline/out")
    args = ap.parse_args()

    io_utils.ensure_dir(args.out)

    frames = io_utils.load_frames(args.frames, max_frames=args.max_frames, proc_size=args.proc_size)
    if len(frames) < 2:
        raise SystemExit(f"2 フレーム以上必要です（{args.frames} に {len(frames)} 枚）")
    print(f"loaded {len(frames)} frames {frames[0].shape}")

    stylizer = build_stylizer(args.stylizer, args)

    grays = [T.to_gray(f) for f in frames]

    # フロー源: gbuffer なら motion_*.raw からエンジンのモーションベクトルを読む（案B / 段階0b）
    gflows = gmasks = None
    use_gbuffer = args.flow == "gbuffer"
    if use_gbuffer:
        loaded = G.load_flows_and_masks(args.frames, [f.shape for f in frames], feather=args.occ_feather)
        if loaded is None:
            raise SystemExit(f"--flow gbuffer だが {args.frames} に motion_*.raw が見つかりません")
        gflows, gmasks = loaded
        print(f"flow: gbuffer（エンジンのモーションベクトル, {len(gflows)} frames）")
    else:
        print("flow: farneback（推定光学フロー）")

    warm = args.warm_start
    temporal = args.temporal_weight > 0.0
    if (warm or temporal) and args.stylizer != "neural":
        print(f"warning: --warm-start / --temporal-weight は neural 専用のため {args.stylizer} では無視されます")
        warm = temporal = False
    if warm:
        print("warm-start: 前フレーム出力を warp して最適化の初期値に使用（旧案A）")
    if temporal:
        print(f"temporal-loss: 損失に前フレーム整合項を追加（案A改, weight={args.temporal_weight}）")

    # コヒーレント・ボイリング用の揺れ場を全フレーム分事前生成（空間/時間なめらか）
    cfields = [None] * len(frames)
    if args.stylizer == "neural" and args.coherent_boil > 0.0:
        H0, W0 = frames[0].shape[:2]
        cfields = T.coherent_boil_fields(len(frames), H0, W0, args.coherent_boil,
                                         args.coherent_space, args.coherent_time)
        print(f"coherent-boil: amp={args.coherent_boil} space={args.coherent_space} time_k={args.coherent_time}")

    t0 = time.time()
    naive = []
    prev_out = None
    for i, f in enumerate(frames):
        init = None
        tgt = None
        mask = None
        if prev_out is not None and (warm or temporal):
            # 前フレーム出力を現フレーム座標へ整列（dst=現, src=前 のフロー）
            if use_gbuffer:
                flow = gflows[i]
                warped_prev = T.warp(prev_out, flow)
                occ = gmasks[i]
            else:
                flow = T.compute_flow(grays[i - 1], grays[i])
                warped_prev = T.warp(prev_out, flow)
                occ = T.occlusion_mask(grays[i - 1], grays[i], flow)
            if warm:
                init = warped_prev
            if temporal:
                tgt = warped_prev
                mask = occ
        extra = {}
        if args.stylizer == "neural" and not (warm or temporal):
            if cfields[i] is not None:
                extra = {"jitter_field": cfields[i]}      # コヒーレント・ボイリング優先
            elif args.boil > 0.0:
                # 旧ボイリング: フレーム index をシードにして毎フレーム異なる白色ノイズ（再現性あり）
                extra = {"jitter": args.boil, "jitter_seed": i}
        out = stylizer.stylize(f, init=init, temporal_target=tgt, temporal_mask=mask, **extra)
        naive.append(out)
        prev_out = out
        print(f"  stylize {i + 1}/{len(frames)}", end="\r")
    print(f"\nstylized in {time.time() - t0:.1f}s")

    stabilize_on = not args.no_stabilize
    if not stabilize_on:
        # ボイリング路線: 安定化なし。ちらつき量は参考に測るが warp はしない。
        err_naive = T.temporal_errors(naive, grays)
        stable = None
        err_stable = None
    elif use_gbuffer:
        err_naive = T.temporal_errors_pre(naive, gflows, gmasks)
        stable = T.stabilize_pre(naive, gflows, gmasks, alpha=args.alpha, reject_sigma=args.reject_sigma,
                                 reject_power=args.reject_power, warp_reject=args.warp_reject)
        err_stable = T.temporal_errors_pre(stable, gflows, gmasks)
    else:
        err_naive = T.temporal_errors(naive, grays)
        stable = T.stabilize(naive, grays, alpha=args.alpha)
        err_stable = T.temporal_errors(stable, grays)

    # 安定化後の筆致ゆらぎ（一律ボイリング）。残像を消した土台の上に手描きの揺れを後乗せする。
    if stable is not None and args.post_boil > 0.0:
        stable = T.post_boil(stable, amp=args.post_boil, scale=args.post_boil_scale)

    m_naive = statistics.mean(err_naive)
    m_stable = statistics.mean(err_stable) if err_stable is not None else None
    reduction = (1.0 - m_stable / m_naive) * 100.0 if (m_stable is not None and m_naive > 0) else 0.0
    sharp_naive = T.sharpness(naive)
    sharp_stable = T.sharpness(stable) if stable is not None else None
    # 累積ボケ検出: 前半と後半の鮮明さ。warm-start は warp 補間が積み上がり後半ほど低下する。
    half = len(naive) // 2
    sharp_first = T.sharpness(naive[:half]) if half else sharp_naive
    sharp_last = T.sharpness(naive[half:])
    sharp_decay = (1.0 - sharp_last / sharp_first) * 100.0 if sharp_first > 0 else 0.0

    if stable is not None:
        comparison = [np.concatenate([f, n, s], axis=1) for f, n, s in zip(frames, naive, stable)]
    else:
        comparison = [np.concatenate([f, n], axis=1) for f, n in zip(frames, naive)]
    io_utils.save_video(os.path.join(args.out, "comparison.mp4"), comparison, fps=args.fps)
    for idx in sorted({0, len(frames) // 2, len(frames) - 1}):
        io_utils.save_image(os.path.join(args.out, f"sample_{idx:04d}.png"), comparison[idx])

    metrics = {
        "frames": len(frames),
        "stylizer": args.stylizer,
        "flow": args.flow,
        "warm_start": warm,
        "temporal_weight": args.temporal_weight if temporal else 0.0,
        "style_weight": args.style_weight,
        "content_weight": args.content_weight,
        "tv_weight": args.tv_weight,
        "boil": args.boil,
        "occ_feather": args.occ_feather,
        "reject_sigma": args.reject_sigma,
        "reject_power": args.reject_power,
        "warp_reject": args.warp_reject,
        "post_boil": args.post_boil,
        "post_boil_scale": args.post_boil_scale,
        "coherent_boil": args.coherent_boil,
        "coherent_space": args.coherent_space,
        "coherent_time": args.coherent_time,
        "alpha": args.alpha,
        "temporal_error_naive_mean": m_naive,
        "temporal_error_stable_mean": m_stable,
        "reduction_percent": reduction,
        "sharpness_naive": sharp_naive,
        "sharpness_stable": sharp_stable,
        "sharpness_naive_first_half": sharp_first,
        "sharpness_naive_last_half": sharp_last,
        "sharpness_decay_percent": sharp_decay,
        "per_frame_naive": err_naive,
        "per_frame_stable": err_stable,
    }
    with open(os.path.join(args.out, "metrics.json"), "w", encoding="utf-8") as fp:
        json.dump(metrics, fp, ensure_ascii=False, indent=2)

    if m_stable is not None:
        print(f"temporal error  naive={m_naive:.5f}  stable={m_stable:.5f}  reduction={reduction:.1f}%")
        print(f"sharpness       naive={sharp_naive:.1f}   stable={sharp_stable:.1f}  (高いほど細部が残る)")
    else:
        print(f"temporal error  naive={m_naive:.5f}  (安定化なし / ボイリング量の目安)")
        print(f"sharpness       naive={sharp_naive:.1f}  (高いほど細部が残る)")
    print(f"sharpness decay first={sharp_first:.1f} last={sharp_last:.1f} decay={sharp_decay:.1f}%  (正に大きい=後半ほどボケ=累積)")
    layout = "[input|naive]" if stable is None else "[input|naive|stable]"
    print(f"out -> {args.out} (comparison.mp4 {layout}, sample_*.png, metrics.json)")


if __name__ == "__main__":
    main()

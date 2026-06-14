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
        return NeuralStyleStylizer(args.style, steps=args.steps, max_size=args.max_size)
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

    t0 = time.time()
    naive = []
    for i, f in enumerate(frames):
        naive.append(stylizer.stylize(f))
        print(f"  stylize {i + 1}/{len(frames)}", end="\r")
    print(f"\nstylized in {time.time() - t0:.1f}s")

    grays = [T.to_gray(f) for f in frames]
    err_naive = T.temporal_errors(naive, grays)
    stable = T.stabilize(naive, grays, alpha=args.alpha)
    err_stable = T.temporal_errors(stable, grays)

    m_naive = statistics.mean(err_naive)
    m_stable = statistics.mean(err_stable)
    reduction = (1.0 - m_stable / m_naive) * 100.0 if m_naive > 0 else 0.0

    comparison = [np.concatenate([f, n, s], axis=1) for f, n, s in zip(frames, naive, stable)]
    io_utils.save_video(os.path.join(args.out, "comparison.mp4"), comparison, fps=args.fps)
    for idx in sorted({0, len(frames) // 2, len(frames) - 1}):
        io_utils.save_image(os.path.join(args.out, f"sample_{idx:04d}.png"), comparison[idx])

    metrics = {
        "frames": len(frames),
        "stylizer": args.stylizer,
        "alpha": args.alpha,
        "temporal_error_naive_mean": m_naive,
        "temporal_error_stable_mean": m_stable,
        "reduction_percent": reduction,
        "per_frame_naive": err_naive,
        "per_frame_stable": err_stable,
    }
    with open(os.path.join(args.out, "metrics.json"), "w", encoding="utf-8") as fp:
        json.dump(metrics, fp, ensure_ascii=False, indent=2)

    print(f"temporal error  naive={m_naive:.5f}  stable={m_stable:.5f}  reduction={reduction:.1f}%")
    print(f"out -> {args.out} (comparison.mp4 [input|naive|stable], sample_*.png, metrics.json)")


if __name__ == "__main__":
    main()

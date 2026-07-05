"""学習済み順伝播ネットでダンプフレームを推論し、見た目を検証する（フェーズ1検証）。

Gatys 法（150ステップ最適化）の naive 出力と、このネット（forward 一発）の出力を比べ、
「順伝播で確定の見た目を再現できているか」を確認する。リアルタイム化の前提が成り立つかの判断。

使い方:
  python infer_feedforward.py --model models/brush_poc.pth --frames ../../source/dump \
      --out out_ff_poc --proc-size 720

ボイリング注入（--boil-amp > 0）:
  順伝播ネットは決定論的なので、同じ入力には常に同じ筆致を返し、空など静止領域は
  「ピタッと止まる」。手描きの微動（ボイリング）を取り戻すため、入力に空間/時間的に
  なめらかなコヒーレントノイズ場をフレーム毎に足し、ネットの再生成を毎フレーム少しずつ
  揺らす。白色ノイズ（粒状）ではなく低周波の揺れ場なので、筆致のかたまりがゆっくりウネる。
"""

import argparse
import os
import time

import cv2
import numpy as np
import torch

import io_utils
from transformer_net import TransformerNet


def coherent_noise(num_frames, h, w, amp, spatial, temporal, seed):
    """空間的・時間的になめらかなコヒーレントノイズ場 [T,H,W,1] を生成。

    低解像度の白色ノイズ列を空間アップサンプル（空間相関）し、時間方向に
    ガウシアン平滑（時間相関）することで、粒状でないゆっくりした揺れを作る。

    :param spatial:  低解像度ノイズの「セル」サイズ(px)。大きいほど揺れの空間スケール大。
    :param temporal: 時間方向のガウシアン σ(フレーム)。大きいほどゆっくり揺れる。
    """
    rng = np.random.default_rng(seed)
    lh = max(2, int(round(h / spatial)))
    lw = max(2, int(round(w / spatial)))
    base = rng.standard_normal((num_frames, lh, lw)).astype(np.float32)
    # 時間方向ガウシアン平滑（1D, セパラブル）。端は反射。
    if temporal > 0.0:
        radius = int(round(temporal * 3))
        k = np.exp(-0.5 * (np.arange(-radius, radius + 1) / temporal) ** 2)
        k /= k.sum()
        pad = np.pad(base, ((radius, radius), (0, 0), (0, 0)), mode="reflect")
        base = np.stack([
            np.tensordot(k, pad[t:t + 2 * radius + 1], axes=(0, 0))
            for t in range(num_frames)
        ], axis=0)
    # 空間アップサンプル（バイリニア）で各フレームを H×W に拡大
    fields = np.empty((num_frames, h, w), np.float32)
    for t in range(num_frames):
        fields[t] = cv2.resize(base[t], (w, h), interpolation=cv2.INTER_LINEAR)
    # 振幅を正規化（平滑で痩せた分を戻す）してから amp スケール
    std = fields.std() + 1e-6
    fields = fields / std * amp
    return fields[..., None]  # [T,H,W,1]


def coherent_warp_inputs(frames, amp_px, spatial, temporal, seed):
    """各入力フレームをなめらかなコヒーレント変位場で微小ワープして返す。

    加算ノイズ（明度のちらつき=ノイズに見える）と違い、ワープは「線・筆致を毎フレーム
    少しズレた位置に描き直す」手描きの揺れに近い。変位場は時間/空間相関を持つので
    かたまりごとにゆっくり泳ぎ、効果は構造（エッジ）のある所にだけ現れる＝絵に従う。

    :param amp_px: 変位の振幅(px)。1〜3 が手描き微動の目安。
    """
    T = len(frames)
    H, W, _ = frames[0].shape
    dx = coherent_noise(T, H, W, amp_px, spatial, temporal, seed)[..., 0]
    dy = coherent_noise(T, H, W, amp_px, spatial, temporal, seed + 777)[..., 0]
    gx, gy = np.meshgrid(np.arange(W, dtype=np.float32), np.arange(H, dtype=np.float32))
    warped = []
    for t in range(T):
        map_x = gx + dx[t]
        map_y = gy + dy[t]
        warped.append(cv2.remap(frames[t], map_x, map_y, cv2.INTER_LINEAR,
                                borderMode=cv2.BORDER_REFLECT))
    return warped


def main():
    ap = argparse.ArgumentParser(description="順伝播スタイル化ネットの推論検証")
    ap.add_argument("--model", required=True, help="学習済み .pth")
    ap.add_argument("--frames", required=True, help="ダンプ連番ディレクトリ")
    ap.add_argument("--max-frames", type=int, default=None)
    ap.add_argument("--proc-size", type=int, default=720)
    ap.add_argument("--fps", type=int, default=24)
    ap.add_argument("--n-blocks", type=int, default=5, help="学習時と同じ残差ブロック数を指定")
    ap.add_argument("--base-ch", type=int, default=32, help="学習時と同じ基準チャンネル数を指定")
    ap.add_argument("--warp-amp", type=float, default=0.0,
                    help="ボイリング(推奨): 入力を微小ワープする変位の振幅(px)。線を描き直す手描きの揺れ。1〜3 目安")
    ap.add_argument("--boil-amp", type=float, default=0.0,
                    help="ボイリング(加算): 入力に足すコヒーレントノイズの振幅(明度ちらつき。ノイズに見えやすい)")
    ap.add_argument("--boil-spatial", type=float, default=24.0,
                    help="ボイリング: 揺れ場の空間スケール(px)。大きいほど揺れのかたまりが大")
    ap.add_argument("--boil-temporal", type=float, default=3.0,
                    help="ボイリング: 時間方向ガウシアンσ(フレーム)。大きいほどゆっくり揺れる")
    ap.add_argument("--boil-seed", type=int, default=0)
    ap.add_argument("--out", default="out_ff")
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    io_utils.ensure_dir(args.out)

    net = TransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device).eval()
    net.load_state_dict(torch.load(args.model, map_location=device))
    print(f"loaded {args.model} on {device}")

    frames = io_utils.load_frames(args.frames, max_frames=args.max_frames, proc_size=args.proc_size)
    print(f"loaded {len(frames)} frames {frames[0].shape}")

    # ボイリング: ワープ（推奨・線を描き直す揺れ）を先に適用してから加算ノイズ（任意）
    if args.warp_amp > 0.0:
        frames = coherent_warp_inputs(frames, args.warp_amp,
                                      args.boil_spatial, args.boil_temporal, args.boil_seed)
        print(f"boiling(warp): amp={args.warp_amp}px spatial={args.boil_spatial}px "
              f"temporal={args.boil_temporal}f (input micro-warp — strokes redrawn shifted)")
    boil = None
    if args.boil_amp > 0.0:
        H0, W0, _ = frames[0].shape
        boil = coherent_noise(len(frames), H0, W0, args.boil_amp,
                              args.boil_spatial, args.boil_temporal, args.boil_seed)
        print(f"boiling(add): amp={args.boil_amp} (coherent noise added to input)")

    t0 = time.time()
    outs = []
    with torch.no_grad():
        for i, f in enumerate(frames):
            H, W, _ = f.shape
            inp = f if boil is None else np.clip(f + boil[i], 0.0, 1.0).astype(np.float32)
            x = torch.from_numpy(inp).permute(2, 0, 1).unsqueeze(0).to(device)
            y = net(x).clamp(0.0, 1.0)
            # down/up サンプルで 4 の倍数に丸まり入力と数px ずれるため元サイズに戻す
            if y.shape[2] != H or y.shape[3] != W:
                y = torch.nn.functional.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
            outs.append(y.squeeze(0).permute(1, 2, 0).cpu().numpy())
            print(f"  infer {i + 1}/{len(frames)}", end="\r")
    dt = time.time() - t0
    print(f"\ninferred {len(frames)} frames in {dt:.2f}s = {dt / len(frames) * 1000:.1f} ms/frame "
          f"({len(frames) / dt:.1f} fps) @ {frames[0].shape[1]}x{frames[0].shape[0]}")

    comparison = [np.concatenate([f, o], axis=1) for f, o in zip(frames, outs)]
    io_utils.save_video(os.path.join(args.out, "comparison.mp4"), comparison, fps=args.fps)
    for idx in sorted({0, len(frames) // 2, len(frames) - 1}):
        io_utils.save_image(os.path.join(args.out, f"sample_{idx:04d}.png"), comparison[idx])
    print(f"out -> {args.out} (comparison.mp4 [input|feedforward], sample_*.png)")


if __name__ == "__main__":
    main()

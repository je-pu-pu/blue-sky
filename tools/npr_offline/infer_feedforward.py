"""学習済み順伝播ネットでダンプフレームを推論し、見た目を検証する（フェーズ1検証）。

Gatys 法（150ステップ最適化）の naive 出力と、このネット（forward 一発）の出力を比べ、
「順伝播で確定の見た目を再現できているか」を確認する。リアルタイム化の前提が成り立つかの判断。

使い方:
  python infer_feedforward.py --model models/brush_poc.pth --frames ../../source/dump \
      --out out_ff_poc --proc-size 720
"""

import argparse
import os
import time

import numpy as np
import torch

import io_utils
from transformer_net import TransformerNet


def main():
    ap = argparse.ArgumentParser(description="順伝播スタイル化ネットの推論検証")
    ap.add_argument("--model", required=True, help="学習済み .pth")
    ap.add_argument("--frames", required=True, help="ダンプ連番ディレクトリ")
    ap.add_argument("--max-frames", type=int, default=None)
    ap.add_argument("--proc-size", type=int, default=720)
    ap.add_argument("--fps", type=int, default=24)
    ap.add_argument("--n-blocks", type=int, default=5, help="学習時と同じ残差ブロック数を指定")
    ap.add_argument("--base-ch", type=int, default=32, help="学習時と同じ基準チャンネル数を指定")
    ap.add_argument("--out", default="out_ff")
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    io_utils.ensure_dir(args.out)

    net = TransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device).eval()
    net.load_state_dict(torch.load(args.model, map_location=device))
    print(f"loaded {args.model} on {device}")

    frames = io_utils.load_frames(args.frames, max_frames=args.max_frames, proc_size=args.proc_size)
    print(f"loaded {len(frames)} frames {frames[0].shape}")

    t0 = time.time()
    outs = []
    with torch.no_grad():
        for i, f in enumerate(frames):
            H, W, _ = f.shape
            x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
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

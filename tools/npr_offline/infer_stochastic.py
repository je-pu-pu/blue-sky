"""確率的スタイル化ネット（StochasticTransformerNet）の推論＝ボイリング検証。

確率版は net(x, noise) を取る。推論時にフレーム毎ノイズを与えると筆致が描き直され、
順伝播一発（リアルタイム）でボイリングが出る、を確認する。

ノイズの作り方が肝: 学習は白色ノイズ(std≈1)で行ったので、推論ノイズも空間は白色のまま
（std≈1）にし、時間方向だけガウシアン平滑する。すると各フレームのノイズは学習時と同じ統計、
かつ隣接フレームで相関 → 筆致の配置がゆっくり変わる＝ボイリング。空間的なかたまり構造は
ネット側が作るのでノイズ側で滑らかにする必要はない（むしろ滑らかにすると応答が鈍る）。

並べ: 入力 | noise=0(決定論ベース) | noise有り(ボイリング)
"""

import argparse
import os
import time

import cv2
import numpy as np
import torch
import torch.nn.functional as F

import io_utils
from transformer_net_stochastic import StochasticTransformerNet


def recolor_lab(out_rgb, content_rgb, blue, yellow, a_spread, dark_clip):
    """色保持（非対称カラースプレッド）。Lab 空間で:
    - L(明度): ネット出力の筆致。dark_clip(L単位)で暗くなりすぎだけ防ぐ
    - b 軸: 青方向(db<0)は blue 倍率で増幅、黄方向(db>0)は yellow 倍率でカット
    - a 軸: a_spread で控えめに（肌色=赤方向の暴走を防ぐ）
    Gram が転写する星月夜の全パレット混入（黒塊・黄色・肌色）を抑え、入力の局所色を
    基準に「青の中で広くばらける」見た目にする。後処理のみ＝リアルタイム維持・再学習不要。
    """
    out_lab = cv2.cvtColor(out_rgb, cv2.COLOR_RGB2Lab)
    con_lab = cv2.cvtColor(content_rgb, cv2.COLOR_RGB2Lab)
    L = con_lab[..., 0] + np.maximum(out_lab[..., 0] - con_lab[..., 0], -dark_clip)
    da = out_lab[..., 1] - con_lab[..., 1]
    db = out_lab[..., 2] - con_lab[..., 2]
    a = con_lab[..., 1] + da * a_spread
    b = con_lab[..., 2] + np.where(db < 0, db * blue, db * yellow)
    lab = np.stack([L, a, b], axis=-1).astype(np.float32)
    return np.clip(cv2.cvtColor(lab, cv2.COLOR_Lab2RGB), 0, 1)


def temporal_white_noise(num_frames, h, w, amp, temporal, seed):
    """空間は白色・時間方向だけガウシアン平滑したノイズ列 [T,H,W]（std≈amp）。"""
    rng = np.random.default_rng(seed)
    base = rng.standard_normal((num_frames, h, w)).astype(np.float32)
    if temporal > 0.0:
        radius = int(round(temporal * 3))
        k = np.exp(-0.5 * (np.arange(-radius, radius + 1) / temporal) ** 2)
        k /= k.sum()
        pad = np.pad(base, ((radius, radius), (0, 0), (0, 0)), mode="reflect")
        base = np.stack([np.tensordot(k, pad[t:t + 2 * radius + 1], axes=(0, 0))
                         for t in range(num_frames)], axis=0)
    base = base / (base.std() + 1e-6) * amp
    return base.astype(np.float32)


def main():
    ap = argparse.ArgumentParser(description="確率的スタイル化ネットの推論（ボイリング）")
    ap.add_argument("--model", required=True)
    ap.add_argument("--frames", required=True)
    ap.add_argument("--max-frames", type=int, default=None)
    ap.add_argument("--proc-size", type=int, default=720)
    ap.add_argument("--fps", type=int, default=24)
    ap.add_argument("--n-blocks", type=int, default=8)
    ap.add_argument("--base-ch", type=int, default=48)
    ap.add_argument("--noise-amp", type=float, default=1.0,
                    help="推論ノイズ振幅(std)。学習が std≈1 なので 1.0 が基準。小さいと揺れ弱・大きいと強")
    ap.add_argument("--noise-temporal", type=float, default=3.0,
                    help="ノイズの時間方向ガウシアンσ(フレーム)。大きいほどゆっくり描き直す")
    ap.add_argument("--noise-seed", type=int, default=0)
    # 色保持（非対称カラースプレッド）。既定はユーザー確定値 blue=3.0 dark=60。
    ap.add_argument("--color", action="store_true",
                    help="色保持（輝度転写＋青方向スプレッド）を有効化。空の黒塊/黄色/肌色混入を抑え青の中で色をばらけさせる")
    ap.add_argument("--color-blue", type=float, default=3.0, help="青方向(b軸負)の色ブレ増幅倍率")
    ap.add_argument("--color-yellow", type=float, default=0.0, help="黄方向(b軸正)の倍率。0=黄色カット")
    ap.add_argument("--color-a", type=float, default=0.5, help="a軸(赤緑)スプレッド倍率。肌色暴走防止に控えめ")
    ap.add_argument("--color-dark", type=float, default=60.0, help="暗さ制限(L単位)。大きいほど深い青/暗部を残す")
    ap.add_argument("--out", default="out_boil_net")
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    io_utils.ensure_dir(args.out)

    net = StochasticTransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device).eval()
    net.load_state_dict(torch.load(args.model, map_location=device))
    print(f"loaded {args.model} on {device}")

    frames = io_utils.load_frames(args.frames, max_frames=args.max_frames, proc_size=args.proc_size)
    H, W, _ = frames[0].shape
    print(f"loaded {len(frames)} frames {frames[0].shape}")

    noise = temporal_white_noise(len(frames), H, W, args.noise_amp, args.noise_temporal, args.noise_seed)
    print(f"noise: amp={args.noise_amp} temporal={args.noise_temporal}f (spatial white, temporally smooth)")

    def run(use_noise):
        outs = []
        t0 = time.time()
        with torch.no_grad():
            for i, f in enumerate(frames):
                x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
                n = None
                if use_noise:
                    n = torch.from_numpy(noise[i]).view(1, 1, H, W).to(device)
                y = net(x, n).clamp(0.0, 1.0)
                if y.shape[2] != H or y.shape[3] != W:
                    y = F.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
                o = y.squeeze(0).permute(1, 2, 0).cpu().numpy().astype(np.float32)
                if args.color:
                    o = recolor_lab(o, f, args.color_blue, args.color_yellow, args.color_a, args.color_dark)
                outs.append(o)
        dt = time.time() - t0
        tag = "noise" if use_noise else "det"
        print(f"  {tag}: {dt/len(frames)*1000:.1f} ms/frame ({len(frames)/dt:.1f} fps)")
        return outs

    det = run(False)
    boil = run(True)

    combined = [np.concatenate([f, d, b], axis=1) for f, d, b in zip(frames, det, boil)]
    io_utils.save_video(os.path.join(args.out, "comparison.mp4"), combined, fps=args.fps)
    for idx in sorted({0, len(frames) // 2, len(frames) - 1}):
        io_utils.save_image(os.path.join(args.out, f"sample_{idx:04d}.png"), combined[idx])
    print(f"out -> {args.out} (comparison.mp4 [input|det|boil], sample_*.png)")


if __name__ == "__main__":
    main()

"""色保持（輝度転写）の検証。

Gram スタイル損失は星月夜の全体パレット（青/黒/黄/肌色）を場所を問わず転写するので、
平坦な空に黒・肌色など局所の色（空の青）と無関係な色が混入する。対策＝輝度転写:
ネット出力の輝度（筆致パターン）だけ採り、色は入力フレームの局所色に従わせる。
→ 空は青のまま輝度だけ筆致で揺れ、異物色は局所色に寄る。後処理のみで再学習不要。

並べ: 入力 | ボイリング(色保持なし) | ボイリング(色保持あり)
"""

import sys

import numpy as np
import torch
import torch.nn.functional as F

import io_utils
from infer_stochastic import temporal_white_noise
from transformer_net_stochastic import StochasticTransformerNet

sys.stdout.reconfigure(encoding="utf-8")

MODEL = "models/starry_boil2.pth"
FRAMES = "../../source/dump"
PROC = 720
NFRAMES = 60
NOISE_AMP, NOISE_T = 1.0, 3.0
STRENGTH = 1.0  # 1=完全に入力の色へ / 0=ネットの色のまま

_LUMA = np.array([0.299, 0.587, 0.114], np.float32)


def color_preserve(out_rgb, content_rgb, strength):
    """out の輝度 + content の色。加算式輝度転写（色相・彩度を content から保つ）。"""
    out_y = (out_rgb * _LUMA).sum(-1, keepdims=True)
    con_y = (content_rgb * _LUMA).sum(-1, keepdims=True)
    recolored = content_rgb + (out_y - con_y)        # content の色に out の輝度を載せる
    return np.clip((1 - strength) * out_rgb + strength * recolored, 0, 1)


device = "cuda" if torch.cuda.is_available() else "cpu"
net = StochasticTransformerNet(n_blocks=8, base_ch=48).to(device).eval()
net.load_state_dict(torch.load(MODEL, map_location=device))
frames = io_utils.load_frames(FRAMES, max_frames=NFRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
noise = temporal_white_noise(len(frames), H, W, NOISE_AMP, NOISE_T, 0)
print(f"{len(frames)} frames {frames[0].shape}")

combined = []
with torch.no_grad():
    for i, f in enumerate(frames):
        x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
        n = torch.from_numpy(noise[i]).view(1, 1, H, W).to(device)
        y = net(x, n).clamp(0, 1)
        if y.shape[2] != H or y.shape[3] != W:
            y = F.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
        boil = y.squeeze(0).permute(1, 2, 0).cpu().numpy()
        cp = color_preserve(boil, f, STRENGTH)
        combined.append(np.concatenate([f, boil, cp], axis=1))

io_utils.ensure_dir("out_colorfix")
io_utils.save_video("out_colorfix/colorfix.mp4", combined, fps=12)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_colorfix/sample_{idx:04d}.png", combined[idx])
print("out -> out_colorfix/colorfix.mp4 (左=入力 / 中=色保持なし / 右=色保持あり)")

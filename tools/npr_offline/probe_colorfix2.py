"""色操作を輝度/色の2軸に分解して制御する検証。

- dark_clip: 筆致がこれ以上暗くなれない床（空の黒い塊を消す）
- spread:    入力の局所色を基準に、ネットの色変化を混ぜる量（青周りの色ブレを増やす）
- input_blend: 最終結果を入力へさらに寄せる量（入力に近い版）

並べ: 入力 | 現状(spread0/床なし) | 色ブレ中+黒塊除去 | 色ブレ大+黒塊除去 | 入力寄り
"""

import sys

import cv2
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
_LUMA = np.array([0.299, 0.587, 0.114], np.float32)

# (ラベル, spread, dark_clip, input_blend)
VARIANTS = [
    ("current spread0", 0.0, None, 0.0),
    ("spread0.5 noBlack", 0.5, 0.15, 0.0),
    ("spread0.9 noBlack", 0.9, 0.15, 0.0),
    ("closer to input", 0.0, 0.15, 0.4),
]


def recolor(out_rgb, content_rgb, spread, dark_clip, input_blend):
    out_y = (out_rgb * _LUMA).sum(-1, keepdims=True)
    con_y = (content_rgb * _LUMA).sum(-1, keepdims=True)
    d = out_y - con_y
    if dark_clip is not None:
        d = np.maximum(d, -dark_clip)              # 暗くなりすぎ防止＝黒塊除去
    final_y = con_y + d
    out_chroma = out_rgb - out_y
    con_chroma = content_rgb - con_y
    final_chroma = con_chroma + spread * (out_chroma - con_chroma)
    res = np.clip(final_y + final_chroma, 0, 1)
    if input_blend > 0:
        res = np.clip((1 - input_blend) * res + input_blend * content_rgb, 0, 1)
    return res


device = "cuda" if torch.cuda.is_available() else "cpu"
net = StochasticTransformerNet(n_blocks=8, base_ch=48).to(device).eval()
net.load_state_dict(torch.load(MODEL, map_location=device))
frames = io_utils.load_frames(FRAMES, max_frames=NFRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
noise = temporal_white_noise(len(frames), H, W, 1.0, 3.0, 0)
print(f"{len(frames)} frames {frames[0].shape}")


def label(img, txt):
    b = cv2.cvtColor((img * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
    cv2.putText(b, txt, (10, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2, cv2.LINE_AA)
    return cv2.cvtColor(b, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0


combined = []
with torch.no_grad():
    for i, f in enumerate(frames):
        x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
        n = torch.from_numpy(noise[i]).view(1, 1, H, W).to(device)
        y = net(x, n).clamp(0, 1)
        if y.shape[2] != H or y.shape[3] != W:
            y = F.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
        boil = y.squeeze(0).permute(1, 2, 0).cpu().numpy()
        cols = [label(f, "input")]
        for lab, sp, dc, ib in VARIANTS:
            cols.append(label(recolor(boil, f, sp, dc, ib), lab))
        combined.append(np.concatenate(cols, axis=1))

io_utils.ensure_dir("out_colorfix2")
io_utils.save_video("out_colorfix2/colorfix2.mp4", combined, fps=12)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_colorfix2/sample_{idx:04d}.png", combined[idx])
print("out -> out_colorfix2/colorfix2.mp4")

"""非対称カラースプレッド検証（青方向だけ広げ、黄色は入れない）。

Lab 空間で色を扱う:
- L(明度): ネット出力の筆致。dark_clip(L単位)で暗くなりすぎだけ防ぐ（緩め＝深い青は残す）
- b 軸(+黄/-青): 青方向(db<0)は blue 倍率で増幅、黄方向(db>0)は yellow 倍率(=0)でカット
- a 軸(+赤/-緑): a_spread で控えめに（肌色=赤方向の暴走を防ぐ）
→ 入力の局所色を基準に「青の中で広くばらけ」「黄色は入らない」。

並べ: 入力 | blue1.0 dark30 | blue1.5 dark30 | blue1.5 dark45 | blue2.0 dark45
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

# (ラベル, blue倍率, yellow倍率, a倍率, dark_clip[L単位])
VARIANTS = [
    ("blue2.0 dark45", 2.0, 0.0, 0.4, 45),
    ("blue3.0 dark45", 3.0, 0.0, 0.5, 45),
    ("blue3.0 dark60", 3.0, 0.0, 0.5, 60),
]


def recolor_lab(out_rgb, content_rgb, blue, yellow, a_spread, dark_clip):
    out_lab = cv2.cvtColor(out_rgb, cv2.COLOR_RGB2Lab)
    con_lab = cv2.cvtColor(content_rgb, cv2.COLOR_RGB2Lab)
    L = con_lab[..., 0] + np.maximum(out_lab[..., 0] - con_lab[..., 0], -dark_clip)
    da = out_lab[..., 1] - con_lab[..., 1]
    db = out_lab[..., 2] - con_lab[..., 2]
    a = con_lab[..., 1] + da * a_spread
    b = con_lab[..., 2] + np.where(db < 0, db * blue, db * yellow)   # 青は増幅・黄はカット
    lab = np.stack([L, a, b], axis=-1).astype(np.float32)
    return np.clip(cv2.cvtColor(lab, cv2.COLOR_Lab2RGB), 0, 1)


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
        boil = y.squeeze(0).permute(1, 2, 0).cpu().numpy().astype(np.float32)
        cols = [label(f, "input")]
        for lab_txt, bl, ye, asp, dc in VARIANTS:
            cols.append(label(recolor_lab(boil, f, bl, ye, asp, dc), lab_txt))
        combined.append(np.concatenate(cols, axis=1))

io_utils.ensure_dir("out_colorfix3")
io_utils.save_video("out_colorfix3/colorfix3.mp4", combined, fps=12)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_colorfix3/sample_{idx:04d}.png", combined[idx])
print("out -> out_colorfix3/colorfix3.mp4")

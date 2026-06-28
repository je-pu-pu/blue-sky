"""小容量(5/32, ~120fps狙い) vs 大容量(8/48, ~53fps) の確率版を確定色で比較。

両方に確定色設定（blue=3.0 yellow=0 a=0.5 dark=60）を適用し、画質差と速度差を見る。
選択肢3の知見「容量を上げても筆致はほぼ変わらない」が確率版でも成り立つなら、小容量で
速度倍・画質ほぼ同等となるはず。
並べ: 入力 | small(5/32) | big(8/48)
"""

import sys
import time

import numpy as np
import torch
import torch.nn.functional as F

import io_utils
from infer_stochastic import temporal_white_noise, recolor_lab
from transformer_net_stochastic import StochasticTransformerNet

sys.stdout.reconfigure(encoding="utf-8")

FRAMES = "../../source/dump"
PROC = 720
NFRAMES = 60
COLOR = dict(blue=3.0, yellow=0.0, a_spread=0.5, dark_clip=60.0)
MODELS = [("small", "models/starry_boil_small.pth", 5, 32),
          ("big", "models/starry_boil2.pth", 8, 48)]

device = "cuda" if torch.cuda.is_available() else "cpu"
frames = io_utils.load_frames(FRAMES, max_frames=NFRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
noise = temporal_white_noise(len(frames), H, W, 1.0, 3.0, 0)
print(f"{len(frames)} frames {frames[0].shape}")


def run(path, nb, bc):
    net = StochasticTransformerNet(n_blocks=nb, base_ch=bc).to(device).eval()
    net.load_state_dict(torch.load(path, map_location=device))
    outs = []
    t0 = time.time()
    with torch.no_grad():
        for i, f in enumerate(frames):
            x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
            n = torch.from_numpy(noise[i]).view(1, 1, H, W).to(device)
            y = net(x, n).clamp(0, 1)
            if y.shape[2] != H or y.shape[3] != W:
                y = F.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
            outs.append(y.squeeze(0).permute(1, 2, 0).cpu().numpy().astype(np.float32))
    dt = time.time() - t0
    print(f"{path}: {dt/len(frames)*1000:.1f} ms/frame ({len(frames)/dt:.1f} fps)")
    return [recolor_lab(o, f, **COLOR) for o, f in zip(outs, frames)]


cols = [run(p, nb, bc) for _, p, nb, bc in MODELS]
combined = [np.concatenate([frames[i], cols[0][i], cols[1][i]], axis=1) for i in range(len(frames))]

io_utils.ensure_dir("out_small_vs_big")
io_utils.save_video("out_small_vs_big/compare.mp4", combined, fps=12)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_small_vs_big/sample_{idx:04d}.png", combined[idx])
print("out -> out_small_vs_big/compare.mp4 (左=入力 / 中=small 5/32 / 右=big 8/48, 確定色)")

"""容量比較: cw6(n_blocks5/base32) vs cap(n_blocks8/base48)。

決定論モデルの「画質・渦/筆致の忠実度」が容量で上がるかを並べて確認し、推論速度も測る。
並べ: 入力 | cw6 | b8c48
"""

import sys
import time

import numpy as np
import torch

import io_utils
from transformer_net import TransformerNet

sys.stdout.reconfigure(encoding="utf-8")

FRAMES = "../../source/dump"
PROC = 720
MODELS = [
    ("cw6", "models/starry_cw6.pth", 5, 32),
    ("b8c48", "models/starry_cap_b8c48.pth", 8, 48),
]

device = "cuda" if torch.cuda.is_available() else "cpu"
frames = io_utils.load_frames(FRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
print(f"{len(frames)} frames {frames[0].shape}")


def run(path, nb, bc):
    net = TransformerNet(n_blocks=nb, base_ch=bc).to(device).eval()
    net.load_state_dict(torch.load(path, map_location=device))
    outs = []
    t0 = time.time()
    with torch.no_grad():
        for f in frames:
            x = torch.from_numpy(f).permute(2, 0, 1).unsqueeze(0).to(device)
            y = net(x).clamp(0.0, 1.0)
            if y.shape[2] != H or y.shape[3] != W:
                y = torch.nn.functional.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
            outs.append(y.squeeze(0).permute(1, 2, 0).cpu().numpy())
    dt = time.time() - t0
    print(f"{path}: {dt/len(frames)*1000:.1f} ms/frame ({len(frames)/dt:.1f} fps)")
    return outs


cols = [run(p, nb, bc) for _, p, nb, bc in MODELS]
gap = np.ones((H, 6, 3), np.float32)
combined = []
for i in range(len(frames)):
    row = frames[i]
    for c in cols:
        row = np.concatenate([row, gap, c[i]], axis=1)
    combined.append(row)

io_utils.ensure_dir("out_cap_compare")
io_utils.save_video("out_cap_compare/compare.mp4", combined, fps=24)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_cap_compare/sample_{idx:04d}.png", combined[idx])
print("out -> out_cap_compare/compare.mp4 (左=入力 / 中=cw6 / 右=b8c48)")

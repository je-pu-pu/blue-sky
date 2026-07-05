"""ボイリング方式の比較: 無し | 加算ノイズ | ワープ。

加算ノイズは明度ちらつきで「絵柄と無関係に乗るノイズ」に見える。ワープは入力を微小変位
させてから順伝播 → 線・筆致が「少しズレた位置に描き直される」手描きの揺れに近い（効果は
構造のある所だけに現れ、絵に従う）。並べて違いを確認する。
"""

import sys

import numpy as np
import torch

import io_utils
from infer_feedforward import coherent_noise, coherent_warp_inputs
from transformer_net import TransformerNet

sys.stdout.reconfigure(encoding="utf-8")

MODEL = "models/starry_cw6.pth"
FRAMES = "../../source/dump"
PROC = 720
SPATIAL, TEMPORAL = 24.0, 3.0
ADD_AMP = 0.06
WARP_AMP = 2.0

device = "cuda" if torch.cuda.is_available() else "cpu"
net = TransformerNet(n_blocks=5, base_ch=32).to(device).eval()
net.load_state_dict(torch.load(MODEL, map_location=device))
frames = io_utils.load_frames(FRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
print(f"{len(frames)} frames {frames[0].shape}")


def run(in_frames):
    outs = []
    with torch.no_grad():
        for f in in_frames:
            x = torch.from_numpy(np.ascontiguousarray(f)).permute(2, 0, 1).unsqueeze(0).to(device)
            y = net(x).clamp(0.0, 1.0)
            if y.shape[2] != H or y.shape[3] != W:
                y = torch.nn.functional.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
            outs.append(y.squeeze(0).permute(1, 2, 0).cpu().numpy())
    return outs


col_none = run(frames)
print("rendered: none")
boil = coherent_noise(len(frames), H, W, ADD_AMP, SPATIAL, TEMPORAL, 0)
col_add = run([np.clip(frames[i] + boil[i], 0, 1).astype(np.float32) for i in range(len(frames))])
print("rendered: add")
col_warp = run(coherent_warp_inputs(frames, WARP_AMP, SPATIAL, TEMPORAL, 0))
print("rendered: warp")

gap = np.ones((H, 6, 3), np.float32)
combined = []
for i in range(len(frames)):
    row = np.concatenate([col_none[i], gap, col_add[i], gap, col_warp[i]], axis=1)
    combined.append(row)

io_utils.ensure_dir("out_boil_warp")
io_utils.save_video("out_boil_warp/warp.mp4", combined, fps=24)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_boil_warp/sample_{idx:04d}.png", combined[idx])
print(f"out -> out_boil_warp/warp.mp4 (左=無し / 中=加算{ADD_AMP} / 右=ワープ{WARP_AMP}px)")

"""推論時ボイリング注入の振幅スイープ検証。

順伝播ネット(cw6)は決定論的で静止領域が止まる。入力にコヒーレントノイズ場を足して
再生成を揺らし「手描きの微動」を取り戻せるか、振幅を振って比べる。
並べ: ボイリング無し | amp=0.06 | amp=0.10（すべて同じフレーム）
"""

import sys

import numpy as np
import torch

import io_utils
from infer_feedforward import coherent_noise
from transformer_net import TransformerNet

sys.stdout.reconfigure(encoding="utf-8")

MODEL = "models/starry_cw6.pth"
FRAMES = "../../source/dump"
PROC = 720
AMPS = [0.0, 0.06, 0.10]
SPATIAL, TEMPORAL = 24.0, 3.0

device = "cuda" if torch.cuda.is_available() else "cpu"
net = TransformerNet(n_blocks=5, base_ch=32).to(device).eval()
net.load_state_dict(torch.load(MODEL, map_location=device))
frames = io_utils.load_frames(FRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
print(f"{len(frames)} frames {frames[0].shape}")


def render(amp):
    boil = None if amp == 0.0 else coherent_noise(len(frames), H, W, amp, SPATIAL, TEMPORAL, 0)
    outs = []
    with torch.no_grad():
        for i, f in enumerate(frames):
            inp = f if boil is None else np.clip(f + boil[i], 0.0, 1.0).astype(np.float32)
            x = torch.from_numpy(inp).permute(2, 0, 1).unsqueeze(0).to(device)
            y = net(x).clamp(0.0, 1.0)
            if y.shape[2] != H or y.shape[3] != W:
                y = torch.nn.functional.interpolate(y, size=(H, W), mode="bilinear", align_corners=False)
            outs.append(y.squeeze(0).permute(1, 2, 0).cpu().numpy())
    print(f"rendered amp={amp}")
    return outs


cols = [render(a) for a in AMPS]
gap = np.ones((H, 6, 3), np.float32)
combined = []
for i in range(len(frames)):
    row = cols[0][i]
    for c in cols[1:]:
        row = np.concatenate([row, gap, c[i]], axis=1)
    combined.append(row)

io_utils.ensure_dir("out_boil_sweep")
io_utils.save_video("out_boil_sweep/sweep.mp4", combined, fps=24)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_boil_sweep/sample_{idx:04d}.png", combined[idx])
print("out -> out_boil_sweep/sweep.mp4 (左=無し / 中=0.06 / 右=0.10)")

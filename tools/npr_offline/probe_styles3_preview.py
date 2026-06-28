"""3スタイル（ムンク/スーラ/モネ）の Gatys 即プレビュー（学習不要）。

各スタイルがこのシーンにどう乗るかを先に見る。良ければ realtime 用に学習する。
並べ: 縦=フレーム(0/40/90)、横=[入力 | ムンク | スーラ | モネ]
"""

import sys

import cv2
import numpy as np
from PIL import Image

from stylizers.neural_style import NeuralStyleStylizer

sys.stdout.reconfigure(encoding="utf-8")

STYLES = [("munch", "styles/munch_scream.jpg"),
          ("seurat", "styles/seurat_jatte.jpg"),
          ("monet", "styles/monet_sunrise.jpg")]
FRAME_IDS = [0, 40, 90]
STEPS, MAXSIZE, SW, CW, SS = 60, 448, 1e6, 1.0, 1.0


def load(i):
    return np.asarray(Image.open(f"../../source/dump/color_{i:04d}.png").convert("RGB")).astype(np.float32) / 255.0


def bgr(x, txt=None):
    b = cv2.cvtColor((x * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
    if txt:
        cv2.putText(b, txt, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 255), 2, cv2.LINE_AA)
    return b


styls = {name: NeuralStyleStylizer(path, steps=STEPS, style_weight=SW, content_weight=CW,
                                   max_size=MAXSIZE, style_scale=SS) for name, path in STYLES}

rows = []
for fid in FRAME_IDS:
    img = load(fid)
    tiles = [bgr(img, f"input f{fid}")]
    for name, _ in STYLES:
        out = styls[name].stylize(img)
        tiles.append(bgr(out, name))
        print(f"done f{fid} {name}")
    gap = np.full((img.shape[0], 6, 3), 255, np.uint8)
    row = tiles[0]
    for t in tiles[1:]:
        row = cv2.hconcat([row, gap, t])
    rows.append(row)

vgap = np.full((6, rows[0].shape[1], 3), 255, np.uint8)
full = rows[0]
for r in rows[1:]:
    full = cv2.vconcat([full, vgap, r])
cv2.imwrite("styles3_preview.png", full)
print("out -> styles3_preview.png")

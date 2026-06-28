"""Gatys のスタイル解像度(style_scale)を振って「渦が Gram で出るスケール」を探る検証。

スタイル画像を content と同解像度で VGG に通すと、星月夜の渦が縮小されて VGG 受容野
から見て大きすぎ、Gram には筆致の粒しか残らない（=渦が出ない）。style_scale を下げて
渦全体を受容野に収めれば大スケール構造が転写されるはず、という仮説の確認。
"""

import sys

import cv2
import numpy as np
from PIL import Image

from stylizers.neural_style import NeuralStyleStylizer

sys.stdout.reconfigure(encoding="utf-8")

FRAME = "../../source/dump/color_0060.png"
STYLE = "styles/brush_starry_night.jpg"
MAX_SIZE = 512
STEPS = 300
STYLE_WEIGHT = 1e6
SCALES = [1.0, 1.5, 2.0, 2.5]

img = np.asarray(Image.open(FRAME).convert("RGB")).astype(np.float32) / 255.0

tiles = []
inp = cv2.cvtColor((img * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
tiles.append(inp)
for sc in SCALES:
    s = NeuralStyleStylizer(STYLE, steps=STEPS, style_weight=STYLE_WEIGHT,
                            max_size=MAX_SIZE, style_scale=sc)
    out = s.stylize(img)
    bgr = cv2.cvtColor((out * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
    cv2.putText(bgr, f"scale={sc}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
                0.9, (255, 255, 255), 2, cv2.LINE_AA)
    tiles.append(bgr)
    print(f"done scale={sc}")

gap = np.full((tiles[0].shape[0], 6, 3), 255, np.uint8)
row = tiles[0]
for t in tiles[1:]:
    row = cv2.hconcat([row, gap, t])
cv2.imwrite("probe_style_scale.png", row)
print("out -> probe_style_scale.png (左=入力, 以降 scale 昇順)")

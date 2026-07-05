"""ハイブリッド検証: Gatys(色・流れる筆致=本物の絵柄) に flow_paint の明度うねり(渦)を重ねる。

Gram が出せない大域的な「渦のうねり」を、手続き的な明度うねり場で後乗せする。
並べ: 入力 | Gatys のみ | Gatys + うねり
"""

import sys

import cv2
import numpy as np
from PIL import Image

from flow_paint import value_field
from stylizers.neural_style import NeuralStyleStylizer

sys.stdout.reconfigure(encoding="utf-8")

FRAME = "../../source/dump/color_0060.png"
STYLE = "styles/brush_starry_night.jpg"

img = np.asarray(Image.open(FRAME).convert("RGB")).astype(np.float32) / 255.0

s = NeuralStyleStylizer(STYLE, steps=300, style_weight=1e6, max_size=512, style_scale=1.0)
gatys = s.stylize(img)                              # RGB float [0,1]
print("gatys done")

h, w, _ = gatys.shape
vf = value_field(h, w, scale=60.0, freq=5.0, seed=0)   # [-1,1]
amp, tint = 0.16, 0.5
hyb = gatys.copy()
hyb[..., 0] += vf * amp + vf * tint * amp * 0.5         # R: 明部=黄
hyb[..., 1] += vf * amp * 0.4                           # G
hyb[..., 2] += -vf * amp * 0.6 - vf * tint * amp * 0.5  # B: 暗部=青
hyb = np.clip(hyb, 0, 1)

def bgr(x):
    return cv2.cvtColor((x * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)

gap = np.full((img.shape[0], 6, 3), 255, np.uint8)
cv2.imwrite("probe_hybrid.png", cv2.hconcat([bgr(img), gap, bgr(gatys), gap, bgr(hyb)]))
print("out -> probe_hybrid.png (左=入力 / 中=Gatys / 右=Gatys+うねり)")

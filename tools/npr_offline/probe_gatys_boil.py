"""Gatys のボイリング量スイープ。

init=コンテンツのまま高ステップで収束させると各フレームが同じ極小に落ち「揺れない」。
各フレームの初期値にフレーム固有ノイズ(jitter)を入れると最適化が別の極小=筆致の別配置へ
落ち、手描きの「描き直し」揺れが出る。ノイズ量を振って比較する。
並べ: boil=0(収束=揺れない) | 0.15 | 0.30
"""

import sys

import numpy as np

import io_utils
from stylizers.neural_style import NeuralStyleStylizer

sys.stdout.reconfigure(encoding="utf-8")

FRAMES = "../../source/dump"
STYLE = "styles/brush_starry_night.jpg"
PROC = 720
NFRAMES = 30
STEPS = 40          # 収束させすぎると init の差が消えるので控えめ
MAXSIZE = 320
STYLE_W = 1e6
BOILS = [0.0, 0.15, 0.30]

frames = io_utils.load_frames(FRAMES, max_frames=NFRAMES, proc_size=PROC)
H, W, _ = frames[0].shape
print(f"{len(frames)} frames {frames[0].shape}")

s = NeuralStyleStylizer(STYLE, steps=STEPS, style_weight=STYLE_W, content_weight=1.0,
                        max_size=MAXSIZE, style_scale=1.5)


def run(boil):
    outs = []
    for i, f in enumerate(frames):
        if boil > 0.0:
            o = s.stylize(f, jitter=boil, jitter_seed=i)   # フレーム毎に異なる初期ノイズ
        else:
            o = s.stylize(f)
        outs.append(o)
        print(f"  boil={boil} {i+1}/{len(frames)}", end="\r")
    print(f"\ndone boil={boil}")
    return outs


cols = [run(b) for b in BOILS]
gap = np.ones((H, 6, 3), np.float32)
combined = []
for i in range(len(frames)):
    row = cols[0][i]
    for c in cols[1:]:
        row = np.concatenate([row, gap, c[i]], axis=1)
    combined.append(row)

io_utils.ensure_dir("out_gatys_boil")
io_utils.save_video("out_gatys_boil/boil.mp4", combined, fps=12)
for idx in (0, len(frames) // 2, len(frames) - 1):
    io_utils.save_image(f"out_gatys_boil/sample_{idx:04d}.png", combined[idx])
print("out -> out_gatys_boil/boil.mp4 (左=0/揺れない 中=0.15 右=0.30)")

"""既存スクショ（R↔B 入替＋アルファ付きで保存された旧バグ版）を補正して public/shots/ へ。

- R と B を入れ替え（黄色→水色 を元に戻す）
- アルファを捨てて不透明化（文字周りの透明箱を解消）

エンジンは修正済みなので、今後の撮影ではこの補正は不要（そのまま置けばよい）。
今のエンジンでは、ゲーム中に F11 を押すと作業ディレクトリ（source）の screenshot/shot_%04d.png に保存される。
使い方: python scripts/fix_shots.py  （下の PICK を編集して選ぶ）
"""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent          # tools/pr-video
SRC = ROOT.parent.parent / "source" / "screenshot"     # <repo>/source/screenshot
OUT = ROOT / "public" / "shots"
OUT.mkdir(parents=True, exist_ok=True)

# 動画で使うショット（スコア進行・状態・アングルが分かれるよう選定）
PICK = [2, 6, 12, 18, 24, 30, 38, 40]

for f in OUT.glob("*.png"):
    f.unlink()

for i, n in enumerate(PICK, start=1):
    src = SRC / f"shot_{n:04d}.png"
    if not src.exists():
        print("無いので飛ばします:", src)
        continue
    im = Image.open(src).convert("RGBA")
    r, g, b, _a = im.split()
    fixed = Image.merge("RGB", (b, g, r))   # R↔B を戻す＋アルファ破棄
    dst = OUT / f"shot_{i:02d}.png"
    fixed.save(dst)
    print(f"shot_{n:04d} -> {dst.name}")

print("完了:", len(list(OUT.glob('*.png'))), "枚")

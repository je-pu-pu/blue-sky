"""ONNX モデルを fp16 化する（入出力は fp32 のまま・内部だけ fp16）。

DirectML/Blackwell で推論が約1.75〜3倍速になる（実機 768 で infer 21.6→6.5ms）。
入出力 fp32 維持（keep_io_types=True）なので C++ 側は無改修でモデル差し替えのみ。
重みはインライン化して単一ファイルにする（.onnx.data 不要）。

使い方（npr venv で）:
    .venv/Scripts/python.exe to_fp16.py
依存: onnx, onnxconverter-common
    python -m uv pip install --python tools/npr_offline/.venv/Scripts/python.exe onnxconverter-common
"""
import onnx
import os
from onnxconverter_common import float16

SRC = "models/starry_boil_small.onnx"
DST = "models/starry_boil_small_fp16.onnx"


def main():
    m = onnx.load(SRC)  # 同ディレクトリの .onnx.data を自動ロード
    print(f"loaded {SRC}; converting to fp16 (keep_io_types=True)...")
    m16 = float16.convert_float_to_float16(m, keep_io_types=True)
    onnx.save(m16, DST, save_as_external_data=False)  # 重みインライン化
    print(f"saved {DST} ({os.path.getsize(DST)} bytes)")
    print("→ source/media/neural/ にコピーして使う（gitignore 対象）")


if __name__ == "__main__":
    main()

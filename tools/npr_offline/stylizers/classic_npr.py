"""古典 NPR フィルタ（スモークテスト用）。

依存は opencv / numpy のみ・モデル DL 不要で即時に動くため、
ハーネスの配線（連番読み込み→変換→メトリクス→出力）を数秒で確認するのに使う。
セル調（バイラテラル平滑＋ポスタリゼーション）＋輪郭線（Canny）。
"""

import cv2
import numpy as np

from .base import Stylizer


class ClassicNprStylizer(Stylizer):
    def __init__(self, levels: int = 6, edge_strength: float = 0.8):
        self.levels = max(2, levels)
        self.edge_strength = float(np.clip(edge_strength, 0.0, 1.0))

    def stylize(self, image: np.ndarray) -> np.ndarray:
        bgr = cv2.cvtColor((image * 255.0).astype(np.uint8), cv2.COLOR_RGB2BGR)

        # 平滑化して色を量子化（セル調）
        smooth = cv2.bilateralFilter(bgr, d=9, sigmaColor=75, sigmaSpace=75)
        q = np.round(smooth / 255.0 * (self.levels - 1)) / (self.levels - 1) * 255.0
        out = q.astype(np.uint8)

        # 輪郭線を重ねる
        gray = cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray, 80, 160)
        edges = cv2.dilate(edges, np.ones((2, 2), np.uint8))
        out[edges > 0] = (out[edges > 0] * (1.0 - self.edge_strength)).astype(np.uint8)

        rgb = cv2.cvtColor(out, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0
        return np.clip(rgb, 0.0, 1.0)

"""鉛筆スケッチ・スタイライザ（古典アルゴリズム・モデル不要）。

鉛筆画の本質は「陰影」と「線」。Gatys にスタイル画像として鉛筆画を渡すより、
専用アルゴリズムの方が綺麗で、かつ確定的・時間的に安定する。

- 陰影: グレースケール → 反転 → ぼかし → ドッジ合成（color dodge）。
        紙の白地に黒鉛が乗ったような連続トーンを作る。
- 線:   XDoG（eXtended Difference of Gaussians, Winnemoller 2011）。
        Canny の硬い2値線より鉛筆ハッチング調の柔らかい線が出る。
- 仕上げ: 陰影 × 線 を乗算。任意で淡い色味（tint）。

手続き的な紙テクスチャ（乱数ノイズ）は意図的に入れない。画面に固定されず
モーションに追従しないため、本ハーネスの目的（時間安定の検証）でちらつき源になる。
"""

import cv2
import numpy as np

from .base import Stylizer


def _xdog(gray: np.ndarray, sigma: float, k: float, tau: float,
          eps: float, phi: float) -> np.ndarray:
    """XDoG による線画レイヤ。値域 [0,1]、線=暗(0付近)・地=明(1)。"""
    g1 = cv2.GaussianBlur(gray, (0, 0), sigmaX=sigma)
    g2 = cv2.GaussianBlur(gray, (0, 0), sigmaX=sigma * k)
    dog = g1 - tau * g2
    # ソフト閾値: eps 以上は白、未満は tanh で滑らかに暗くする
    out = np.where(dog >= eps, 1.0, 1.0 + np.tanh(phi * (dog - eps)))
    return np.clip(out, 0.0, 1.0).astype(np.float32)


class PencilSketchStylizer(Stylizer):
    def __init__(self, dodge_sigma: float = 12.0, line_sigma: float = 1.2,
                 line_k: float = 1.6, line_tau: float = 0.985,
                 line_eps: float = 0.0, line_phi: float = 18.0,
                 line_strength: float = 1.0, contrast: float = 1.1,
                 tint=(1.0, 1.0, 1.0)):
        self.dodge_sigma = float(dodge_sigma)
        self.line_sigma = float(line_sigma)
        self.line_k = float(line_k)
        self.line_tau = float(line_tau)
        self.line_eps = float(line_eps)
        self.line_phi = float(line_phi)
        self.line_strength = float(np.clip(line_strength, 0.0, 1.0))
        self.contrast = float(contrast)
        self.tint = np.array(tint, dtype=np.float32).reshape(1, 1, 3)

    def stylize(self, image: np.ndarray) -> np.ndarray:
        gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY).astype(np.float32)

        # --- 陰影（ドッジ合成）---
        inv = 1.0 - gray
        blur = cv2.GaussianBlur(inv, (0, 0), sigmaX=self.dodge_sigma)
        denom = np.clip(1.0 - blur, 1e-3, 1.0)
        shade = np.clip(gray / denom, 0.0, 1.0)

        # --- 線（XDoG）---
        lines = _xdog(gray, self.line_sigma, self.line_k, self.line_tau,
                      self.line_eps, self.line_phi)
        lines = 1.0 - self.line_strength * (1.0 - lines)  # 線の濃さを調整

        # --- 合成 ---
        pencil = shade * lines
        pencil = np.clip((pencil - 0.5) * self.contrast + 0.5, 0.0, 1.0)

        rgb = np.repeat(pencil[..., None], 3, axis=2) * self.tint
        return np.clip(rgb, 0.0, 1.0).astype(np.float32)

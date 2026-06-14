"""スタイライザの共通インターフェース。

評価優先ハーネスでは「フレーム1枚を NPR 調へ変換する関数」を差し替え可能にする。
将来 neural / classic / diffusion / 作例ペア学習 を同じ口で扱えるようにするための基底。
"""

from abc import ABC, abstractmethod

import numpy as np


class Stylizer(ABC):
    """フレーム変換器の基底クラス。

    入出力はいずれも HxWx3 の float32 RGB（値域 [0, 1]）で統一する。
    """

    @abstractmethod
    def stylize(self, image: np.ndarray) -> np.ndarray:
        """1 フレームをスタイル変換して返す。

        :param image: HxWx3 float32 RGB, [0, 1]
        :return:      HxWx3 float32 RGB, [0, 1]
        """
        raise NotImplementedError

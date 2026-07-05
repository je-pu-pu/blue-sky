"""PatchGAN 識別器（GAN スタイル化フェーズ③）。

画像全体ではなく「局所パッチごと」に本物のスタイル（スタイル画像）か生成物かを判定する。
渦巻く筆致のような局所テクスチャ構造を捉えるのに向く（Gram 統計が落とす「並び・方向」を
敵対的に学習で捕まえる狙い）。学習安定化のため各 conv にスペクトル正規化を掛ける。
"""

import torch.nn as nn
from torch.nn.utils import spectral_norm


def _c(in_ch, out_ch, k, s):
    return spectral_norm(nn.Conv2d(in_ch, out_ch, k, s, padding=k // 2))


class PatchDiscriminator(nn.Module):
    def __init__(self, base_ch: int = 64):
        super().__init__()
        c = base_ch
        self.net = nn.Sequential(
            _c(3, c, 4, 2),            nn.LeakyReLU(0.2, inplace=True),
            _c(c, c * 2, 4, 2),        nn.InstanceNorm2d(c * 2, affine=True), nn.LeakyReLU(0.2, inplace=True),
            _c(c * 2, c * 4, 4, 2),    nn.InstanceNorm2d(c * 4, affine=True), nn.LeakyReLU(0.2, inplace=True),
            _c(c * 4, c * 8, 4, 1),    nn.InstanceNorm2d(c * 8, affine=True), nn.LeakyReLU(0.2, inplace=True),
            _c(c * 8, 1, 4, 1),        # パッチごとのロジット（実数。hinge 損失で使う）
        )

    def forward(self, x):
        return self.net(x)

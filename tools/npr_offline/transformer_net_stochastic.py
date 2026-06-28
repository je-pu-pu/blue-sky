"""確率的順伝播スタイル化ネット（ボイリング対応版）。

決定論版 TransformerNet（transformer_net.py）は「同じ入力→必ず同じ出力」なので、
空など静止領域が止まりボイリング（手描きの筆致の描き直し）が出ない。本クラスは
フレーム毎のノイズマップを追加入力に取り、StyleGAN 流に残差ブロックの特徴マップへ
学習スケールでノイズを足す。これにより「乱数次第で筆致の置き方が変わる」を表現でき、
推論時に時間的になめらかなノイズ場を与えると筆致が描き直されるボイリングになる。

学習では mode-seeking 多様性損失（train 側）で「ノイズを使え」を強制する。知覚損失だけ
だとノイズ無視（決定論）へ縮退するため。注入は入力ピクセルでなく特徴レベルなので、
揺れが色・明度ではなく筆致の配置に効きやすい（狙い）。

決定論版とは別クラス。既存チェックポイント（cw6 等）は従来 TransformerNet で読む。
"""

import torch
import torch.nn as nn
import torch.nn.functional as F

from transformer_net import _ConvLayer, _UpsampleConv


class _NoiseInject(nn.Module):
    """特徴マップ x [B,C,H,W] に、1ch ノイズ場を学習スケールで足す（StyleGAN B 法）。

    スケールはチャンネル毎の学習パラメータ（初期 0 = 最初はノイズ無視。多様性損失が
    徐々に持ち上げる）。ノイズは入力解像度の 1ch を各層の解像度へ最近傍リサイズして使う。
    """

    def __init__(self, ch):
        super().__init__()
        self.scale = nn.Parameter(torch.zeros(1, ch, 1, 1))

    def forward(self, x, noise):
        if noise is None:
            return x
        if noise.shape[2:] != x.shape[2:]:
            noise = F.interpolate(noise, size=x.shape[2:], mode="nearest")
        return x + self.scale * noise


class _NoisyResidualBlock(nn.Module):
    """残差ブロック（conv-IN-noise-relu-conv-IN-noise + skip）。各 conv 後にノイズ注入。"""

    def __init__(self, ch):
        super().__init__()
        self.c1 = _ConvLayer(ch, ch, 3, 1)
        self.n1 = nn.InstanceNorm2d(ch, affine=True)
        self.ni1 = _NoiseInject(ch)
        self.c2 = _ConvLayer(ch, ch, 3, 1)
        self.n2 = nn.InstanceNorm2d(ch, affine=True)
        self.ni2 = _NoiseInject(ch)
        self.relu = nn.ReLU(inplace=True)

    def forward(self, x, noise):
        y = self.relu(self.ni1(self.n1(self.c1(x)), noise))
        y = self.ni2(self.n2(self.c2(y)), noise)
        return x + y


class StochasticTransformerNet(nn.Module):
    def __init__(self, n_blocks: int = 5, base_ch: int = 32):
        super().__init__()
        c1, c2, c3 = base_ch, base_ch * 2, base_ch * 4
        self.relu = nn.ReLU(inplace=True)
        self.d1 = _ConvLayer(3, c1, 9, 1);   self.dn1 = nn.InstanceNorm2d(c1, affine=True)
        self.d2 = _ConvLayer(c1, c2, 3, 2);  self.dn2 = nn.InstanceNorm2d(c2, affine=True)
        self.d3 = _ConvLayer(c2, c3, 3, 2);  self.dn3 = nn.InstanceNorm2d(c3, affine=True)
        self.res = nn.ModuleList([_NoisyResidualBlock(c3) for _ in range(n_blocks)])
        self.u1 = _UpsampleConv(c3, c2, 3, 2); self.un1 = nn.InstanceNorm2d(c2, affine=True)
        self.u2 = _UpsampleConv(c2, c1, 3, 2); self.un2 = nn.InstanceNorm2d(c1, affine=True)
        self.out = _ConvLayer(c1, 3, 9, 1)

    def forward(self, x, noise=None):
        """:param x: [B,3,H,W] コンテンツ
        :param noise: [B,1,H,W] フレーム毎ノイズ場（None で決定論動作＝ノイズ無視）"""
        y = self.relu(self.dn1(self.d1(x)))
        y = self.relu(self.dn2(self.d2(y)))
        y = self.relu(self.dn3(self.d3(y)))
        for blk in self.res:
            y = blk(y, noise)
        y = self.relu(self.un1(self.u1(y)))
        y = self.relu(self.un2(self.u2(y)))
        y = torch.sigmoid(self.out(y))
        return y

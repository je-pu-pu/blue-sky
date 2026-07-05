"""順伝播スタイル化ネット（Johnson et al. 2016 の image transformation network）。

Gatys 法（neural_style.py）が「画像を150ステップ最適化」するのに対し、こちらは
「画像を入れたら1回の forward で絵を返す」生成器。学習でこのネットの重みを作り、
推論時は forward 一発なのでリアルタイム化できる（実機統合フェーズ1）。

構成: down-conv ×3 → 残差ブロック ×5 → up-conv ×2 → 出力。
- パディングは reflection（境界のにじみ防止）
- 正規化は InstanceNorm（スタイル転写で定番。バッチに依存せず安定）
- アップサンプルは「最近傍 interpolate + conv」（転置畳み込みのチェッカーボード回避）
- 出力は sigmoid で [0,1]（ハーネスの RGB [0,1] 規約に合わせる）
"""

import torch
import torch.nn as nn
import torch.nn.functional as F


class _ConvLayer(nn.Module):
    """reflection パディング + conv。"""

    def __init__(self, in_ch, out_ch, kernel, stride):
        super().__init__()
        pad = kernel // 2
        self.pad = nn.ReflectionPad2d(pad)
        self.conv = nn.Conv2d(in_ch, out_ch, kernel, stride)

    def forward(self, x):
        return self.conv(self.pad(x))


class _ResidualBlock(nn.Module):
    """残差ブロック（conv-IN-relu-conv-IN + skip）。"""

    def __init__(self, ch):
        super().__init__()
        self.c1 = _ConvLayer(ch, ch, 3, 1)
        self.n1 = nn.InstanceNorm2d(ch, affine=True)
        self.c2 = _ConvLayer(ch, ch, 3, 1)
        self.n2 = nn.InstanceNorm2d(ch, affine=True)
        self.relu = nn.ReLU(inplace=True)

    def forward(self, x):
        y = self.relu(self.n1(self.c1(x)))
        y = self.n2(self.c2(y))
        return x + y


class _UpsampleConv(nn.Module):
    """最近傍アップサンプル + conv（チェッカーボード回避）。"""

    def __init__(self, in_ch, out_ch, kernel, scale):
        super().__init__()
        self.scale = scale
        self.conv = _ConvLayer(in_ch, out_ch, kernel, 1)

    def forward(self, x):
        x = F.interpolate(x, scale_factor=self.scale, mode="nearest")
        return self.conv(x)


class TransformerNet(nn.Module):
    def __init__(self, n_blocks: int = 5, base_ch: int = 32):
        """:param n_blocks: 残差ブロック数（多いほど容量↑＝複雑な画風を表現できる／推論やや重い）
        :param base_ch: 基準チャンネル数（大きいほど容量↑）。各段は base, 2*base, 4*base。"""
        super().__init__()
        c1, c2, c3 = base_ch, base_ch * 2, base_ch * 4
        self.relu = nn.ReLU(inplace=True)
        # down
        self.d1 = _ConvLayer(3, c1, 9, 1);   self.dn1 = nn.InstanceNorm2d(c1, affine=True)
        self.d2 = _ConvLayer(c1, c2, 3, 2);  self.dn2 = nn.InstanceNorm2d(c2, affine=True)
        self.d3 = _ConvLayer(c2, c3, 3, 2);  self.dn3 = nn.InstanceNorm2d(c3, affine=True)
        # residual
        self.res = nn.Sequential(*[_ResidualBlock(c3) for _ in range(n_blocks)])
        # up
        self.u1 = _UpsampleConv(c3, c2, 3, 2); self.un1 = nn.InstanceNorm2d(c2, affine=True)
        self.u2 = _UpsampleConv(c2, c1, 3, 2); self.un2 = nn.InstanceNorm2d(c1, affine=True)
        self.out = _ConvLayer(c1, 3, 9, 1)

    def forward(self, x):
        y = self.relu(self.dn1(self.d1(x)))
        y = self.relu(self.dn2(self.d2(y)))
        y = self.relu(self.dn3(self.d3(y)))
        y = self.res(y)
        y = self.relu(self.un1(self.u1(y)))
        y = self.relu(self.un2(self.u2(y)))
        y = torch.sigmoid(self.out(y))
        return y

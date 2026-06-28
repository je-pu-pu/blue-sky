"""ニューラルスタイル転写（既定スタイライザ）。

torchvision の VGG19（重みは自動 DL）を使った Gatys 流の最適化ベース転写。
既定では各フレームを独立に最適化するため意図的に「フレーム間でちらつく」結果になり、
warp + 時間ブレンドの安定化がそのちらつきを抑えられるかの検証対象になる。

時間整合（案A改 / Ruder et al. 流）:
  stylize() に temporal_target（前フレーム出力を現フレームへ warp したもの）と
  temporal_mask（遮蔽マスク, 非遮蔽=1）を渡すと、損失に「前フレームとの整合項」を加える。
  初期値はコンテンツのままなので細部（筆致）は毎フレーム再生成されて鮮明さを保ち、
  整合はソフトな罰則として効く。遮蔽領域には整合をかけない。
  ※ 旧 warm-start（init に warp 前フレームを渡す方式）は、warp 補間のボケがフレーム間で
    累積し最適化も下書きで止まるため絵が眠くなった。整合は init ではなく損失で促すのが本筋。

低速なので --max-size / --steps / --max-frames で調整すること（GPU 推奨）。
"""

import numpy as np
import torch
import torch.nn.functional as F
from PIL import Image
from torchvision.models import vgg19, VGG19_Weights

from .base import Stylizer

# VGG19.features におけるレイヤ index
_CONTENT_LAYERS = ("21",)                       # conv4_2
_STYLE_LAYERS = ("0", "5", "10", "19", "28")    # conv1_1 ... conv5_1


class NeuralStyleStylizer(Stylizer):
    def __init__(
        self,
        style_image_path: str,
        device: str = None,
        steps: int = 150,
        style_weight: float = 1e6,
        content_weight: float = 1.0,
        temporal_weight: float = 0.0,
        tv_weight: float = 0.0,
        max_size: int = 384,
        style_scale: float = 1.0,
    ):
        self.device = device or ("cuda" if torch.cuda.is_available() else "cpu")
        self.steps = steps
        self.style_weight = style_weight
        self.content_weight = content_weight
        self.temporal_weight = temporal_weight
        self.tv_weight = tv_weight
        self.max_size = max_size
        # スタイル画像を VGG に通す解像度の倍率。<1 にすると渦などの大きな構造が
        # VGG の受容野に収まり、Gram に大スケール構造の統計が乗る（筆致が大きくなる）。
        self.style_scale = style_scale

        vgg = vgg19(weights=VGG19_Weights.DEFAULT).features.to(self.device).eval()
        for p in vgg.parameters():
            p.requires_grad_(False)
        self.vgg = vgg

        self.mean = torch.tensor([0.485, 0.456, 0.406], device=self.device).view(1, 3, 1, 1)
        self.std = torch.tensor([0.229, 0.224, 0.225], device=self.device).view(1, 3, 1, 1)

        self._style_img = self._load_image(style_image_path)
        self._style_cache = {}

    def _load_image(self, path: str) -> torch.Tensor:
        arr = np.asarray(Image.open(path).convert("RGB")).astype(np.float32) / 255.0
        return torch.from_numpy(arr).permute(2, 0, 1).unsqueeze(0).to(self.device)

    def _features(self, x: torch.Tensor) -> dict:
        x = (x - self.mean) / self.std
        feats = {}
        out = x
        for name, layer in self.vgg._modules.items():
            out = layer(out)
            if name in _CONTENT_LAYERS or name in _STYLE_LAYERS:
                feats[name] = out
        return feats

    @staticmethod
    def _gram(f: torch.Tensor) -> torch.Tensor:
        _, c, h, w = f.shape
        m = f.view(c, h * w)
        return (m @ m.t()) / (c * h * w)

    def _style_targets(self, hw: tuple) -> dict:
        if hw in self._style_cache:
            return self._style_cache[hw]
        # Gram は CxC でサイズ非依存なので、スタイルは content と別解像度で通してよい。
        sh = max(1, int(round(hw[0] * self.style_scale)))
        sw = max(1, int(round(hw[1] * self.style_scale)))
        s = F.interpolate(self._style_img, size=(sh, sw), mode="bilinear", align_corners=False)
        feats = self._features(s)
        grams = {l: self._gram(feats[l]).detach() for l in _STYLE_LAYERS}
        self._style_cache[hw] = grams
        return grams

    def stylize(self, image: np.ndarray, init: np.ndarray = None,
                temporal_target: np.ndarray = None, temporal_mask: np.ndarray = None,
                jitter: float = 0.0, jitter_seed: int = 0, jitter_field: np.ndarray = None) -> np.ndarray:
        """1 フレームをスタイル変換する。

        :param image:           HxWx3 float32 RGB, [0, 1]（コンテンツ＝この内容に寄せる）
        :param init:            最適化の初期値（HxWx3）。旧 warm-start 用。None ならコンテンツ自身。
        :param temporal_target: 前フレーム出力を現フレームへ warp したもの（HxWx3）。渡すと損失に
                                時間整合項を加える（案A改）。初期値はいじらないので鮮明さは保たれる。
        :param temporal_mask:   非遮蔽=1.0 のマスク（HxWx1 か HxW）。整合項をこの領域だけに効かせる。
        """
        H, W, _ = image.shape
        scale = self.max_size / max(H, W) if max(H, W) > self.max_size else 1.0
        h, w = int(round(H * scale)), int(round(W * scale))

        content = torch.from_numpy(image).permute(2, 0, 1).unsqueeze(0).to(self.device)
        content = F.interpolate(content, size=(h, w), mode="bilinear", align_corners=False)
        content_feats = {l: self._features(content)[l].detach() for l in _CONTENT_LAYERS}
        style_grams = self._style_targets((h, w))

        if init is not None:
            init_t = torch.from_numpy(np.ascontiguousarray(init)).permute(2, 0, 1).unsqueeze(0).to(self.device)
            init_t = F.interpolate(init_t, size=(h, w), mode="bilinear", align_corners=False)
            var = init_t.clamp(0.0, 1.0).clone().requires_grad_(True)
        else:
            var = content.clone()
            if jitter_field is not None:
                # コヒーレント・ボイリング: 空間/時間的になめらかな揺れ場を初期値に注入。
                # 白色ノイズ(粒状=ノイズに見える)と違い、筆致のかたまりがゆっくりウネる。
                jf = torch.from_numpy(np.ascontiguousarray(jitter_field)).permute(2, 0, 1).unsqueeze(0).to(self.device)
                jf = F.interpolate(jf, size=(h, w), mode="bilinear", align_corners=False)
                var = (var + jf).clamp(0.0, 1.0)
            elif jitter > 0.0:
                # 旧ボイリング: フレーム固有の白色ノイズを初期値に注入（粒状でノイズに見えやすい）。
                g = torch.Generator(device=self.device).manual_seed(int(jitter_seed))
                noise = torch.randn(var.shape, generator=g, device=self.device) * jitter
                var = (var + noise).clamp(0.0, 1.0)
            var = var.requires_grad_(True)

        # 時間整合の準備（前フレーム warp 目標と遮蔽マスクを最適化解像度に揃える）
        temporal_on = temporal_target is not None and self.temporal_weight > 0.0
        if temporal_on:
            tgt = torch.from_numpy(np.ascontiguousarray(temporal_target)).permute(2, 0, 1).unsqueeze(0).to(self.device)
            tgt = F.interpolate(tgt, size=(h, w), mode="bilinear", align_corners=False).clamp(0.0, 1.0)
            if temporal_mask is not None:
                m = np.ascontiguousarray(temporal_mask).astype(np.float32)
                if m.ndim == 3:
                    m = m[..., 0]
                m_t = torch.from_numpy(m).view(1, 1, *m.shape).to(self.device)
                m_t = F.interpolate(m_t, size=(h, w), mode="nearest")
            else:
                m_t = torch.ones(1, 1, h, w, device=self.device)
            t_denom = m_t.sum() * 3 + 1e-6

        opt = torch.optim.LBFGS([var], max_iter=self.steps, line_search_fn="strong_wolfe")

        def closure():
            opt.zero_grad()
            with torch.no_grad():
                var.clamp_(0.0, 1.0)
            feats = self._features(var)
            c_loss = sum(F.mse_loss(feats[l], content_feats[l]) for l in _CONTENT_LAYERS)
            s_loss = sum(F.mse_loss(self._gram(feats[l]), style_grams[l]) for l in _STYLE_LAYERS)
            loss = self.content_weight * c_loss + self.style_weight * s_loss
            if self.tv_weight > 0.0:
                # Total Variation 正則化: 隣接ピクセル差にペナルティ → 高周波ノイズを抑制。
                # style-weight を高く保ったまま「崩壊」だけを抑えられる。
                dh = (var[:, :, 1:, :] - var[:, :, :-1, :]).abs().mean()
                dw = (var[:, :, :, 1:] - var[:, :, :, :-1]).abs().mean()
                loss = loss + self.tv_weight * (dh + dw)
            if temporal_on:
                t_loss = (m_t * (var - tgt) ** 2).sum() / t_denom
                loss = loss + self.temporal_weight * t_loss
            loss.backward()
            return loss

        opt.step(closure)
        with torch.no_grad():
            var.clamp_(0.0, 1.0)
            out = F.interpolate(var.detach(), size=(H, W), mode="bilinear", align_corners=False)
        out = out.squeeze(0).permute(1, 2, 0).cpu().numpy().astype(np.float32)
        return np.clip(out, 0.0, 1.0)

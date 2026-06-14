"""ニューラルスタイル転写（既定スタイライザ）。

torchvision の VGG19（重みは自動 DL）を使った Gatys 流の最適化ベース転写。
フレームごとに独立して最適化するため意図的に「フレーム間でちらつく」結果になり、
warp + 時間ブレンドの安定化がそのちらつきを抑えられるかの検証対象になる。

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
        max_size: int = 384,
    ):
        self.device = device or ("cuda" if torch.cuda.is_available() else "cpu")
        self.steps = steps
        self.style_weight = style_weight
        self.content_weight = content_weight
        self.max_size = max_size

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
        s = F.interpolate(self._style_img, size=hw, mode="bilinear", align_corners=False)
        feats = self._features(s)
        grams = {l: self._gram(feats[l]).detach() for l in _STYLE_LAYERS}
        self._style_cache[hw] = grams
        return grams

    def stylize(self, image: np.ndarray) -> np.ndarray:
        H, W, _ = image.shape
        scale = self.max_size / max(H, W) if max(H, W) > self.max_size else 1.0
        h, w = int(round(H * scale)), int(round(W * scale))

        content = torch.from_numpy(image).permute(2, 0, 1).unsqueeze(0).to(self.device)
        content = F.interpolate(content, size=(h, w), mode="bilinear", align_corners=False)
        content_feats = {l: self._features(content)[l].detach() for l in _CONTENT_LAYERS}
        style_grams = self._style_targets((h, w))

        var = content.clone().requires_grad_(True)
        opt = torch.optim.LBFGS([var], max_iter=self.steps, line_search_fn="strong_wolfe")

        def closure():
            opt.zero_grad()
            with torch.no_grad():
                var.clamp_(0.0, 1.0)
            feats = self._features(var)
            c_loss = sum(F.mse_loss(feats[l], content_feats[l]) for l in _CONTENT_LAYERS)
            s_loss = sum(F.mse_loss(self._gram(feats[l]), style_grams[l]) for l in _STYLE_LAYERS)
            loss = self.content_weight * c_loss + self.style_weight * s_loss
            loss.backward()
            return loss

        opt.step(closure)
        with torch.no_grad():
            var.clamp_(0.0, 1.0)
            out = F.interpolate(var.detach(), size=(H, W), mode="bilinear", align_corners=False)
        out = out.squeeze(0).permute(1, 2, 0).cpu().numpy().astype(np.float32)
        return np.clip(out, 0.0, 1.0)

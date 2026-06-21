"""順伝播スタイル化ネットの学習（実機リアルタイム化フェーズ1）。

Gatys 法（neural_style.py）と同じ VGG 知覚損失を使い、TransformerNet の重みを学習する。
ペア教師データは不要: コンテンツ画像と1枚のスタイル画像があれば、content 損失（VGG 特徴）と
style 損失（Gram）でネットが「Gatys と同じ最適化先」を1回の forward で出すよう学習される。

学習後 ONNX を書き出し、エンジン側（ONNX Runtime + DirectML）で推論する。

使い方（概念実証 = 既存ダンプ120枚で過学習気味に道筋確認）:
  python train_feedforward.py --content ../../source/dump --style styles/brush_starry_night.jpg \
      --epochs 40 --train-size 256 --out-model models/brush.pth --out-onnx models/brush.onnx

注意: コンテンツが少ない/単調だと新規アングルへ汎化しない。本番は多様なレンダ画像で学習する。
損失の重み（--style-weight / --content-weight）は Gatys と同じ尺度ではない（最適化対象が
ネット重みかピクセルかで効き方が違う）ので、出力を見て調整する。
"""

import argparse
import glob
import os
import sys
import time

# torch.onnx の進捗ログに絵文字が含まれ、Windows コンソール(cp932)だと
# UnicodeEncodeError で落ちるため、標準出力を UTF-8 に切り替える。
try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except Exception:
    pass

import numpy as np
import torch
import torch.nn.functional as F
from PIL import Image
from torchvision.models import vgg19, VGG19_Weights

from transformer_net import TransformerNet

# VGG19.features の index → 通称。content は浅いほど細部を保つ:
#   "8"=relu2_2(細部強), "13"=relu3_2, "21"=conv4_2(構図のみ・細部弱)
_STYLE_LAYERS = ("0", "5", "10", "19", "28")


class _VGG(torch.nn.Module):
    """VGG19 特徴抽出（重み凍結）。content/style 損失の物差し。"""

    def __init__(self, device, content_layers):
        super().__init__()
        vgg = vgg19(weights=VGG19_Weights.DEFAULT).features.to(device).eval()
        for p in vgg.parameters():
            p.requires_grad_(False)
        self.vgg = vgg
        self.capture = set(content_layers) | set(_STYLE_LAYERS)
        self.mean = torch.tensor([0.485, 0.456, 0.406], device=device).view(1, 3, 1, 1)
        self.std = torch.tensor([0.229, 0.224, 0.225], device=device).view(1, 3, 1, 1)

    def forward(self, x):
        x = (x - self.mean) / self.std
        feats = {}
        out = x
        for name, layer in self.vgg._modules.items():
            out = layer(out)
            if name in self.capture:
                feats[name] = out
        return feats


def _gram(f):
    b, c, h, w = f.shape
    m = f.view(b, c, h * w)
    return (m @ m.transpose(1, 2)) / (c * h * w)


class _ContentDataset(torch.utils.data.Dataset):
    """ディレクトリ内の画像を学習用にランダムクロップ+反転して返す。"""

    def __init__(self, root, size):
        self.paths = sorted(glob.glob(os.path.join(root, "*.png"))) or \
                     sorted(glob.glob(os.path.join(root, "*.jpg")))
        if not self.paths:
            raise SystemExit(f"コンテンツ画像が見つかりません: {root}")
        self.size = size

    def __len__(self):
        return len(self.paths)

    def __getitem__(self, i):
        img = Image.open(self.paths[i]).convert("RGB")
        arr = np.asarray(img).astype(np.float32) / 255.0
        h, w, _ = arr.shape
        s = self.size
        # 短辺が s 未満なら拡大
        if min(h, w) < s:
            scale = s / min(h, w)
            img = img.resize((int(round(w * scale)), int(round(h * scale))), Image.BILINEAR)
            arr = np.asarray(img).astype(np.float32) / 255.0
            h, w, _ = arr.shape
        # ランダムクロップ（疑似乱数は index ベースで十分散る）
        y = (i * 37) % (h - s + 1)
        x = (i * 53) % (w - s + 1)
        crop = arr[y:y + s, x:x + s, :]
        if (i % 2) == 0:  # 水平反転で多様性
            crop = crop[:, ::-1, :].copy()
        return torch.from_numpy(crop).permute(2, 0, 1)


def main():
    ap = argparse.ArgumentParser(description="順伝播スタイル化ネットの学習（Johnson 法）")
    ap.add_argument("--content", required=True, help="コンテンツ画像ディレクトリ")
    ap.add_argument("--style", required=True, help="スタイル画像")
    ap.add_argument("--epochs", type=int, default=40)
    ap.add_argument("--batch-size", type=int, default=4)
    ap.add_argument("--train-size", type=int, default=256, help="学習クロップ解像度")
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--style-weight", type=float, default=1e10)
    ap.add_argument("--content-weight", type=float, default=1.0)
    ap.add_argument("--workers", type=int, default=6, help="DataLoader の並列ワーカ数（データ読込のボトルネック解消）")
    ap.add_argument("--init-model", default=None, help="この .pth から重みを読み込んで継続学習（高解像度ファインチューン用）")
    ap.add_argument("--n-blocks", type=int, default=5, help="残差ブロック数（容量。多いほど複雑な画風を表現）")
    ap.add_argument("--base-ch", type=int, default=32, help="基準チャンネル数（容量）")
    ap.add_argument("--content-layers", default="8",
                    help="content 評価層(VGG index, カンマ区切り)。浅いほど細部を保つ。"
                         "8=relu2_2(細部), 13=relu3_2, 21=conv4_2(構図のみ)")
    ap.add_argument("--tv-weight", type=float, default=1e-4, help="Total Variation 正則化（高周波ノイズ抑制）")
    ap.add_argument("--patience", type=int, default=3,
                    help="早期終了: total loss が改善しない epoch がこの回数続いたら停止（収束判定）。0 で無効")
    ap.add_argument("--min-delta", type=float, default=0.005,
                    help="早期終了の改善判定しきい値（相対）。0.005=0.5%未満の改善は「改善なし」扱い")
    ap.add_argument("--out-model", default="models/brush.pth")
    ap.add_argument("--out-onnx", default="models/brush.onnx")
    ap.add_argument("--onnx-size", type=int, default=384, help="ONNX 書き出し時のダミー入力解像度（推論は可変）")
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"device: {device}")
    os.makedirs(os.path.dirname(args.out_model) or ".", exist_ok=True)

    content_layers = tuple(s.strip() for s in args.content_layers.split(",") if s.strip())
    print(f"content layers: {content_layers}  (浅いほど細部を保つ)")
    vgg = _VGG(device, content_layers)
    print(f"net capacity: n_blocks={args.n_blocks} base_ch={args.base_ch}")
    net = TransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device)
    if args.init_model:
        net.load_state_dict(torch.load(args.init_model, map_location=device))
        print(f"warm-start from {args.init_model}")
    net = net.train()
    opt = torch.optim.Adam(net.parameters(), lr=args.lr)

    # スタイル目標（Gram）を学習解像度で1回計算
    s_img = np.asarray(Image.open(args.style).convert("RGB")).astype(np.float32) / 255.0
    s_t = torch.from_numpy(s_img).permute(2, 0, 1).unsqueeze(0).to(device)
    s_t = F.interpolate(s_t, size=(args.train_size, args.train_size), mode="bilinear", align_corners=False)
    with torch.no_grad():
        s_feats = vgg(s_t)
        style_grams = {l: _gram(s_feats[l]).detach() for l in _STYLE_LAYERS}

    ds = _ContentDataset(args.content, args.train_size)
    dl = torch.utils.data.DataLoader(ds, batch_size=args.batch_size, shuffle=True, drop_last=True,
                                     num_workers=args.workers, persistent_workers=args.workers > 0,
                                     pin_memory=True)
    print(f"content images: {len(ds)}  batches/epoch: {len(dl)}")

    t0 = time.time()
    best = float("inf")
    best_state = None
    stale = 0
    for ep in range(args.epochs):
        ep_c = ep_s = ep_tv = ep_total = 0.0
        for content in dl:
            content = content.to(device)
            opt.zero_grad()
            out = net(content)

            out_feats = vgg(out)
            with torch.no_grad():
                c_feats = vgg(content)
            c_loss = sum(F.mse_loss(out_feats[l], c_feats[l].detach()) for l in content_layers)
            bs = out.shape[0]
            s_loss = sum(F.mse_loss(_gram(out_feats[l]), style_grams[l].expand(bs, -1, -1)) for l in _STYLE_LAYERS)
            tv = (out[:, :, 1:, :] - out[:, :, :-1, :]).abs().mean() + \
                 (out[:, :, :, 1:] - out[:, :, :, :-1]).abs().mean()
            loss = args.content_weight * c_loss + args.style_weight * s_loss + args.tv_weight * tv
            loss.backward()
            opt.step()
            ep_c += float(c_loss); ep_s += float(s_loss) * args.style_weight
            ep_tv += float(tv); ep_total += float(loss)
        nb = len(dl)
        epoch_loss = ep_total / nb
        print(f"epoch {ep + 1}/{args.epochs}  total={epoch_loss:.3f}  content={ep_c / nb:.3f}  "
              f"style={ep_s / nb:.3f}  tv={ep_tv / nb:.4f}  ({time.time() - t0:.0f}s)")

        # 早期終了: total loss が min-delta 以上改善しない epoch が patience 回続いたら停止
        if epoch_loss < best * (1.0 - args.min_delta):
            best = epoch_loss
            best_state = {k: v.detach().cpu().clone() for k, v in net.state_dict().items()}
            stale = 0
        else:
            stale += 1
            if args.patience > 0 and stale >= args.patience:
                print(f"early stop: {args.patience} epoch 連続で改善 < {args.min_delta * 100:.1f}%（収束）")
                break

    if best_state is not None:
        net.load_state_dict(best_state)  # ベスト（最小 loss）の重みを採用
    torch.save(net.state_dict(), args.out_model)
    print(f"saved model -> {args.out_model}  (best total loss={best:.3f})")

    # ONNX 書き出し（動的な高さ・幅で推論できるよう dynamic_axes 指定）
    net.eval()
    dummy = torch.randn(1, 3, args.onnx_size, args.onnx_size, device=device)
    torch.onnx.export(
        net, dummy, args.out_onnx,
        input_names=["input"], output_names=["output"],
        dynamic_axes={"input": {0: "batch", 2: "height", 3: "width"},
                      "output": {0: "batch", 2: "height", 3: "width"}},
        opset_version=18,
    )
    print(f"saved onnx -> {args.out_onnx}")


if __name__ == "__main__":
    main()

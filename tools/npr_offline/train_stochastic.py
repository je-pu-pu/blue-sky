"""確率的スタイル化ネットの学習（ボイリング対応・mode-seeking 多様性損失）。

通常の知覚損失（content + Gram + TV）だけだとネットはノイズ入力を無視して決定論へ
縮退する（決定論が最小損失）。そこで「同じ入力に異なる2ノイズを通したら出力も違え」と
強制する mode-seeking 多様性損失を足す（Mao et al. 2019）。これでノイズが筆致の配置を
切り替える＝推論時に時間的になめらかなノイズを与えるとボイリングが出る、を狙う。

土台は決定論版から warm-start 推奨（--init-model）。共通パラメータ名なので strict=False で
読める。画質の良い決定論から始め、多様性だけを後付けで学習させると安定しやすい。

使い方:
  python train_stochastic.py --content data/val2017 --style styles/brush_starry_night.jpg \
      --init-model models/starry_cap_b8c48.pth --n-blocks 8 --base-ch 48 \
      --epochs 4 --train-size 384 --style-scale 1.5 --style-weight 3e6 --content-weight 6 \
      --div-weight 50 --out-model models/starry_boil.pth --out-onnx models/starry_boil.onnx
"""

import argparse
import os
import sys
import time

try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except Exception:
    pass

import numpy as np
import torch
import torch.nn.functional as F
from PIL import Image

# 決定論版の学習資産を再利用（VGG 知覚損失・Gram・データセット・スタイル層定義）
from train_feedforward import _VGG, _gram, _ContentDataset, _STYLE_LAYERS
from transformer_net_stochastic import StochasticTransformerNet


def main():
    ap = argparse.ArgumentParser(description="確率的スタイル化ネットの学習（ボイリング）")
    ap.add_argument("--content", required=True)
    ap.add_argument("--style", required=True)
    ap.add_argument("--epochs", type=int, default=4)
    ap.add_argument("--batch-size", type=int, default=4)
    ap.add_argument("--train-size", type=int, default=384)
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--style-weight", type=float, default=3e6)
    ap.add_argument("--content-weight", type=float, default=6.0)
    ap.add_argument("--style-scale", type=float, default=1.5)
    ap.add_argument("--tv-weight", type=float, default=1e-4)
    ap.add_argument("--div-weight", type=float, default=50.0,
                    help="mode-seeking 多様性損失の重み。大きいほどノイズで出力が変わる（ボイリング強）。"
                         "0 で無効＝決定論。縮退するなら上げ、色チラつきになるなら下げる")
    ap.add_argument("--div-layer", default="13",
                    help="多様性を測る VGG 層(index)。13=relu3_2。特徴空間で測ると色でなく筆致の配置で"
                         "差を付けやすい。'pixel' で生ピクセル空間")
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--init-model", default=None, help="決定論版 .pth から warm-start（strict=False）")
    ap.add_argument("--n-blocks", type=int, default=8)
    ap.add_argument("--base-ch", type=int, default=48)
    ap.add_argument("--content-layers", default="8")
    ap.add_argument("--out-model", default="models/starry_boil.pth")
    ap.add_argument("--out-onnx", default="models/starry_boil.onnx")
    ap.add_argument("--onnx-size", type=int, default=384)
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"device: {device}")
    os.makedirs(os.path.dirname(args.out_model) or ".", exist_ok=True)

    content_layers = tuple(s.strip() for s in args.content_layers.split(",") if s.strip())
    div_vgg = args.div_layer != "pixel"
    capture = content_layers + ((args.div_layer,) if div_vgg else ())
    vgg = _VGG(device, capture)
    print(f"content layers: {content_layers}  div: {'vgg['+args.div_layer+']' if div_vgg else 'pixel'}")

    net = StochasticTransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device)
    if args.init_model:
        missing, unexpected = net.load_state_dict(torch.load(args.init_model, map_location=device), strict=False)
        loaded = sum(1 for _ in net.state_dict()) - len(missing)
        print(f"warm-start from {args.init_model}: loaded≈{loaded}, missing(noise等)={len(missing)}, unexpected={len(unexpected)}")
    net = net.train()
    opt = torch.optim.Adam(net.parameters(), lr=args.lr)

    # スタイル Gram（アスペクト比保持で train_size*style_scale を長辺に）
    s_img = np.asarray(Image.open(args.style).convert("RGB")).astype(np.float32) / 255.0
    s_t = torch.from_numpy(s_img).permute(2, 0, 1).unsqueeze(0).to(device)
    _, _, sh, sw = s_t.shape
    target_long = int(round(args.train_size * args.style_scale))
    sc = target_long / max(sh, sw)
    s_t = F.interpolate(s_t, size=(max(1, int(round(sh*sc))), max(1, int(round(sw*sc)))),
                        mode="bilinear", align_corners=False)
    with torch.no_grad():
        s_feats = vgg(s_t)
        style_grams = {l: _gram(s_feats[l]).detach() for l in _STYLE_LAYERS}

    ds = _ContentDataset(args.content, args.train_size)
    dl = torch.utils.data.DataLoader(ds, batch_size=args.batch_size, shuffle=True, drop_last=True,
                                     num_workers=args.workers, persistent_workers=args.workers > 0,
                                     pin_memory=True)
    print(f"content images: {len(ds)}  batches/epoch: {len(dl)}")

    t0 = time.time()
    for ep in range(args.epochs):
        ep_c = ep_s = ep_div = ep_tv = 0.0
        for content in dl:
            content = content.to(device)
            b, _, h, w = content.shape
            n1 = torch.randn(b, 1, h, w, device=device)
            n2 = torch.randn(b, 1, h, w, device=device)
            opt.zero_grad()
            out1 = net(content, n1)
            out2 = net(content, n2)

            f1, f2 = vgg(out1), vgg(out2)
            with torch.no_grad():
                c_feats = vgg(content)
            # content/style は両ブランチ平均（両方を妥当な絵に保つ）
            c_loss = sum(F.mse_loss(f1[l], c_feats[l].detach()) + F.mse_loss(f2[l], c_feats[l].detach())
                         for l in content_layers) * 0.5
            s_loss = sum(F.mse_loss(_gram(f1[l]), style_grams[l].expand(b, -1, -1)) +
                         F.mse_loss(_gram(f2[l]), style_grams[l].expand(b, -1, -1))
                         for l in _STYLE_LAYERS) * 0.5
            tv = (out1[:, :, 1:, :] - out1[:, :, :-1, :]).abs().mean() + \
                 (out1[:, :, :, 1:] - out1[:, :, :, :-1]).abs().mean()

            # mode-seeking: 出力差 / ノイズ差 を最大化（= -ratio を最小化）。
            if div_vgg:
                out_dist = (f1[args.div_layer] - f2[args.div_layer]).abs().mean()
            else:
                out_dist = (out1 - out2).abs().mean()
            noise_dist = (n1 - n2).abs().mean()
            div_ratio = out_dist / (noise_dist + 1e-5)

            loss = (args.content_weight * c_loss + args.style_weight * s_loss
                    + args.tv_weight * tv - args.div_weight * div_ratio)
            loss.backward()
            opt.step()
            ep_c += float(c_loss); ep_s += float(s_loss) * args.style_weight
            ep_div += float(div_ratio); ep_tv += float(tv)
        nb = len(dl)
        # 学習したノイズスケールの平均絶対値（ノイズを使い始めたかの指標）
        scale_mag = float(torch.stack([p.abs().mean() for n, p in net.named_parameters()
                                       if n.endswith("ni1.scale") or n.endswith("ni2.scale")]).mean())
        print(f"epoch {ep+1}/{args.epochs}  content={ep_c/nb:.3f}  style={ep_s/nb:.3f}  "
              f"div_ratio={ep_div/nb:.4f}  tv={ep_tv/nb:.4f}  noise_scale={scale_mag:.4f}  ({time.time()-t0:.0f}s)")

    torch.save(net.state_dict(), args.out_model)
    print(f"saved model -> {args.out_model}")

    net.eval()
    dummy = torch.randn(1, 3, args.onnx_size, args.onnx_size, device=device)
    dummy_n = torch.randn(1, 1, args.onnx_size, args.onnx_size, device=device)
    torch.onnx.export(
        net, (dummy, dummy_n), args.out_onnx,
        input_names=["input", "noise"], output_names=["output"],
        dynamic_axes={"input": {0: "batch", 2: "height", 3: "width"},
                      "noise": {0: "batch", 2: "height", 3: "width"},
                      "output": {0: "batch", 2: "height", 3: "width"}},
        opset_version=18,
    )
    print(f"saved onnx -> {args.out_onnx}")


if __name__ == "__main__":
    main()

"""GAN スタイル化ネットの学習（フェーズ③）。

Johnson 法（Gram 知覚損失）では「点/星の質感」止まりで Gatys の渦巻く筆致が出なかった。
GAN は識別器に「本物のスタイル画像らしいか」をパッチ単位で判定させ、生成器をそれに寄せる
ことで、Gram が捉えない「並び・方向（渦の構造）」を学習させる狙い。

- 生成器: TransformerNet（--init-model で Johnson 収束モデルから warm-start 推奨）
- 識別器: PatchDiscriminator（スペクトル正規化 + hinge 損失で安定化）
- 本物サンプル: スタイル画像のランダムクロップ（1枚しかないので局所パッチの分布で代用）
- 損失: G = 敵対的(D を騙す) + content(VGG relu2_2 で構造保持) + TV / D = hinge(real/fake)

使い方:
  python train_gan.py --content data/val2017 --style styles/brush_starry_night.jpg \
    --init-model models/coco_sw3e6_conv.pth --epochs 10 --out-model models/gan.pth --out-onnx models/gan.onnx
"""

import argparse
import os
import random
import sys
import time

import numpy as np
import torch
import torch.nn.functional as F
from PIL import Image

try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except Exception:
    pass

from transformer_net import TransformerNet
from discriminator import PatchDiscriminator
from train_feedforward import _VGG, _ContentDataset

_CONTENT_LAYERS = ("8",)  # relu2_2（構造保持）


class _StyleCropDataset(torch.utils.data.Dataset):
    """スタイル画像のランダムクロップを本物サンプルとして返す（スケール・反転で多様化）。"""

    def __init__(self, path, size, length):
        self.img = Image.open(path).convert("RGB")
        self.size = size
        self.length = length

    def __len__(self):
        return self.length

    def __getitem__(self, _):
        w, h = self.img.size
        s = self.size
        # ランダムスケール: 筆致サイズに多様性を持たせる
        scale = random.uniform(1.0, 2.0)
        cw = min(w, max(s, int(s * scale)))
        ch = min(h, max(s, int(s * scale)))
        x = random.randint(0, w - cw); y = random.randint(0, h - ch)
        crop = self.img.crop((x, y, x + cw, y + ch)).resize((s, s), Image.BILINEAR)
        arr = np.asarray(crop).astype(np.float32) / 255.0
        if random.random() < 0.5:
            arr = arr[:, ::-1, :].copy()
        return torch.from_numpy(arr).permute(2, 0, 1)


def main():
    ap = argparse.ArgumentParser(description="GAN スタイル化ネットの学習（フェーズ③）")
    ap.add_argument("--content", required=True)
    ap.add_argument("--style", required=True)
    ap.add_argument("--init-model", default=None, help="生成器の warm-start 元（Johnson 収束モデル推奨）")
    ap.add_argument("--epochs", type=int, default=10)
    ap.add_argument("--batch-size", type=int, default=8)
    ap.add_argument("--train-size", type=int, default=256)
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--n-blocks", type=int, default=5)
    ap.add_argument("--base-ch", type=int, default=32)
    ap.add_argument("--lr-g", type=float, default=1e-4)
    ap.add_argument("--lr-d", type=float, default=1e-4)
    ap.add_argument("--adv-weight", type=float, default=1.0, help="敵対的損失の重み（大きいほど画風を D に強く寄せる）")
    ap.add_argument("--content-weight", type=float, default=1.0, help="content 損失（構造保持）。小さいほど画風優先")
    ap.add_argument("--tv-weight", type=float, default=1e-4)
    ap.add_argument("--save-every", type=int, default=2, help="N エポックごとに中間モデルを保存")
    ap.add_argument("--out-model", default="models/gan.pth")
    ap.add_argument("--out-onnx", default="models/gan.onnx")
    ap.add_argument("--onnx-size", type=int, default=384)
    args = ap.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"device: {device}")
    os.makedirs(os.path.dirname(args.out_model) or ".", exist_ok=True)

    vgg = _VGG(device, _CONTENT_LAYERS)
    G = TransformerNet(n_blocks=args.n_blocks, base_ch=args.base_ch).to(device)
    if args.init_model:
        G.load_state_dict(torch.load(args.init_model, map_location=device))
        print(f"G warm-start from {args.init_model}")
    D = PatchDiscriminator().to(device)
    G.train(); D.train()

    opt_g = torch.optim.Adam(G.parameters(), lr=args.lr_g, betas=(0.5, 0.999))
    opt_d = torch.optim.Adam(D.parameters(), lr=args.lr_d, betas=(0.5, 0.999))

    c_ds = _ContentDataset(args.content, args.train_size)
    s_ds = _StyleCropDataset(args.style, args.train_size, length=len(c_ds))
    mk = lambda ds: torch.utils.data.DataLoader(ds, batch_size=args.batch_size, shuffle=True,
                                                drop_last=True, num_workers=args.workers,
                                                persistent_workers=args.workers > 0, pin_memory=True)
    c_dl, s_dl = mk(c_ds), mk(s_ds)
    print(f"content={len(c_ds)} batches/epoch={len(c_dl)}")

    t0 = time.time()
    for ep in range(args.epochs):
        ed = eg = ea = ec = 0.0
        for content, style in zip(c_dl, s_dl):
            content = content.to(device); style = style.to(device)

            # --- D step: 本物(style crop)=real / 生成=fake を hinge で判定 ---
            opt_d.zero_grad()
            with torch.no_grad():
                fake = G(content)
            d_real = D(style); d_fake = D(fake)
            d_loss = F.relu(1.0 - d_real).mean() + F.relu(1.0 + d_fake).mean()
            d_loss.backward(); opt_d.step()

            # --- G step: D を騙す + content 保持 + TV ---
            opt_g.zero_grad()
            fake = G(content)
            g_adv = -D(fake).mean()
            of = vgg(fake);
            with torch.no_grad():
                cf = vgg(content)
            c_loss = sum(F.mse_loss(of[l], cf[l].detach()) for l in _CONTENT_LAYERS)
            tv = (fake[:, :, 1:, :] - fake[:, :, :-1, :]).abs().mean() + \
                 (fake[:, :, :, 1:] - fake[:, :, :, :-1]).abs().mean()
            g_loss = args.adv_weight * g_adv + args.content_weight * c_loss + args.tv_weight * tv
            g_loss.backward(); opt_g.step()

            ed += float(d_loss); eg += float(g_loss); ea += float(g_adv); ec += float(c_loss)
        nb = len(c_dl)
        print(f"epoch {ep + 1}/{args.epochs}  D={ed / nb:.3f}  G={eg / nb:.3f}  "
              f"adv={ea / nb:.3f}  content={ec / nb:.3f}  ({time.time() - t0:.0f}s)")
        if (ep + 1) % args.save_every == 0:
            torch.save(G.state_dict(), args.out_model.replace(".pth", f"_ep{ep + 1}.pth"))
            print(f"  saved intermediate -> {args.out_model.replace('.pth', f'_ep{ep + 1}.pth')}")

    torch.save(G.state_dict(), args.out_model)
    print(f"saved model -> {args.out_model}")

    G.eval()
    dummy = torch.randn(1, 3, args.onnx_size, args.onnx_size, device=device)
    torch.onnx.export(G, dummy, args.out_onnx, input_names=["input"], output_names=["output"],
                      dynamic_axes={"input": {0: "batch", 2: "height", 3: "width"},
                                    "output": {0: "batch", 2: "height", 3: "width"}},
                      opset_version=18)
    print(f"saved onnx -> {args.out_onnx}")


if __name__ == "__main__":
    main()

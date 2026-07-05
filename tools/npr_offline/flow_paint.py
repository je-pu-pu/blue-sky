"""フローガイドのストローク描画（Hertzmann 流の試作）。

neural（Gram/GAN）では「方向は出るが渦にならない」壁にぶつかった。原因は
「渦＝大スケールの曲がった構造」で、Gram は構造を測れず、PatchGAN は受容野が
小さく enforce できないこと。さらに平坦な空には追従する勾配が無い。

本手法は曲がった筆ストロークを実際に描く:
  1. content の構造テンソルから「流れの向き（エッジに沿う方向）」を出す
  2. 平坦部（コヒーレンス低）には手続き的な渦ノイズ場を合成 → 空にも渦が描ける
  3. その流れ場に沿って曲線ストロークを引く（色は元画像から採取）

リアルタイム／アートディレクション可／モデル不要。渦の巻き方をパラメータで制御できる。
"""

import argparse
import os

import cv2
import numpy as np

import io_utils


def structure_flow(gray, sigma=2.0):
    """構造テンソルから各画素の「エッジに沿う方向」(angle) とコヒーレンスを返す。"""
    gx = cv2.Sobel(gray, cv2.CV_32F, 1, 0, ksize=3)
    gy = cv2.Sobel(gray, cv2.CV_32F, 0, 1, ksize=3)
    # 構造テンソル成分を平滑化
    jxx = cv2.GaussianBlur(gx * gx, (0, 0), sigma)
    jyy = cv2.GaussianBlur(gy * gy, (0, 0), sigma)
    jxy = cv2.GaussianBlur(gx * gy, (0, 0), sigma)
    # 主固有ベクトル方向（勾配が最大の向き）。それに直交＝エッジに沿う向き
    theta = 0.5 * np.arctan2(2 * jxy, jxx - jyy)          # 勾配方向
    flow_angle = theta + np.pi / 2.0                       # エッジに沿う方向
    # コヒーレンス（構造の強さ）。低いほど平坦
    tmp = np.sqrt((jxx - jyy) ** 2 + 4 * jxy ** 2)
    coh = tmp / (jxx + jyy + 1e-6)
    return flow_angle.astype(np.float32), np.clip(coh, 0, 1).astype(np.float32)


def swirl_noise_field(h, w, scale=80.0, seed=0):
    """手続き的な渦ノイズ場（角度）。平坦部に“描かれた渦”を足すための場。

    スムーズなスカラーポテンシャルの勾配に直交を取る＝湧き出しの無い渦巻く流れ。
    """
    rng = np.random.RandomState(seed)
    pot = rng.randn(h, w).astype(np.float32)
    pot = cv2.GaussianBlur(pot, (0, 0), sigmaX=scale)
    gx = cv2.Sobel(pot, cv2.CV_32F, 1, 0, ksize=3)
    gy = cv2.Sobel(pot, cv2.CV_32F, 0, 1, ksize=3)
    return (np.arctan2(gy, gx) + np.pi / 2.0).astype(np.float32)  # 勾配に直交＝渦


def value_field(h, w, scale=80.0, freq=6.0, seed=0):
    """フロー場に沿った明度のうねり場 [-1,1]。

    渦ノイズと同じ滑らかなポテンシャルを sin で帯状にする。等値線が曲がった
    縞になり、平坦な空でも「暗い地に明るい筋」のコントラストが乗って渦が見える。
    """
    rng = np.random.RandomState(seed + 7)
    pot = rng.randn(h, w).astype(np.float32)
    pot = cv2.GaussianBlur(pot, (0, 0), sigmaX=scale)
    pot = (pot - pot.mean()) / (pot.std() + 1e-6)
    return np.sin(pot * freq).astype(np.float32)


def blend_angles(a1, a2, w):
    """角度を重み w で滑らかに合成（向きの周期性を考慮し単位ベクトルで内挿）。"""
    vx = (1 - w) * np.cos(a1) + w * np.cos(a2)
    vy = (1 - w) * np.sin(a1) + w * np.sin(a2)
    return np.arctan2(vy, vx).astype(np.float32)


def paint(img, n_strokes, length, radius, swirl_scale, swirl_amount, coh_thresh,
          seed, value_amp=0.0, value_freq=6.0, value_tint=0.0):
    """フローガイドのストローク描画。img は RGB float [0,1]。BGR uint8 で返す。"""
    h, w, _ = img.shape
    bgr = cv2.cvtColor((img * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
    gray = cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY).astype(np.float32) / 255.0

    angle, coh = structure_flow(gray, sigma=3.0)
    swirl = swirl_noise_field(h, w, scale=swirl_scale, seed=seed)
    # コヒーレンスが低い（平坦）ほど渦ノイズへ寄せる
    w_swirl = swirl_amount * (1.0 - np.clip(coh / max(coh_thresh, 1e-6), 0, 1))
    flow = blend_angles(angle, swirl, w_swirl)

    # フロー場に沿った明度のうねり。scale は渦より細かめにして縞を数本作る。
    vfield = value_field(h, w, scale=swirl_scale * 0.5, freq=value_freq, seed=seed)

    # 下地は元画像を少しぼかしたもの（隙間を埋める）。さらに明度うねりを下地にも乗せる
    # ことで、ストロークがまばらな平坦部（空）でも渦の明暗が確実に見える。
    canvas = cv2.GaussianBlur(bgr, (0, 0), sigmaX=3).astype(np.float32)
    if value_amp > 1e-6:
        # うねりは全面に一様にかける（平坦部は coherence がノイズで高く出て
        # vweight で抑制すると空に乗らないため、下地では vweight を使わない）。
        mod = vfield * value_amp                                 # HxW
        canvas[..., 0] += -mod * 0.6 - mod * value_tint * 0.5    # B: 暗部=青寄り
        canvas[..., 1] += mod * 0.5                              # G
        canvas[..., 2] += mod + mod * value_tint * 0.5          # R: 明部=黄寄り
    canvas = np.clip(canvas, 0, 255).astype(np.uint8)

    rng = np.random.RandomState(seed + 1)
    src_color = cv2.GaussianBlur(bgr, (0, 0), sigmaX=2)
    for _ in range(n_strokes):
        x = rng.randint(0, w); y = rng.randint(0, h)
        b, g, r = (float(c) for c in src_color[y, x])
        # 明度うねり: v>0 で明るく(暖色寄り) v<0 で暗く(寒色寄り)
        v = float(vfield[y, x]) * value_amp
        b = b - v * 0.6 - v * value_tint * 0.5      # 暗部=青寄り
        g = g + v * 0.5
        r = r + v + v * value_tint * 0.5            # 明部=黄寄り
        color = (int(np.clip(b, 0, 255)), int(np.clip(g, 0, 255)),
                 int(np.clip(r, 0, 255)))
        pts = [(x, y)]
        fx, fy = float(x), float(y)
        # 流れ場に沿って前方へ曲線を伸ばす
        for _ in range(length):
            a = flow[int(np.clip(fy, 0, h - 1)), int(np.clip(fx, 0, w - 1))]
            fx += np.cos(a); fy += np.sin(a)
            if not (0 <= fx < w and 0 <= fy < h):
                break
            pts.append((int(fx), int(fy)))
        # 後方へも伸ばす
        fx, fy = float(x), float(y)
        back = []
        for _ in range(length):
            a = flow[int(np.clip(fy, 0, h - 1)), int(np.clip(fx, 0, w - 1))]
            fx -= np.cos(a); fy -= np.sin(a)
            if not (0 <= fx < w and 0 <= fy < h):
                break
            back.append((int(fx), int(fy)))
        poly = np.array(back[::-1] + pts, np.int32)
        if len(poly) >= 2:
            cv2.polylines(canvas, [poly], False, color, thickness=radius, lineType=cv2.LINE_AA)
    return canvas


def main():
    ap = argparse.ArgumentParser(description="フローガイドのストローク描画（試作）")
    ap.add_argument("--frames", required=True)
    ap.add_argument("--frame-index", type=int, default=60, help="試す1フレームの番号")
    ap.add_argument("--proc-size", type=int, default=720)
    ap.add_argument("--strokes", type=int, default=12000)
    ap.add_argument("--length", type=int, default=12, help="片側のストローク長")
    ap.add_argument("--radius", type=int, default=3, help="筆の太さ")
    ap.add_argument("--swirl-scale", type=float, default=80.0, help="渦の大きさ(px)")
    ap.add_argument("--swirl-amount", type=float, default=1.0, help="平坦部を渦に寄せる強さ0..1")
    ap.add_argument("--coh-thresh", type=float, default=0.15, help="この値未満のコヒーレンスを平坦扱い")
    ap.add_argument("--value-amp", type=float, default=0.0, help="明度うねりの強さ(0で無効, 例:60)")
    ap.add_argument("--value-freq", type=float, default=6.0, help="明度うねりの細かさ(縞の本数感)")
    ap.add_argument("--value-tint", type=float, default=0.0, help="明=黄/暗=青の色味付け0..1")
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out", default="out_flowpaint")
    args = ap.parse_args()

    io_utils.ensure_dir(args.out)
    frames = io_utils.load_frames(args.frames, proc_size=args.proc_size)
    idx = min(args.frame_index, len(frames) - 1)
    img = frames[idx]
    out = paint(img, args.strokes, args.length, args.radius,
                args.swirl_scale, args.swirl_amount, args.coh_thresh, args.seed,
                value_amp=args.value_amp, value_freq=args.value_freq,
                value_tint=args.value_tint)
    inp = cv2.cvtColor((img * 255).astype(np.uint8), cv2.COLOR_RGB2BGR)
    gap = np.full((inp.shape[0], 8, 3), 255, np.uint8)
    cv2.imwrite(os.path.join(args.out, f"compare_{idx:04d}.png"), cv2.hconcat([inp, gap, out]))
    print(f"out -> {args.out}/compare_{idx:04d}.png  (左=入力 右=フローペイント)")


if __name__ == "__main__":
    main()

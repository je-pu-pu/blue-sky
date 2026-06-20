"""時間方向の評価と安定化。

- 連続フレームから光学フロー（OpenCV Farneback）を推定
- 前フレームの出力を現フレーム座標へ warp
- 前後フロー整合（forward-backward）で遮蔽マスクを作る
- 時間誤差メトリック（非遮蔽画素の warp 差分）
- warp + 時間ブレンド（EMA）による安定化ベースライン
"""

import cv2
import numpy as np

_FB_PARAMS = dict(pyr_scale=0.5, levels=3, winsize=15, iterations=3, poly_n=5, poly_sigma=1.2, flags=0)


def to_gray(image: np.ndarray) -> np.ndarray:
    return cv2.cvtColor((image * 255.0).astype(np.uint8), cv2.COLOR_RGB2GRAY)


def sharpness(seq) -> float:
    """系列の鮮明さ指標（ラプラシアン分散の平均）。高いほど細部が残っている。

    時間誤差は「ボケ・のっぺり」でも下がってしまうため、安定化が
    鮮明さを犠牲にしていないかを切り分けるために併用する。
    """
    vals = []
    for f in seq:
        g = (np.clip(f, 0.0, 1.0) * 255.0).astype(np.uint8)
        if g.ndim == 3:
            g = cv2.cvtColor(g, cv2.COLOR_RGB2GRAY)
        vals.append(float(cv2.Laplacian(g, cv2.CV_64F).var()))
    return float(np.mean(vals)) if vals else 0.0


def compute_flow(src_gray: np.ndarray, dst_gray: np.ndarray) -> np.ndarray:
    """dst の各画素が src のどこから来たかを表すフローを返す（dst を src へ写すための変位）。"""
    return cv2.calcOpticalFlowFarneback(dst_gray, src_gray, None, **_FB_PARAMS)


def warp(image: np.ndarray, flow: np.ndarray) -> np.ndarray:
    h, w = flow.shape[:2]
    gx, gy = np.meshgrid(np.arange(w), np.arange(h))
    map_x = (gx + flow[..., 0]).astype(np.float32)
    map_y = (gy + flow[..., 1]).astype(np.float32)
    return cv2.remap(image, map_x, map_y, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)


def post_boil(seq, amp: float = 0.0, scale: float = 24.0) -> list:
    """安定化後の各フレームを微小ワープして手描きの「筆致のゆらぎ（ボイリング）」を一律に乗せる。

    最適化や安定化には触れず、出来上がった各フレームをフレーム固有のなめらかな低周波変位場で
    ずらすだけ。色はそのまま、絵が少しウネる＝紙の上で線/塗りが揺れる質感。安定化の後段なので
    warp と干渉せず残像を生まない（boil を最適化初期値に入れると二重写り＝残像になるのを回避）。

    :param amp:   ゆらぎの振幅(px)。0 で無効。例 0.5〜3。
    :param scale: 変位場の空間スケール(px)。大きいほど大きくゆったりウネる。小さいと細かく震える。
    """
    if amp <= 0.0:
        return seq
    out = []
    h, w = seq[0].shape[:2]
    for t, f in enumerate(seq):
        # フレーム index をシードに再現性のある低周波変位場を作る（粗い乱数→ぼかし）
        rng = np.random.RandomState(t)
        dx = cv2.GaussianBlur(rng.randn(h, w).astype(np.float32), (0, 0), sigmaX=scale)
        dy = cv2.GaussianBlur(rng.randn(h, w).astype(np.float32), (0, 0), sigmaX=scale)
        # 平均振幅を amp(px) に正規化
        dx *= amp / (np.abs(dx).mean() + 1e-6)
        dy *= amp / (np.abs(dy).mean() + 1e-6)
        flow = np.stack([dx, dy], axis=-1)
        out.append(warp(f, flow))
    return out


def coherent_boil_fields(n: int, h: int, w: int, amp: float, space: float, time_k: int) -> list:
    """空間/時間的になめらかな揺れ場を n フレーム分作る（コヒーレント・ボイリング用）。

    白色ノイズ(ピクセル粒状=ノイズに見える)と違い、(1)空間的にぼかして筆致サイズのかたまりにし、
    (2)時間的にキーフレーム間を補間してゆっくり変化させる。結果、筆致がゆっくりウネる手描き感。

    :param amp:    揺れ場の振幅（ピクセル値 [0,1] に対する量。例 0.02〜0.08）
    :param space:  空間ぼかし sigma(px)。大きいほど大きなかたまり単位で揺れる
    :param time_k: 時間キーフレーム間隔(フレーム)。大きいほどゆっくり変化
    """
    if amp <= 0.0:
        return [None] * n
    # キーフレームごとに空間スムーズなノイズを作り、フレーム間は線形補間
    n_key = n // max(1, time_k) + 2
    keys = []
    for k in range(n_key):
        rng = np.random.RandomState(1000 + k)
        f = rng.randn(h, w, 3).astype(np.float32)
        for c in range(3):
            f[..., c] = cv2.GaussianBlur(f[..., c], (0, 0), sigmaX=space)
        f *= amp / (np.abs(f).mean() + 1e-6)
        keys.append(f)
    fields = []
    for t in range(n):
        pos = t / max(1, time_k)
        i = int(pos); frac = pos - i
        fields.append((1.0 - frac) * keys[i] + frac * keys[i + 1])
    return fields


def occlusion_mask(prev_gray: np.ndarray, cur_gray: np.ndarray, flow_cur_to_prev: np.ndarray, thresh: float = 1.5) -> np.ndarray:
    """非遮蔽=1.0 / 遮蔽=0.0 のマスク（HxWx1）。前後フローの整合で判定。"""
    flow_prev_to_cur = cv2.calcOpticalFlowFarneback(prev_gray, cur_gray, None, **_FB_PARAMS)
    warped = warp(flow_prev_to_cur, flow_cur_to_prev)
    diff = flow_cur_to_prev + warped
    err = np.sqrt((diff ** 2).sum(axis=-1))
    mask = (err < thresh).astype(np.float32)
    return mask[..., None]


def temporal_errors(seq, grays) -> list:
    """各フレーム t について |seq[t] - warp(seq[t-1])| を非遮蔽画素で平均した列。小さいほど安定。"""
    errs = []
    for t in range(1, len(seq)):
        flow = compute_flow(grays[t - 1], grays[t])
        warped_prev = warp(seq[t - 1], flow)
        mask = occlusion_mask(grays[t - 1], grays[t], flow)
        d = np.abs(seq[t] - warped_prev) * mask
        denom = mask.sum() * seq[t].shape[2] + 1e-6
        errs.append(float(d.sum() / denom))
    return errs


def temporal_errors_pre(seq, flows, masks) -> list:
    """temporal_errors の事前計算フロー/マスク版（G-buffer モーションベクトル用）。"""
    errs = []
    for t in range(1, len(seq)):
        warped_prev = warp(seq[t - 1], flows[t])
        mask = masks[t]
        d = np.abs(seq[t] - warped_prev) * mask
        denom = mask.sum() * seq[t].shape[2] + 1e-6
        errs.append(float(d.sum() / denom))
    return errs


def stabilize_pre(naive_seq, flows, masks, alpha: float = 0.6, reject_sigma: float = 0.0,
                  reject_power: float = 2.0, warp_reject: float = 0.0, warp_reject_feather: float = 8.0) -> list:
    """stabilize の事前計算フロー/マスク版（G-buffer モーションベクトル用）。

    :param reject_sigma: >0 で「残像リジェクション」を有効化。warp した前フレームと現フレームの
        生スタイル化が食い違う画素ほど α を自動で下げる（今フレーム寄せ）。大きく変化した領域
        （前フレームに正しい対応が無い＝残像になる所）を弾く。小さいほど厳しく弾く（例 0.05〜0.2）。
    :param reject_power: 減衰カーブの指数。2 はなだらかな丘、4〜6 で「閾値未満はほぼ無傷・超えたら
        一気に弾く」崖型になり、中程度の変化を守りつつ残像級の大変化だけ落とせる（案1）。
    :param warp_reject: >0 で「warp 整合マスク」を有効化（案2）。warp した前フレームと現フレームの
        食い違いがこの閾値を超える画素を mask=0（＝前フレームを使わず今フレームで描き直す）にする。
        遮蔽が外れた/大きく変形した「対応の無い領域」＝残像の真因を直接落とす。
    :param warp_reject_feather: warp 整合マスクのぼかし量(px)。硬いカットの継ぎ目をなめらかにする。
    """
    out = [naive_seq[0].copy()]
    for t in range(1, len(naive_seq)):
        warped_prev = warp(out[t - 1], flows[t])
        mask = masks[t]
        if warp_reject > 0.0:
            # 案2: warp 整合。前フレームと現フレームが大きく食い違う＝対応が無い領域を mask から除外。
            diff = np.abs(warped_prev - naive_seq[t]).mean(axis=-1, keepdims=True)
            keep = (diff < warp_reject).astype(np.float32)
            if warp_reject_feather > 0.0:
                keep = cv2.GaussianBlur(keep[..., 0], (0, 0), sigmaX=warp_reject_feather)[..., None]
            mask = mask * keep
        if reject_sigma > 0.0:
            # 案1: 内容の食い違い（チャンネル平均）が大きいほどブレンド率を指数減衰させる
            diff = np.abs(warped_prev - naive_seq[t]).mean(axis=-1, keepdims=True)
            a_eff = alpha * np.exp(-(diff / reject_sigma) ** reject_power)
        else:
            a_eff = alpha
        blended = (1.0 - a_eff) * naive_seq[t] + a_eff * warped_prev
        cur = mask * blended + (1.0 - mask) * naive_seq[t]
        out.append(np.clip(cur, 0.0, 1.0))
    return out


def stabilize(naive_seq, grays, alpha: float = 0.6) -> list:
    """warp + 時間ブレンド（EMA）。stable[t] = (1-a)*naive[t] + a*warp(stable[t-1])。遮蔽画素は naive にフォールバック。"""
    out = [naive_seq[0].copy()]
    for t in range(1, len(naive_seq)):
        flow = compute_flow(grays[t - 1], grays[t])
        warped_prev = warp(out[t - 1], flow)
        mask = occlusion_mask(grays[t - 1], grays[t], flow)
        blended = (1.0 - alpha) * naive_seq[t] + alpha * warped_prev
        cur = mask * blended + (1.0 - mask) * naive_seq[t]
        out.append(np.clip(cur, 0.0, 1.0))
    return out

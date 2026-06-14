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


def compute_flow(src_gray: np.ndarray, dst_gray: np.ndarray) -> np.ndarray:
    """dst の各画素が src のどこから来たかを表すフローを返す（dst を src へ写すための変位）。"""
    return cv2.calcOpticalFlowFarneback(dst_gray, src_gray, None, **_FB_PARAMS)


def warp(image: np.ndarray, flow: np.ndarray) -> np.ndarray:
    h, w = flow.shape[:2]
    gx, gy = np.meshgrid(np.arange(w), np.arange(h))
    map_x = (gx + flow[..., 0]).astype(np.float32)
    map_y = (gy + flow[..., 1]).astype(np.float32)
    return cv2.remap(image, map_x, map_y, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)


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

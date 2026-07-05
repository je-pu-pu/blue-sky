"""G-buffer（エンジンのモーションベクトル）ベースの正確なフロー（案B / 段階0b）。

エンジンが `dump/motion_%04d.raw` に書き出した NDC 速度（cur_ndc - prev_ndc, RG float32,
レンダ解像度）を読み、`temporal.warp` が使う「cur→prev フロー（画素）」へ変換する。
Farneback 推定と違いカメラ運動が正確なため、warp のボケ/固まりが出ない。

motion ファイルはフレーム t が「cur=t, prev=t-1」の速度。t=0 は prev=cur のため速度≈0。
"""

import glob
import os

import cv2
import numpy as np


def _native_size(color_path: str):
    img = cv2.imread(color_path, cv2.IMREAD_COLOR)
    if img is None:
        raise FileNotFoundError(color_path)
    return img.shape[1], img.shape[0]  # (W, H)


def load_motion_raw(path: str, w: int, h: int) -> np.ndarray:
    """RG float32 の生バイナリを HxWx2 の NDC 速度として読む。"""
    data = np.fromfile(path, dtype=np.float32)
    if data.size != w * h * 2:
        raise ValueError(f"{path}: float 数 {data.size} != 期待 {w * h * 2}（解像度不一致）")
    return data.reshape(h, w, 2)  # [...,0]=vel_ndc.x, [...,1]=vel_ndc.y


def motion_to_flow(vel_ndc: np.ndarray, out_w: int, out_h: int) -> np.ndarray:
    """NDC 速度 → cur→prev フロー（画素, 出力解像度）。

    画素 px=(ndc.x*0.5+0.5)*W, py=(0.5-ndc.y*0.5)*H より
    flow_cur_to_prev = prev_px - cur_px = (-vel_ndc.x*0.5*W, +vel_ndc.y*0.5*H)。
    """
    h, w = vel_ndc.shape[:2]
    fx = -vel_ndc[..., 0] * 0.5 * w
    fy = vel_ndc[..., 1] * 0.5 * h
    flow = np.stack([fx, fy], axis=-1).astype(np.float32)

    if (w, h) != (out_w, out_h):
        sx, sy = out_w / float(w), out_h / float(h)
        flow = cv2.resize(flow, (out_w, out_h), interpolation=cv2.INTER_LINEAR)
        flow[..., 0] *= sx
        flow[..., 1] *= sy
    return flow


def occlusion_from_flow(flow: np.ndarray, grad_thresh: float = 2.0,
                        feather: float = 0.0) -> np.ndarray:
    """簡易遮蔽マスク（非遮蔽=1）。画面外参照とフロー不連続（物体/深度境界）を 0 にする。

    :param feather: >0 でマスクを Gaussian ぼかしして 0/1 の硬い段差をなめらかな勾配にする。
                    安定化合成で「明滅する境界帯」と「静かな領域」の硬い継ぎ目（境界ノイズ）を消す。
    """
    h, w = flow.shape[:2]
    gx, gy = np.meshgrid(np.arange(w), np.arange(h))
    sx = gx + flow[..., 0]
    sy = gy + flow[..., 1]
    in_bounds = (sx >= 0) & (sx <= w - 1) & (sy >= 0) & (sy <= h - 1)

    # フローの空間勾配が大きい=遮蔽境界
    dxx = cv2.Sobel(flow[..., 0], cv2.CV_32F, 1, 0, ksize=3)
    dxy = cv2.Sobel(flow[..., 0], cv2.CV_32F, 0, 1, ksize=3)
    dyx = cv2.Sobel(flow[..., 1], cv2.CV_32F, 1, 0, ksize=3)
    dyy = cv2.Sobel(flow[..., 1], cv2.CV_32F, 0, 1, ksize=3)
    grad = np.sqrt(dxx ** 2 + dxy ** 2 + dyx ** 2 + dyy ** 2)
    smooth = grad < grad_thresh

    mask = (in_bounds & smooth).astype(np.float32)
    if feather > 0.0:
        mask = cv2.GaussianBlur(mask, (0, 0), sigmaX=feather)
    return mask[..., None]


def available(frames_dir: str) -> bool:
    return bool(glob.glob(os.path.join(frames_dir, "motion_*.raw")))


def load_flows_and_masks(frames_dir: str, frame_shapes: list, color_pattern: str = "color_*.png",
                         feather: float = 0.0):
    """各フレーム t の (flow_cur_to_prev, mask) のリストを返す。t=0 はゼロフロー/全 1 マスク。

    :param frame_shapes: 処理後フレームの shape リスト（出力解像度を決める）
    :param feather: 遮蔽マスクのフェザー量（境界ノイズ抑制）。occlusion_from_flow に渡す。
    :return: (flows, masks) or None（motion ファイルが無い場合）
    """
    color_paths = sorted(glob.glob(os.path.join(frames_dir, color_pattern)))
    if not color_paths:
        color_paths = sorted(glob.glob(os.path.join(frames_dir, "*.png")))
    motion_paths = sorted(glob.glob(os.path.join(frames_dir, "motion_*.raw")))
    if not motion_paths or not color_paths:
        return None

    w, h = _native_size(color_paths[0])

    flows, masks = [], []
    for t in range(len(frame_shapes)):
        out_h, out_w = frame_shapes[t][:2]
        if t == 0 or t >= len(motion_paths):
            flows.append(np.zeros((out_h, out_w, 2), np.float32))
            masks.append(np.ones((out_h, out_w, 1), np.float32))
            continue
        vel = load_motion_raw(motion_paths[t], w, h)
        flow = motion_to_flow(vel, out_w, out_h)
        flows.append(flow)
        masks.append(occlusion_from_flow(flow, feather=feather))
    return flows, masks

"""入出力ユーティリティ（連番読み込み・画像/動画保存）。

画像は内部的に HxWx3 float32 RGB [0,1] で扱う。OpenCV は BGR なので境界で変換する。
"""

import glob
import os

import cv2
import numpy as np


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def _resize_longest(image: np.ndarray, max_size: int) -> np.ndarray:
    """最長辺が max_size を超える場合のみ等比縮小する。"""
    h, w = image.shape[:2]
    longest = max(h, w)
    if max_size and longest > max_size:
        scale = max_size / float(longest)
        new_w, new_h = int(round(w * scale)), int(round(h * scale))
        image = cv2.resize(image, (new_w, new_h), interpolation=cv2.INTER_AREA)
    return image


def load_frames(frames_dir: str, pattern: str = "color_*.png", max_frames: int = None,
                proc_size: int = None) -> list:
    paths = sorted(glob.glob(os.path.join(frames_dir, pattern)))
    if not paths:
        paths = sorted(glob.glob(os.path.join(frames_dir, "*.png")))
    if max_frames:
        paths = paths[:max_frames]

    frames = []
    for p in paths:
        bgr = cv2.imread(p, cv2.IMREAD_COLOR)
        if bgr is None:
            continue
        bgr = _resize_longest(bgr, proc_size)
        rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0
        frames.append(rgb)
    return frames


def _to_uint8(image: np.ndarray) -> np.ndarray:
    return np.clip(image * 255.0, 0, 255).astype(np.uint8)


def save_image(path: str, image_rgb_float: np.ndarray) -> None:
    cv2.imwrite(path, cv2.cvtColor(_to_uint8(image_rgb_float), cv2.COLOR_RGB2BGR))


def save_video(path: str, frames_rgb_float: list, fps: int = 24) -> None:
    if not frames_rgb_float:
        return
    h, w = frames_rgb_float[0].shape[:2]
    writer = cv2.VideoWriter(path, cv2.VideoWriter_fourcc(*"mp4v"), fps, (w, h))
    for f in frames_rgb_float:
        writer.write(cv2.cvtColor(_to_uint8(f), cv2.COLOR_RGB2BGR))
    writer.release()

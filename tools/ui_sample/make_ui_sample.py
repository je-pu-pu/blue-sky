"""UI のサンプルテーマ用の画像（角丸の枠）を生成する（標準ライブラリのみ）

使い方（リポジトリのルートで）:
  python tools/ui_sample/make_ui_sample.py source/media/texture/ui/sample

- 4x4 のスーパーサンプリングで縁をなめらかにする
- 透明な部分の RGB にも縁の色を入れておく（UI 用の読み込みは乗算済みアルファにするので必須ではないが、
  ほかのツールで表示したときに縁が黒くならないように）
- サンプルテーマは source/media/ui/sample_theme.txt
"""
import os
import struct
import sys
import zlib

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)


def write_png(path, w, h, pixels):
    raw = b''.join(b'\x00' + bytes(pixels[y * w * 4:(y + 1) * w * 4]) for y in range(h))

    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)

    data = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) \
        + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    with open(path, 'wb') as f:
        f.write(data)


def inside_round_rect(x, y, x0, y0, x1, y1, r):
    if x < x0 or x > x1 or y < y0 or y > y1:
        return False
    cx = min(max(x, x0 + r), x1 - r)
    cy = min(max(y, y0 + r), y1 - r)
    return (x - cx) ** 2 + (y - cy) ** 2 <= r * r


def frame(name, w, h, radius, border, fill_top, fill_bottom, border_color):
    ss = 4
    px = []
    for y in range(h):
        t = y / max(h - 1, 1)
        fill = [round(fill_top[i] * (1 - t) + fill_bottom[i] * t) for i in range(4)]
        for x in range(w):
            outer = inner = 0
            for sy in range(ss):
                for sx in range(ss):
                    fx = x + (sx + 0.5) / ss
                    fy = y + (sy + 0.5) / ss
                    if inside_round_rect(fx, fy, 0, 0, w, h, radius):
                        outer += 1
                        if inside_round_rect(fx, fy, border, border, w - border, h - border, max(radius - border, 0)):
                            inner += 1
            n = ss * ss
            a_outer = outer / n
            a_inner = inner / n
            # 枠の色の上に中身を重ねる（中身の割合 = a_inner / a_outer）
            k = a_inner / a_outer if a_outer > 0 else 0
            rgb = [round(border_color[i] * (1 - k) + fill[i] * k) for i in range(3)]
            alpha = (border_color[3] * (1 - k) + fill[3] * k) * a_outer
            if a_outer == 0:
                rgb = list(border_color[:3])  # 透明部分にも縁の色を入れてにじみを防ぐ
            px += rgb + [round(alpha)]
    write_png(os.path.join(OUT, name), w, h, px)
    print('生成:', name)


def ornate(name, size=48, slice_=16):
    """辺に飾りの丸が 16px ごとに並ぶ枠（9 分割の辺を repeat / round で並べる見本）

    9 分割の辺の 1 区間（slice_ ～ size - slice_）にちょうど 1 個の丸が入るようにしてある。
    引き伸ばすと丸が楕円に崩れ、並べると丸のまま増える。
    """
    ss = 4
    px = []
    band = 10                       # 枠の帯の太さ
    fill = (14, 18, 30, 235)
    band_color = (40, 60, 110, 255)
    dot_color = (170, 205, 255, 255)
    for y in range(size):
        for x in range(size):
            acc = [0.0, 0.0, 0.0, 0.0]
            for sy in range(ss):
                for sx in range(ss):
                    fx = x + (sx + 0.5) / ss
                    fy = y + (sy + 0.5) / ss
                    edge_dist = min(fx, fy, size - fx, size - fy)
                    if edge_dist < band:
                        c = band_color
                        # 帯の中心線上、区間ごとの中央に丸（四隅にも 1 個）
                        cx = (fx % slice_) - slice_ / 2
                        cy = (fy % slice_) - slice_ / 2
                        if edge_dist < band and (cx * cx + cy * cy) <= 3.2 ** 2 and abs(edge_dist - band / 2) < 4:
                            c = dot_color
                    else:
                        c = fill
                    for i in range(3):
                        acc[i] += c[i] * c[3] / 255
                    acc[3] += c[3]
            n = ss * ss
            a = acc[3] / n
            rgb = [round(acc[i] / n * 255 / a) if a > 0 else 0 for i in range(3)]
            px += rgb + [round(a)]
    write_png(os.path.join(OUT, name), size, size, px)
    print('生成:', name)


frame('panel.png', 64, 64, 16, 3, (22, 28, 44, 240), (12, 15, 24, 240), (110, 160, 255, 255))
ornate('panel_ornate.png')
frame('row.png', 48, 48, 10, 2, (44, 50, 68, 220), (34, 38, 52, 220), (70, 80, 110, 255))
frame('row_focused.png', 48, 48, 10, 3, (70, 96, 150, 245), (48, 66, 108, 245), (170, 210, 255, 255))
frame('row_pressed.png', 48, 48, 10, 3, (100, 135, 200, 250), (80, 110, 170, 250), (220, 240, 255, 255))
frame('track.png', 32, 32, 8, 1, (8, 10, 16, 255), (14, 16, 24, 255), (56, 62, 80, 255))
frame('accent.png', 32, 32, 8, 1, (110, 170, 255, 255), (60, 110, 230, 255), (150, 200, 255, 255))
frame('knob.png', 24, 48, 10, 2, (250, 252, 255, 255), (205, 215, 235, 255), (60, 110, 230, 255))

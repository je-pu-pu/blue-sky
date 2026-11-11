"""MIDI の音源（SoundFont）FluidR3_GM.sf2 を入手して source/media/music に置く。

大きい（約 141 MB）ので git には入れない。代わりにこのスクリプトで、誰でも同じものを手に入れられるようにする。

入手元: Debian の fluid-soundfont-gm パッケージ（Frank Wen の Fluid R3 GM。MIT ライセンス）
  https://deb.debian.org/debian/pool/main/f/fluid-soundfont/fluid-soundfont-gm_3.1-6_all.deb
  .deb の SHA-256 は https://packages.debian.org/sid/all/fluid-soundfont-gm/download の値と同じ。

使い方（リポジトリの根から。標準ライブラリだけで動く）:
  python -I tools/soundfont/fetch_soundfont.py
  python -I tools/soundfont/fetch_soundfont.py --deb <手元の .deb>   # ダウンロードせず手元のものを使う

.deb は ar 形式で、中の data.tar.* に usr/share/sounds/sf2/FluidR3_GM.sf2 が入っている。
.deb と取り出した SF2 の両方のハッシュを確かめ、違えば置かずに止まる。
"""

import argparse
import hashlib
import io
import lzma
import os
import sys
import tarfile
import tempfile
import urllib.request

DEB_URL = "https://deb.debian.org/debian/pool/main/f/fluid-soundfont/fluid-soundfont-gm_3.1-6_all.deb"
DEB_SHA256 = "9965fbcc6acee17d6f72b685d28c7f968ec64eba14645445b476d5c6cc2eee4c"
DEB_SIZE = 119613032

SF2_MEMBER = "usr/share/sounds/sf2/FluidR3_GM.sf2"
SF2_SHA256 = "74594e8f4250680adf590507a306655a299935343583256f3b722c48a1bc1cb0"  # 148398306 バイト
SF2_NAME = "FluidR3_GM.sf2"

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DEFAULT_OUT = os.path.join(REPO_ROOT, "source", "media", "music", SF2_NAME)


def sha256_of_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_of_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def download(url, path):
    print(f"ダウンロード中: {url}")
    with urllib.request.urlopen(url) as res, open(path, "wb") as f:
        while True:
            chunk = res.read(1 << 20)
            if not chunk:
                break
            f.write(chunk)


def read_ar_members(data):
    """ar 形式（.deb）の中身を {名前: バイト列} で返す。"""
    if not data.startswith(b"!<arch>\n"):
        raise ValueError(".deb（ar 形式）ではない")
    members = {}
    pos = 8
    while pos + 60 <= len(data):
        header = data[pos:pos + 60]
        name = header[0:16].decode("ascii").strip().rstrip("/")
        size = int(header[48:58].decode("ascii").strip())
        pos += 60
        members[name] = data[pos:pos + size]
        pos += size + (size & 1)  # 2 バイト境界にそろえる
    return members


def open_data_tar(members):
    for name, body in members.items():
        if not name.startswith("data.tar"):
            continue
        if name.endswith(".xz"):
            return tarfile.open(fileobj=io.BytesIO(lzma.decompress(body)))
        if name.endswith(".gz"):
            return tarfile.open(fileobj=io.BytesIO(body), mode="r:gz")
        if name.endswith(".zst"):
            from compression import zstd  # Python 3.14 以降
            return tarfile.open(fileobj=io.BytesIO(zstd.decompress(body)))
        return tarfile.open(fileobj=io.BytesIO(body))
    raise ValueError(".deb に data.tar.* が無い")


def main():
    # Windows の端末で日本語が化けないように、出力を UTF-8 にそろえる
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8")

    parser = argparse.ArgumentParser(description="FluidR3_GM.sf2 を入手して置く")
    parser.add_argument("--deb", help="手元の .deb を使う（ダウンロードしない）")
    parser.add_argument("--out", default=DEFAULT_OUT, help=f"置き場所（既定: {DEFAULT_OUT}）")
    args = parser.parse_args()

    if os.path.exists(args.out) and SF2_SHA256 and sha256_of_file(args.out) == SF2_SHA256:
        print(f"すでに置いてある（ハッシュ一致）: {args.out}")
        return 0

    if args.deb:
        deb_path = args.deb
    else:
        work_dir = tempfile.mkdtemp(prefix="blue-sky-sf2-")
        deb_path = os.path.join(work_dir, os.path.basename(DEB_URL))
        download(DEB_URL, deb_path)

    with open(deb_path, "rb") as f:
        deb = f.read()
    if len(deb) != DEB_SIZE or sha256_of_bytes(deb) != DEB_SHA256:
        print(f"失敗: .deb のハッシュが違う（{sha256_of_bytes(deb)}、{len(deb)} バイト）", file=sys.stderr)
        return 1
    print(".deb のハッシュを確認した")

    with open_data_tar(read_ar_members(deb)) as tar:
        names = {m.name.lstrip("./"): m for m in tar.getmembers()}
        member = names.get(SF2_MEMBER)
        if member is None:
            print(f"失敗: .deb の中に {SF2_MEMBER} が無い", file=sys.stderr)
            return 1
        sf2 = tar.extractfile(member).read()

    digest = sha256_of_bytes(sf2)
    if SF2_SHA256 is None:
        print(f"SF2 の SHA-256（まだ書いていない）: {digest}  {len(sf2)} バイト")
    elif digest != SF2_SHA256:
        print(f"失敗: SF2 のハッシュが違う（{digest}）", file=sys.stderr)
        return 1
    else:
        print("SF2 のハッシュを確認した")

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    tmp = args.out + ".part"
    with open(tmp, "wb") as f:
        f.write(sf2)
    os.replace(tmp, args.out)
    print(f"完了: {args.out}（{len(sf2)} バイト）")
    return 0


if __name__ == "__main__":
    sys.exit(main())

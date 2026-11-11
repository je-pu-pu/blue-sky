"""ステージの前後の文と背景の古い形式（.intro・.outro）を TOML（.intro.toml・.outro.toml）に変える

使い方:
    python tools/stage_convert/convert_story_text.py <.intro・.outro のファイル か フォルダ>...
    （フォルダなら中の *.intro・*.outro を全部。書き出すのは同じ場所の <名前>.toml）

    --check   書き出さずに、変換と読み直しの確かめだけをする

決まり（doc/ステージ形式の設計.md の 7 章）:
    - 標準ライブラリだけで動く（Python 3.11 以上。tomllib で読み直して確かめる）
    - 文字コードは UTF-8（BOM があれば外す）で読み、読めなければ CP932 で読む。書き出すのは UTF-8
    - コメント（# で始まる行）は写す。数は書いてある文字列のまま写す（convert_stage.py の Num）
    - layer-rotate は回転（rotation）に写さず、コメントで残して報告に出す。以前の読み方は layer-rotation という
      名前しか読まず、同梱の全ファイルにある layer-rotate を黙って無視していた（2013-08 から一度も回っていない）。
      利用者の決定（2026-11-11）で、回っていなかった見た目を保つ。写すと、古い .intro を変換し直したときにまた回ってしまう。
      回したい場面は TOML に rotation を書けば回る（layer-rotation と書いた行は rotation に写す）
    - 以前の読み方が読まなかった行・余った語は、TOML の値に書かずコメントで残し、報告に出す
    - 本文（text 行より後）は、以前の読み方が作った文字列と同じになるように写す
      （以前は 1 行ごとに改行を足し、ファイルが改行で終わると最後にもう 1 つ改行を足していた）
"""

import json
import pathlib
import sys
import tomllib

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from convert_stage import Num, is_number, render  # noqa: E402

# 背景の重なりの動き（値・目標・速さ）の既定の値（core/graphics/BgSpriteLayer.cpp と同じ）
LAYER_DEFAULTS = {
    "translation": ([0.0, 0.0], [0.0, 0.0], [0.0, 0.0]),
    "rotation": (0.0, 0.0, 0.0),
    "scale": (1.0, 1.0, 0.0),
    "color": ([1.0, 1.0, 1.0, 1.0], [1.0, 1.0, 1.0, 1.0], [0.0, 0.0, 0.0, 0.0]),
}

# 古い形式の layer-* の名前 → TOML のキー
LAYER_COMMANDS = {
    "layer-translation": "translation",
    "layer-rotation": "rotation",
    "layer-scale": "scale",
    "layer-color": "color",
}

# 1 つの動きの数の個数（値・目標・速さのそれぞれ）
WIDTH = {"translation": 2, "rotation": 1, "scale": 1, "color": 4}

# 先頭に置く 1 つの値のコマンド → TOML のキー
SINGLE_COMMANDS = {
    "sound": "sound",
    "next-scene": "next_scene",
    "next-stage-name": "next_stage",
}


def read_text(path, report):
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    try:
        text = raw.decode("utf-8")
        encoding = "UTF-8"
    except UnicodeDecodeError:
        text = raw.decode("cp932")
        encoding = "CP932"
        report.append(f"{path.name}: CP932 で読んだ（UTF-8 で書き出す）")
    return text.replace("\r\n", "\n").replace("\r", "\n"), encoding


def leading_numbers(words):
    """stringstream で数を読むのと同じく、読めない語の手前までの数"""
    out = []
    for w in words:
        if not is_number(w):
            break
        out.append(w)
    return out


def convert(path):
    """1 つのファイルを変換する。(TOML の文字列, 報告の行, 確かめ用の中身, 文字コード) を返す"""
    report = []
    text, encoding = read_text(path, report)

    def note(line_no, message):
        report.append(f"{path.name}:{line_no}: {message}")

    head = {}                 # sound・bgm・next_scene・next_stage
    head_comments = []
    layers = []               # {"image", "src_rect", 動き…, "comments", "after"}
    text_table = {}           # color・border_color
    text_comments = []
    body = None
    pending = []              # まだ置き場の決まらないコメント

    def take_pending():
        nonlocal pending
        c, pending = pending, []
        return c

    lines = text.split("\n")
    line_no = 0
    while line_no < len(lines):
        line = lines[line_no]
        line_no += 1
        words = line.split()
        if not words:
            continue
        command, args = words[0], words[1:]

        if command.startswith("#"):
            pending.append("# " + line.strip()[1:].strip() if line.strip() != "#" else "#")
            continue

        def extra(used):
            if len(args) > used:
                note(line_no, f"{command} の余った語 {' '.join(args[used:])} を捨てた（以前も読まなかった）")
                return ["# 変換で捨てた余りの語（以前も読まなかった）: " + " ".join(args[used:])]
            return []

        if command == "layer":
            if not args:
                raise SystemExit(f"{path}:{line_no}: layer に画像の名前が無い")
            nums = leading_numbers(args[1:5])
            layer = {"image": args[0], "comments": take_pending(), "after": []}
            if nums:
                if len(nums) < 4:
                    raise SystemExit(f"{path}:{line_no}: layer の切り出しの数が 4 個そろっていない")
                layer["src_rect"] = [int(float(n)) for n in nums]
            layer["after"] += extra(1 + len(nums))
            layers.append(layer)
        elif command == "layer-rotate":
            # 以前の読み方は知らない行として読まなかった（名前違い）。写さずにコメントで残す（上の決まり）
            note(line_no, f"layer-rotate {' '.join(args)} を写さずコメントにした（以前の読み方は layer-rotation しか読まず、回っていなかった。その見た目を保つ）")
            comment = f"# 回転は付けない : 以前の layer-rotate {' '.join(args)} は名前違いで 2013-08 から回っていなかった。その見た目を保つ（2026-11-11 利用者の決定）"
            if layers:
                layers[-1]["comments"] += take_pending()
                layers[-1]["after"].append(comment)
            else:
                pending.append(comment)  # layer の前なら、次に置く所のコメントにする（以前も落ちずに無視していた）
        elif command in LAYER_COMMANDS:
            if not layers:
                raise SystemExit(f"{path}:{line_no}: layer の前の {command} 行（以前の読み方では落ちる）")
            key = LAYER_COMMANDS[command]
            width = WIDTH[key]
            nums = leading_numbers(args[: width * 3])
            if command == "layer-color":
                note(line_no, "layer-color : 以前の読み方は目標の透明度を今の透明度に入れていた（不具合）。書いた意図どおり from・to・speed に写した")
            if len(nums) % width:
                raise SystemExit(f"{path}:{line_no}: {command} の数の個数が {width} の倍数でない")
            parts = {}
            for i, name in enumerate(("from", "to", "speed")):
                chunk = nums[i * width:(i + 1) * width]
                if chunk:
                    parts[name] = [Num(n, []) for n in chunk] if width > 1 else Num(chunk[0], [])
            layer = layers[-1]
            if key in layer:
                note(line_no, f"{command} が同じ layer に 2 回ある。後の行で上書きした（以前と同じ）")
            layer[key] = parts
            layer["comments"] += take_pending()
            layer["after"] += extra(len(nums))
        elif command == "bgm":
            if not args:
                raise SystemExit(f"{path}:{line_no}: bgm に名前が無い")
            bgm = {"file": args[0]}
            nums = leading_numbers(args[1:2])
            if nums:
                bgm["volume"] = Num(nums[0], [])
            head["bgm"] = bgm
            head_comments.extend(take_pending() + extra(1 + len(nums)))
        elif command in SINGLE_COMMANDS:
            if not args:
                raise SystemExit(f"{path}:{line_no}: {command} に名前が無い")
            head[SINGLE_COMMANDS[command]] = args[0]
            head_comments.extend(take_pending() + extra(1))
        elif command in ("text-color", "text-border-color"):
            try:
                value = int(args[0], 16)
            except (IndexError, ValueError):
                raise SystemExit(f"{path}:{line_no}: {command} の色が 16 進で読めない")
            text_table["color" if command == "text-color" else "border_color"] = value
            text_comments.extend(take_pending() + extra(1))
        elif command == "text":
            text_comments.extend(take_pending() + extra(0))
            # 以前の読み方 : text 行の後の行を 1 行ずつ読み、行ごとに改行を足す（ファイルが改行で終わると、最後の空の行の分も足す）
            rest = lines[line_no:]
            body = "".join(seg + "\n" for seg in rest) if line_no < len(lines) else ""
            break
        else:
            note(line_no, f"知らない行 {line.strip()} を捨てた（以前も読まなかった）")
            pending.append("# 変換で捨てた行（以前も読まなかった）: " + line.strip())

    tail_comments = take_pending()
    if body is not None:
        text_table["body"] = body

    toml = render_file(path, head, head_comments, layers, text_table, text_comments, tail_comments)
    content = {"head": head, "layers": layers, "text": text_table}
    return toml, report, content, encoding


def render_inline(table):
    return "{ " + ", ".join(f"{k} = {render(v)}" for k, v in table.items()) + " }"


def render_body(body):
    """本文を複数行の文字列にする（\\ と " と制御文字だけ逃がす）"""
    out = []
    for ch in body:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\n")
        elif ch == "\t":
            out.append("\t")
        elif ord(ch) < 0x20 or ord(ch) == 0x7F:
            out.append(f"\\u{ord(ch):04X}")
        else:
            out.append(ch)
    return '"""\n' + "".join(out) + '"""'


def render_file(path, head, head_comments, layers, text_table, text_comments, tail_comments):
    kind = "前" if path.suffix == ".intro" else "後"
    stem = path.stem
    out = [f"# {stem} のステージの{kind}の文と背景（tools/stage_convert/convert_story_text.py で {path.name} から変換した）",
           "format = 1"]
    out += head_comments
    for key in ("sound", "bgm", "next_scene", "next_stage"):
        if key in head:
            v = head[key]
            out.append(f"{key} = {render_inline(v) if isinstance(v, dict) else render(v)}")

    for layer in layers:
        out.append("")
        out += layer["comments"]
        out.append("[[layer]]")
        out.append(f"image = {render(layer['image'])}")
        if "src_rect" in layer:
            out.append(f"src_rect = {render(layer['src_rect'])}")
        for key in ("translation", "rotation", "scale", "color"):
            if key in layer:
                out.append(f"{key} = {render_inline(layer[key])}")
        out += layer["after"]

    if text_table or text_comments:
        out.append("")
        out += text_comments
        out.append("[text]")
        for key in ("color", "border_color"):
            if key in text_table:
                out.append(f"{key} = 0x{text_table[key]:08X}")
        if "body" in text_table:
            out.append(f"body = {render_body(text_table['body'])}")

    if tail_comments:
        out.append("")
        out += tail_comments
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------- 確かめ

def as_plain(x):
    """Num を float に、整数も float にして比べられる形にする"""
    if isinstance(x, Num):
        return float(x.value)
    if isinstance(x, bool) or isinstance(x, str) or x is None:
        return x
    if isinstance(x, (int, float)):
        return float(x)
    if isinstance(x, list):
        return [as_plain(v) for v in x]
    if isinstance(x, dict):
        return {k: as_plain(v) for k, v in x.items() if k not in ("comments", "after")}
    return x


def check(path, toml, content):
    doc = tomllib.loads(toml)
    got = {
        "head": {k: doc[k] for k in ("sound", "bgm", "next_scene", "next_stage") if k in doc},
        "layers": doc.get("layer", []),
        "text": doc.get("text", {}),
    }
    if doc.get("format") != 1:
        raise SystemExit(f"{path}: format が 1 でない")
    a = json.dumps(as_plain(content), ensure_ascii=False, sort_keys=True)
    b = json.dumps(as_plain(got), ensure_ascii=False, sort_keys=True)
    if a != b:
        raise SystemExit(f"{path}: 読み直した TOML が変換した中身と合わない\n  変換 : {a}\n  読直 : {b}")
    return len(got["layers"]), len(got["text"].get("body", ""))


def main(argv):
    sys.stdout.reconfigure(encoding="utf-8")
    only_check = "--check" in argv
    targets = [a for a in argv if a != "--check"]
    if not targets:
        print(__doc__)
        return 1

    files = []
    for t in targets:
        p = pathlib.Path(t)
        files.extend(sorted(list(p.glob("*.intro")) + list(p.glob("*.outro"))) if p.is_dir() else [p])

    all_report = []
    for f in files:
        toml, report, content, encoding = convert(f)
        layers, body = check(f, toml, content)
        out = f.with_name(f.name + ".toml")
        if not only_check:
            out.write_bytes(toml.encode("utf-8"))
        print(f"{f.name}（{encoding}）→ {out.name} : 重なり {layers}・本文 {body} 文字・報告 {len(report)}" + ("（確かめだけ）" if only_check else ""))
        all_report.extend(report)

    if all_report:
        print()
        print("報告（捨てた物・直した物）:")
        for r in all_report:
            print("  " + r)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

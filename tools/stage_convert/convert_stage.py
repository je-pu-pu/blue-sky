"""ステージの古い形式（.stage）を TOML（.stage.toml）に変える

使い方:
    python tools/stage_convert/convert_stage.py <.stage のファイル か フォルダ>...
    （フォルダなら中の *.stage を全部。書き出すのは同じ場所の <名前>.stage.toml）

    --check   書き出さずに、変換と読み直しの確かめだけをする

決まり（doc/ステージ形式の設計.md の 2 章・3.1）:
    - 標準ライブラリだけで動く（Python 3.11 以上。tomllib で読み直して確かめる）
    - コメントは場所を保って写す。CP932 のコメントを UTF-8 に直す
    - 数は書いてある文字列のまま写す（TOML の数として読めない書き方だけ直して報告する）
    - exec 行は 1 対 1 で [[event]] when = "start" の then に写す
    - event 行は直前の物体の on_turn_on（など）に写す（今の決まりどおり、位置で決まる物体に付ける）
    - 捨てた物（どこも読まない行・余った数・name の 2 語目・見つからない名前を指す動き）は、
      TOML にコメントで残し、報告に出す

ゲームの読み方（source/blue_sky/stage の LegacyStageParser と TomlStageParser）と同じ結果になるように変える。
「古い読み方の結果 == TOML の読み方の結果」は C++ のテスト（StageFormatCompareTest）で確かめる。
"""

import json
import pathlib
import re
import sys
import tomllib

# ---------------------------------------------------------------- 数・値

TOML_INT = re.compile(r"^[+-]?(0|[1-9][0-9]*)$")
TOML_FLOAT = re.compile(r"^[+-]?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?$")


class Num:
    """書いてある文字列のままの数（TOML として読めない書き方は直す）"""

    def __init__(self, text, notes):
        value = float(text)  # 読めなければ ValueError（呼ぶ側で扱う）
        self.value = value
        if TOML_INT.match(text) or TOML_FLOAT.match(text):
            self.text = text
        else:
            self.text = repr(value)
            notes.append(f"数 {text} は TOML で読めないので {self.text} と書いた")

    def __repr__(self):
        return self.text


def is_number(text):
    try:
        float(text)
        return True
    except ValueError:
        return False


def render(value):
    """TOML の値の書き方"""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, Num):
        return value.text
    if isinstance(value, int):
        return str(value)
    if isinstance(value, str):
        return json.dumps(value, ensure_ascii=False)
    if isinstance(value, list):
        return "[" + ", ".join(render(v) for v in value) + "]"
    raise TypeError(value)


def render_inline(table):
    return "{ " + ", ".join(f"{k} = {render(v)}" for k, v in table.items()) + " }"


# ---------------------------------------------------------------- 引数を読む（C++ の stringstream と同じ決まり）


class Tokens:
    """空白で区切った引数を順に読む。読めなかった所から後は読まない（以前の ss >> x と同じ）"""

    def __init__(self, words, notes):
        self.words = list(words)
        self.pos = 0
        self.failed = False
        self.notes = notes

    def word(self):
        if self.failed or self.pos >= len(self.words):
            self.failed = True
            return None
        w = self.words[self.pos]
        self.pos += 1
        return w

    def number(self):
        if self.failed or self.pos >= len(self.words):
            self.failed = True
            return None
        w = self.words[self.pos]
        if not is_number(w):
            self.failed = True
            return None
        self.pos += 1
        return Num(w, self.notes)

    def numbers(self, n):
        """n 個まで読む（読めた分だけ返す）"""
        out = []
        for _ in range(n):
            v = self.number()
            if v is None:
                break
            out.append(v)
        return out

    def boolean(self):
        """0 / 1 だけ（以前の ss >> bool と同じ）"""
        if self.failed or self.pos >= len(self.words):
            self.failed = True
            return None
        w = self.words[self.pos]
        if w not in ("0", "1"):
            self.failed = True
            return None
        self.pos += 1
        return w == "1"

    def rest(self):
        return self.words[self.pos:]


def pad(values, n):
    """足りない数を 0 で補う（以前は初めの値 0 のままだった所）"""
    return values + [Num("0", [])] * (n - len(values))


# ---------------------------------------------------------------- 動き

class Converted:
    def __init__(self, table=None, dropped=None):
        self.table = table          # dict（do が先頭）か None（捨てた）
        self.dropped = dropped      # 捨てた理由


def convert_action(name, words, notes):
    """古いコマンド 1 つを { do = ... } にする。notes に報告を足す"""
    t = Tokens(words, notes)

    def done(table):
        rest = t.rest() if not t.failed else t.words[t.pos:]
        if rest:
            notes.append(f"{name} の余った引数を捨てた : {' '.join(rest)}")
        return Converted(table)

    if name == "timer":
        interval = t.number()
        count = t.number()
        inner_name = t.word()
        if interval is None or count is None or inner_name is None:
            return Converted(None, "timer の間隔・回数・コマンドを読めない")
        inner = convert_action(inner_name, t.rest(), notes)
        if inner.table is None:
            return inner
        if inner.table.get("delay") is not None or inner.table.get("every") is not None:
            return Converted(None, "timer の中の timer は写せない")
        table = dict(inner.table)
        if count.text == "1":
            table["delay"] = interval
        else:
            table["every"] = interval
            table["count"] = int(count.value) if float(count.value).is_integer() else count
        return Converted(table)

    simple_set = {
        "set_line_type": ("line_type", "int"),
        "set_paper_type": ("paper_type", "int"),
        "set_drawing_accent_scale": ("drawing_accent_scale", "num"),
        "set_shading_enabled": ("shading_enabled", "bool"),
        "set_balloon_sound_type": ("balloon_sound_type", "int"),
    }
    if name in simple_set:
        key, kind = simple_set[name]
        if kind == "bool":
            v = t.boolean()
        else:
            v = t.number()
            if v is not None and kind == "int" and not TOML_INT.match(v.text):
                return Converted(None, f"{name} の値 {v.text} は整数ではない")
        if v is None:
            return Converted(None, f"{name} の値を読めない")
        return done({"do": "set", key: v})

    colors = {"ambient": "ambient_color", "shadow": "shadow_color", "shadow_paper": "shadow_paper_color"}
    for prefix, key in colors.items():
        if name == f"set_{prefix}_color":
            c = t.numbers(4)
            if not c:
                return Converted(None, f"{name} の色を読めない")
            if len(c) < 4:
                notes.append(f"{name} の色が {len(c)} 個（残りは前の値のまま。以前と同じ）")
            return done({"do": "set", key: c})
        if name == f"set_{prefix}_color_target":
            c = t.numbers(4)
            if not c:
                return Converted(None, f"{name} の色を読めない")
            table = {"do": "fade", key: c}
            if len(c) == 4:
                s = t.number()
                if s is not None:
                    table["speed"] = s
            if len(c) < 4:
                notes.append(f"{name} の色が {len(c)} 個（残りは前の値のまま。以前と同じ）")
            return done(table)

    if name in ("set_light_position", "set_light_position_target"):
        p = t.numbers(3)
        if not p:
            return Converted(None, f"{name} の位置を読めない")
        return done({"do": "set" if name == "set_light_position" else "fade", "light_position": pad(p, 3)})

    if name == "change_sky_box":
        v = t.word()
        if v is None:
            return Converted(None, "change_sky_box の名前が無い")
        ext = t.word()  # 2 語目（拡張子）は以前も使っていなかった
        if ext is not None:
            notes.append(f"change_sky_box の 2 語目を捨てた : {ext}（以前も使っていなかった）")
        t.failed = False
        return done({"do": "set", "sky_box": v})

    if name == "change_bgm":
        if not words:
            return Converted(None, "change_bgm の名前が無い")
        return Converted({"do": "set", "bgm": " ".join(words)})

    if name == "change_bpm":
        bpm = t.number()
        if bpm is None:
            return Converted(None, "change_bpm の値を読めない")
        table = {"do": "set", "bpm": bpm}
        beat = t.number()
        if beat is not None:
            table["beat"] = beat
        return done(table)

    if name == "load_sound":
        n = t.word()
        if n is None:
            return Converted(None, "load_sound の名前が無い")
        table = {"do": "load_sound", "name": n}
        f = t.word()
        if f is not None:
            table["file"] = f
            r = t.boolean()
            if r is not None:
                table["replace"] = r
        return done(table)

    if name == "play_sound":
        n = t.word()
        if n is None:
            return Converted(None, "play_sound の名前が無い")
        table = {"do": "play_sound", "sound": n}
        f = t.boolean()
        if f is not None:
            table["force"] = f
            lp = t.boolean()
            if lp is not None:
                table["loop"] = lp
        return done(table)

    if name == "stop_sound":
        if not words:
            return Converted(None, "stop_sound の名前が無い")
        return Converted({"do": "stop_sound", "sound": " ".join(words)})

    if name == "stop_all_sound":
        return done({"do": "stop_all_sounds"})

    if name == "end_by_last_switch":
        return done({"do": "end_stage"})

    if name == "player.start_flickering":
        return done({"do": "start_flickering"})

    if name == "game_object.create":
        model = t.word()
        if model is None:
            return Converted(None, "game_object.create のモデルが無い")
        table = {"do": "spawn", "model": model}
        p = t.numbers(3)
        if p:
            table["position"] = pad(p, 3)
            r = t.numbers(3)
            if r:
                table["rotation"] = pad(r, 3)
        return done(table)

    # 名前で引いた物体への動き
    if not name.startswith("game_object."):
        return Converted(None, f"知らないコマンド {name}")

    target = t.word()
    if target is None:
        return Converted(None, f"{name} の物体の名前が無い")
    op = name[len("game_object."):]

    if op == "set_mass":
        v = t.number()
        return done({"do": "set_body", "target": target, "mass": v if v is not None else Num("0", [])})
    if op in ("set_gravity", "set_angular_factor"):
        return done({"do": "set_body", "target": target, op[len("set_"):]: pad(t.numbers(3), 3)})
    if op == "set_kinematic":
        v = t.boolean()
        return done({"do": "set_body", "target": target, "kinematic": bool(v)})
    if op == "set_no_contact_response":
        v = t.boolean()
        return done({"do": "set_body", "target": target, "contact_response": not bool(v)})
    if op == "set_flicker_scale":
        table = {"do": "set_flicker", "target": target}
        v = t.number()
        if v is not None:
            table["scale"] = v
        return done(table)
    if op == "set_target_location":
        p = t.numbers(3)
        table = {"do": "move_to", "target": target, "position": pad(p, 3)}
        if len(p) == 3:
            s = t.number()
            if s is not None:
                table["speed"] = s
        return done(table)
    if op == "set_target_direction":
        table = {"do": "turn_to", "target": target}
        r = t.number()
        table["direction"] = r if r is not None else Num("0", [])
        if r is not None:
            s = t.number()
            if s is not None:
                table["speed"] = s
        return done(table)
    if op == "set_target_direction_object":
        face = t.word()
        if face is None:
            return Converted(None, f"{name} の向く先が無い")
        table = {"do": "turn_to", "target": target, "face": face}
        s = t.number()
        if s is not None:
            table["speed"] = s
        return done(table)
    if op == "play_animation":
        a = t.word()
        if a is None:
            return Converted(None, f"{name} のアニメーションの名前が無い")
        table = {"do": "play_animation", "target": target, "name": a}
        f = t.boolean()
        if f is not None:
            table["force"] = f
            lp = t.boolean()
            if lp is not None:
                table["loop"] = lp
        return done(table)
    if op == "set_animation_speed":
        table = {"do": "set_animation_speed", "target": target}
        v = t.number()
        if v is not None:
            table["speed"] = v
        return done(table)
    if op == "action":
        words2 = t.rest()
        t.pos = len(t.words)
        text = " ".join(words2)
        states = {"turn-on": "on", "turn-off": "off", "break": "broken"}
        if text in states:
            return Converted({"do": "switch", "target": target, "state": states[text]})
        if text in ("float", "rotation", "patrol"):
            return Converted({"do": "set_mode", "target": target, "mode": text})
        if text in ("break_animation 1", "break_animation 0"):
            return Converted({"do": "break_animation", "target": target, "broken": text.endswith("1")})
        if words2 and words2[0] == "set_visible":
            t2 = Tokens(words2[1:], notes)
            table = {"do": "set_visible", "target": target}
            m = t2.boolean()
            if m is not None:
                table["model"] = m
                ln = t2.boolean()
                if ln is not None:
                    table["line"] = ln
            if t2.rest():
                notes.append(f"set_visible の余った引数を捨てた : {' '.join(t2.rest())}")
            return Converted(table)
        return Converted(None, f"知らない物体の動き action {text}")

    return Converted(None, f"知らないコマンド {name}")


# ---------------------------------------------------------------- ステージ

TYPED = {"girl", "robot", "balloon", "medal", "ladder", "rocket", "umbrella", "stone", "switch"}


class Obj:
    def __init__(self, kind, line_no):
        self.kind = kind
        self.line_no = line_no
        self.model = None
        self.position = []
        self.rotation = None
        self.direction = None
        self.size = None
        self.speed = None
        self.names = []
        self.events = []          # (イベント名, Converted, 前のコメント, 行番号)
        self.after = []           # 物体の後に出すコメント
        self.comments_before = []


class Item:
    """表に並べるもの : 物体・始めの動きのかたまり・コメント"""

    def __init__(self, kind, payload):
        self.kind = kind
        self.payload = payload


def convert(path):
    """1 つのファイルを変換する。(TOML の文字列, 報告の行, 確かめ用の中身) を返す"""
    raw = path.read_bytes()
    text = raw.decode("cp932")
    lines = text.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    if lines and lines[-1] == "":
        lines.pop()

    report = []
    header = {"stage": {}, "player": {}, "goal": {}}
    header_comments = {"stage": [], "player": [], "goal": []}
    items = []
    pending = []                 # まだ置き場の決まらないコメント・空行（"" は空行）
    last_obj = None
    start_block = None           # 今の [[event]] when = "start" のかたまり（動きの list）

    def note(line_no, message):
        report.append(f"{path.name}:{line_no}: {message}")

    def flush_pending_to_flow():
        nonlocal pending
        for c in pending:
            items.append(Item("comment", c))
        pending = []

    for line_no, line in enumerate(lines, 1):
        words = line.split()
        if not words:
            pending.append("")
            continue
        command = words[0]
        notes = []

        if command.startswith("#"):
            body = line.strip()[1:].strip() if command == "#" else line.strip()[1:]
            pending.append("# " + body if body else "#")
            continue

        if command in ("event", "exec"):
            if command == "event":
                if last_obj is None:
                    raise SystemExit(f"{path}:{line_no}: 物体の無い所の event 行")
                event_name = words[1] if len(words) > 1 else ""
                action_words = words[2:]
            else:
                event_name = None
                action_words = words[1:]
            if not action_words:
                raise SystemExit(f"{path}:{line_no}: 動きの名前が無い")
            conv = convert_action(action_words[0], action_words[1:], notes)
            for n in notes:
                note(line_no, n)
            if conv.table is None:
                note(line_no, f"捨てた : {line.strip()}（{conv.dropped}）")
            if command == "event":
                last_obj.events.append((event_name, conv, pending, line_no, line.strip()))
                pending = []
            else:
                if start_block is None:
                    flush_pending_to_flow()
                    start_block = []
                    items.append(Item("start", start_block))
                    start_block.append((conv, [], line_no, line.strip()))
                else:
                    start_block.append((conv, pending, line_no, line.strip()))
                    pending = []
            continue

        # exec のかたまりはここで終わる
        start_block = None

        if command == "name":
            if last_obj is None:
                raise SystemExit(f"{path}:{line_no}: 物体の無い所の name 行")
            last_obj.names.append(words[1] if len(words) > 1 else "")
            if len(words) > 2:
                note(line_no, f"name の 2 語目から後を捨てた : {' '.join(words[2:])}（以前も 1 語目だけを名前にしていた）")
            if pending:
                last_obj.after.extend(pending)
                pending = []
            continue

        if command in ("collision", "ground", "bgm", "bgm.loop", "player", "goal"):
            t = Tokens(words[1:], notes)
            if command in ("collision", "ground"):
                v = t.word()
                header["stage"][command] = v if v is not None else ""
                section = "stage"
            elif command == "bgm":
                v = t.word()
                header["stage"]["bgm"] = v if v is not None else ""
                loop = t.boolean()
                if loop is not None:
                    note(line_no, "bgm の 2 語目（繰り返すか）は以前も読んでいなかったので捨てた")
                    bpm = t.number()
                    if bpm is not None:
                        header["stage"]["bgm_bpm"] = bpm
                section = "stage"
            elif command == "bgm.loop":
                v = t.boolean()
                if v is None:
                    note(line_no, f"bgm.loop の値を読めないので捨てた : {line.strip()}")
                else:
                    header["stage"]["bgm_loop"] = v
                section = "stage"
            elif command == "player":
                p = t.numbers(3)
                if len(p) < 3:
                    note(line_no, "player の位置の足りない数を 0 で補った")
                header["player"]["position"] = pad(p, 3)
                if t.pos < len(t.words):
                    r = t.number()
                    header["player"]["camera_direction"] = r if r is not None else Num("0", [])
                section = "player"
            else:
                p = t.numbers(3)
                if len(p) < 3:
                    note(line_no, "goal の位置の足りない数を 0 で補った")
                header["goal"]["position"] = pad(p, 3)
                section = "goal"
            rest = t.words[t.pos:]
            if command not in ("bgm", "bgm.loop", "player") and rest:
                note(line_no, f"{command} の余った引数を捨てた : {' '.join(rest)}")
            for n in notes:
                note(line_no, n)
            header_comments[section].extend(c for c in pending if c)
            pending = []
            continue

        # 物体
        obj = None
        t = Tokens(words[1:], notes)
        if command in ("object", "static-object", "dynamic-object"):
            obj = Obj("static", line_no)
            obj.model = t.word() or ""
            p = t.numbers(3)
            obj.position = pad(p, 3)
            if len(p) == 3:
                r = t.numbers(3)
                if r:
                    obj.rotation = pad(r, 3)
            if command == "dynamic-object":
                note(line_no, "dynamic-object は中身が object と同じなので type = \"static\" にした")
        elif command == "translation-object":
            obj = Obj("translation", line_no)
            p = t.numbers(3)
            obj.position = pad(p, 3)
            if len(p) == 3:
                s = t.numbers(3)
                if s:
                    obj.size = pad(s, 3)
                    if len(s) == 3:
                        obj.speed = t.number()
        elif command in TYPED or command == "area-switch":
            obj = Obj(command, line_no)
            p = t.numbers(3)
            if len(p) < 3:
                note(line_no, f"{command} の位置の足りない数を 0 で補った")
            obj.position = pad(p, 3)
            if len(p) == 3 and command == "area-switch":
                s = t.numbers(3)
                obj.size = pad(s, 3)
                if len(s) < 3:
                    note(line_no, "area-switch の大きさの足りない数を 0 で補った")
                if len(s) == 3:
                    obj.direction = t.number()
            elif len(p) == 3:
                obj.direction = t.number()
            elif command == "area-switch":
                obj.size = pad([], 3)
        if obj is not None:
            t.failed = False
            rest = t.words[t.pos:]
            if rest:
                note(line_no, f"{command} の余った引数を捨てた : {' '.join(rest)}")
            for n in notes:
                note(line_no, n)
            obj.comments_before = pending
            pending = []
            items.append(Item("object", obj))
            last_obj = obj
            continue

        # どこも読まない行（以前はステージの設定に入り、bgm.loop のほかは読まれなかった）
        flush_pending_to_flow()
        items.append(Item("comment", f"# 変換で捨てた行（どこも読まない）: {line.strip()}"))
        note(line_no, f"捨てた : {line.strip()}（どこも読まない行）")

    flush_pending_to_flow()

    # 名前の参照を確かめる（読み終わったときの名前で。遊んでいる最中に名前が増えることは無い）
    names = {"player", "goal"}
    for item in items:
        if item.kind == "object":
            names.update(item.payload.names)
            if item.payload.kind == "girl":
                names.add("girl")

    def check_refs(conv, line_no, text):
        if conv.table is None:
            return conv
        for key in ("target", "face"):
            ref = conv.table.get(key)
            if ref is not None and ref not in names:
                note(line_no, f"捨てた : {text}（{ref} という名前の物体は無い。以前も何もしなかった）")
                return Converted(None, f"{ref} という名前の物体は無い")
        return conv

    for item in items:
        if item.kind == "object":
            o = item.payload
            o.events = [(e, check_refs(c, n, s), p, n, s) for (e, c, p, n, s) in o.events]
        elif item.kind == "start":
            item.payload[:] = [(check_refs(c, n, s), p, n, s) for (c, p, n, s) in item.payload]

    toml = render_stage(path, header, header_comments, items)
    return toml, report, (header, items)


def object_lines(o, positions=None):
    out = ["[[object]]", f"type = {render(o.kind)}"]
    if o.names:
        out.append(f"name = {render(o.names[0] if len(o.names) == 1 else o.names)}")
    if o.model is not None:
        out.append(f"model = {render(o.model)}")
    if positions is not None:
        out.append("positions = [")
        out.extend(f"  {render(p)}," for p in positions)
        out.append("]")
    else:
        out.append(f"position = {render(o.position)}")
    if o.rotation is not None:
        out.append(f"rotation = {render(o.rotation)}")
    if o.direction is not None:
        out.append(f"direction = {render(o.direction)}")
    if o.size is not None:
        out.append(f"size = {render(o.size)}")
    if o.speed is not None:
        out.append(f"speed = {render(o.speed)}")

    # イベントごとの配列（書いた順。イベントの名前ごとにまとめる）
    events = {}
    for e in o.events:
        events.setdefault(e[0], []).append(e)
    for event_name, list_ in events.items():
        key = "on_" + event_name.replace("-", "_")
        out.append(f"{key} = [")
        for (_, conv, comments, line_no, text) in list_:
            out.extend(f"  {c}" if c else "" for c in comments)
            if conv.table is None:
                out.append(f"  # 変換で捨てた : {text}（{conv.dropped}）")
            else:
                out.append(f"  {render_inline(conv.table)},")
        out.append("]")
    return out


def is_plain(o):
    """まとめ書きにできる物体（名前もイベントもコメントも無い、種類のある物体）"""
    return o.kind in TYPED and not o.names and not o.events and not o.after and not o.comments_before


def render_stage(path, header, header_comments, items):
    out = [f"# {path.name.split('.')[0]} のステージ（tools/stage_convert/convert_stage.py で {path.name} から変換した）", "format = 1", ""]

    stage_keys = ["collision", "ground", "bgm", "bgm_bpm", "bgm_loop"]
    if header["stage"] or header_comments["stage"]:
        out.append("[stage]")
        out.extend(header_comments["stage"])
        for k in stage_keys:
            if k in header["stage"]:
                out.append(f"{k} = {render(header['stage'][k])}")
        out.append("")
    for section in ("player", "goal"):
        if header[section] or header_comments[section]:
            out.append(f"[{section}]")
            out.extend(header_comments[section])
            for k, v in header[section].items():
                out.append(f"{k} = {render(v)}")
            out.append("")

    i = 0
    while i < len(items):
        item = items[i]
        if item.kind == "comment":
            # 空行は続けて 2 つ以上置かない
            if item.payload == "" and out and out[-1] == "":
                i += 1
                continue
            out.append(item.payload)
            i += 1
            continue
        if item.kind == "start":
            out.append("[[event]]")
            out.append('when = "start"')
            out.append("then = [")
            for (conv, comments, line_no, text) in item.payload:
                out.extend(f"  {c}" if c else "" for c in comments)
                if conv.table is None:
                    out.append(f"  # 変換で捨てた : {text}（{conv.dropped}）")
                else:
                    out.append(f"  {render_inline(conv.table)},")
            out.append("]")
            out.append("")
            i += 1
            continue
        o = item.payload
        out.extend(f"{c}" for c in o.comments_before if c)
        # まとめ書き : 名前もイベントも無い同じ種類・同じ向きが 3 つ以上続く所
        j = i

        def direction_key(x):
            return None if x.direction is None else x.direction.text

        if is_plain(o):
            while j + 1 < len(items) and items[j + 1].kind == "object" and is_plain(items[j + 1].payload) \
                    and items[j + 1].payload.kind == o.kind and direction_key(items[j + 1].payload) == direction_key(o):
                j += 1
        if j - i + 1 >= 3:
            out.extend(object_lines(o, positions=[items[k].payload.position for k in range(i, j + 1)]))
            i = j + 1
        else:
            out.extend(object_lines(o))
            i += 1
        out.extend(c for c in o.after if c)
        out.append("")

    while out and out[-1] == "":
        out.pop()
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------- 読み直して確かめる

def expected_view(header, items):
    """変換した中身を、読み直した TOML と比べられる形にする"""
    objects = []
    start = []

    def table_view(table):
        return {k: (v.value if isinstance(v, Num) else [x.value if isinstance(x, Num) else x for x in v] if isinstance(v, list) else v)
                for k, v in table.items()}

    for item in items:
        if item.kind == "object":
            o = item.payload
            view = {"type": o.kind, "position": [p.value for p in o.position], "names": o.names, "events": [
                (e, table_view(c.table)) for (e, c, _, _, _) in o.events if c.table is not None]}
            objects.append(view)
        elif item.kind == "start":
            start.extend(table_view(c.table) for (c, _, _, _) in item.payload if c.table is not None)
    return objects, start


def reread_view(doc):
    objects = []
    for o in doc.get("object", []):
        positions = o.get("positions", [o.get("position")])
        names = o.get("name", [])
        names = [names] if isinstance(names, str) else names
        events = []
        for k, v in o.items():
            if k.startswith("on_"):
                events.extend((k[3:].replace("_", "-"), a) for a in v)
        for p in positions:
            objects.append({"type": o["type"], "position": [float(x) for x in p], "names": names, "events": events})
    start = []
    for e in doc.get("event", []):
        if e.get("when") == "start":
            start.extend(e.get("then", []))
    return objects, start


def check(path, toml, content):
    doc = tomllib.loads(toml)
    exp_objects, exp_start = expected_view(*content)
    got_objects, got_start = reread_view(doc)

    def numbers_as_float(x):
        # TOML の整数と小数を同じに扱う（180 と 180.0）
        if isinstance(x, bool) or isinstance(x, str) or x is None:
            return x
        if isinstance(x, (int, float)):
            return float(x)
        if isinstance(x, (list, tuple)):
            return [numbers_as_float(v) for v in x]
        if isinstance(x, dict):
            return {k: numbers_as_float(v) for k, v in x.items()}
        return x

    def norm(x):
        return json.dumps(numbers_as_float(x), ensure_ascii=False, sort_keys=True)

    # イベントの順はイベントの名前ごとにまとめて書くので、名前ごとの順で比べる
    def by_event(objs):
        out = []
        for o in objs:
            o = dict(o)
            o["events"] = sorted(o["events"], key=lambda e: e[0])
            out.append(o)
        return out

    ok = norm(by_event(exp_objects)) == norm(by_event(got_objects)) and norm(exp_start) == norm(got_start)
    if not ok:
        raise SystemExit(f"{path}: 読み直した TOML が変換した中身と合わない")
    return len(got_objects), sum(len(o["events"]) for o in got_objects), len(got_start)


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
        files.extend(sorted(p.glob("*.stage")) if p.is_dir() else [p])

    all_report = []
    for f in files:
        toml, report, content = convert(f)
        objects, events, start = check(f, toml, content)
        out = f.with_name(f.name + ".toml")
        if not only_check:
            out.write_bytes(toml.encode("utf-8"))
        print(f"{f.name} → {out.name} : 物体 {objects}・イベント {events}・始めの動き {start}・報告 {len(report)}" + ("（確かめだけ）" if only_check else ""))
        all_report.extend(report)

    if all_report:
        print()
        print("報告（捨てた物・直した物）:")
        for r in all_report:
            print("  " + r)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

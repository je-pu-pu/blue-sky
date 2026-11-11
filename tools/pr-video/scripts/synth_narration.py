"""VOICEVOX ENGINE でナレーションを合成する。

前提: VOICEVOX ENGINE が http://127.0.0.1:50021 で起動していること。
出力: tools/pr-video/public/audio/narration/{name}.wav
     ＋ 各 wav の長さ(秒)を JSON で標準出力（シーン尺の決定に使う）。
"""
import json
import sys
import wave
import urllib.parse
import urllib.request
from pathlib import Path

BASE = "http://127.0.0.1:50021"
SPEAKER = 47         # ナースロボ＿タイプＴ（ノーマル）。他: 楽々=48 / 恐怖=49 / 内緒話=50
SPEED = 1.18         # やや早口でテンポよく

OUT = Path(__file__).resolve().parent.parent / "public" / "audio" / "narration"
OUT.mkdir(parents=True, exist_ok=True)

# シーン名 → 台本（ずんだもん調）
LINES = [
    ("title",   "ブルースカイの、見つからずに進むゲーム。今回できたことを紹介します。"),
    ("systems", "敵は目と耳で気づきます。だんだん警戒して、最後は追いかけてくる。"
                "花火でおとりにしたり、煙で隠れたり、後ろから気絶させて隠せます。"),
    ("nav",     "建物は階段と橋でつながっていて、屋上まで登れます。"
                "行き方はいろいろあります。"),
    ("verbs",   "空を飛べます。光る箱を集めて、三十個でクリアです。"),
    ("footage", "実際の画面です。"),
    ("commits", "今回作ったもの。敵が屋上に登る、橋や手すり、いろいろな道、箱集めです。"),
    ("outro",   "今回はここまで。続きもお楽しみに。"),
]


def post(path: str, data: bytes | None = None):
    req = urllib.request.Request(BASE + path, data=data, method="POST")
    if data is not None:
        req.add_header("Content-Type", "application/json")
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read()


def synth(text: str) -> bytes:
    q = post("/audio_query?" + urllib.parse.urlencode({"text": text, "speaker": SPEAKER}))
    query = json.loads(q)
    query["speedScale"] = SPEED
    query["prePhonemeLength"] = 0.1
    query["postPhonemeLength"] = 0.3
    return post(f"/synthesis?speaker={SPEAKER}", json.dumps(query).encode("utf-8"))


def wav_seconds(path: Path) -> float:
    with wave.open(str(path), "rb") as w:
        return w.getnframes() / float(w.getframerate())


def main():
    durations = {}
    for name, text in LINES:
        wav = synth(text)
        p = OUT / f"{name}.wav"
        p.write_bytes(wav)
        durations[name] = round(wav_seconds(p), 3)
        print(f"  {name}: {durations[name]}s  <- {text[:24]}...", file=sys.stderr)
    print(json.dumps(durations, ensure_ascii=False))


if __name__ == "__main__":
    main()

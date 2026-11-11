import json
import urllib.request

TARGET = "ナースロボ"
d = json.load(urllib.request.urlopen("http://127.0.0.1:50021/speakers", timeout=10))
for sp in d:
    if TARGET in sp["name"]:
        print("見つけました:", sp["name"])
        for st in sp["styles"]:
            # style 名は文字化けし得るが id（数値）が要る
            print(f"  style_id={st['id']}  name={st['name']}")

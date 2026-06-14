# スタイル参照画像 (neural スタイライザ用)

`run_eval.py --stylizer neural --style styles/<file>` で参照する絵。
VGG Gatys 法は、この1枚の「色・筆致・質感」を各フレームへ転写する。

## 権利について

**全ファイルがパブリックドメイン (Public domain)**。作者は全員没後70年以上、作品も
すべて1929年以前の制作で、米国・生没+70年いずれの基準でも PD。出典は Wikimedia Commons
（2次元 PD 作品の忠実な複製は新たな著作権を生じない＝Commons の PD 方針）。
そのままリポジトリに同梱・改変・再配布して問題ない。

## ファイル一覧

| ファイル | 種別 | 作者 / 作品 | 年 | 採用 |
|---|---|---|---|---|
| `brush_starry_night.jpg` | 強い筆致 | Vincent van Gogh『星月夜 (Starry Night)』 | 1889 | 比較用 |
| `flat_great_wave.jpg` | フラット/浮世絵 | 葛飾北斎『神奈川沖浪裏 (Great Wave)』 | c.1831 | 比較用 |
| `watercolor.jpg` | 水彩 | Winslow Homer『The Blue Boat』 | 1892 | 比較用 |
| `pencil_carmiencke.jpg` | **鉛筆(黒鉛)** | Johann Hermann Carmiencke『View of the Shawangunk Mountains』 | 1865 | **採用(鉛筆)** |
| `pen_vangogh.jpg` | **ペン画(葦ペン)** | Vincent van Gogh『Montmajour の岩山と木々』 | 1888-07 | **採用(ペン)** |
| `pencil_haseltine.jpg` | 鉛筆(黒鉛) | William Stanley Haseltine『West Island』 | 1863 | 候補(淡く線が弱い) |
| `pencil_hill.jpg` | 鉛筆(黒鉛) | John Henry Hill『Niagara Falls』 | c.1873+ | 候補 |
| `pen_vitruvian.jpg` | ペン画 | Leonardo da Vinci『ウィトルウィウス的人体図』 | c.1490 | 候補(人体図のため題材が強い) |

### 採用の経緯
- **鉛筆**は `pencil_carmiencke`（山肌のトーン＋細かい黒鉛ハッチングがあり、風景題材で本シーンと相性が良い）。
  `haseltine` は淡すぎ、`vitruvian` は人体図でモチーフが強く出るため見送り。
- **ペン画**は `pen_vangogh`（葦ペンの濃密なインクハッチングが線画として強く出る）。
- 古典アルゴリズムの `pencil` スタイライザ（`stylizers/pencil_sketch.py`、画像参照なし）も別途あり。

## 出典 (Wikimedia Commons)

各ファイルは `https://commons.wikimedia.org/wiki/File:<元タイトル>` で参照可能。
取得は `Special:FilePath/<タイトル>?width=1000`（リダイレクトで実体に解決）。
元タイトルは長い英語名のものがあるため、再取得が必要な場合は Commons で作者名＋作品名を検索すること。

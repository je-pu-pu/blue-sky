# Neural NPR オフライン検証ハーネス（段階0a / 評価優先・学習なし）

ゲームがダンプした連番フレームを使い、**「シンプルなレンダ結果を neural NPR 調へ変換したとき、時間的にちらつかずに成立しうるか」** を、エンジンへ実機統合する前に最小コストで確かめるためのツール。

- 範囲: **評価優先（学習なし）**。学習ループは持たない。
- やること: 各フレームをスタイル変換し、**素の結果のちらつき** と **warp+時間ブレンドで安定化した結果のちらつき** を数値・動画で比較する。
- 既定スタイライザ: **ニューラルスタイル転写**（差し替え可能）。`classic`（古典NPRフィルタ）はスモークテスト用。

関連: Asana エピックB「Neural NPR レンダリング」。次段（0b）で法線/モーションベクトルの G-buffer を足す。

## 1. 入力データを作る（エンジン側）

1. `blue-sky-exe.exe` を起動 → debug シーン
2. imgui パネル **「Frame Dump (Neural NPR)」** で Frames を設定 → **Start Dump**
3. **ダンプ中にカメラを動かす**（動きが無いと時間安定の検証にならない）
4. 実行ディレクトリ直下の `dump/color_%04d.png` に出力される（通常 `source/dump/`）

## 2. セットアップ

```bash
python -m venv .venv && . .venv/Scripts/activate   # 任意
pip install -r tools/npr_offline/requirements.txt
```

GPU を使う場合は `torch` / `torchvision` を https://pytorch.org の案内に従い CUDA 版で入れること。
neural は CPU だと非常に遅いので、その場合は `--max-frames` を小さく、または `classic` を使う。

## 3. 実行

スモークテスト（数秒・配線確認）:

```bash
python tools/npr_offline/run_eval.py --frames source/dump --stylizer classic --out tools/npr_offline/out
```

本番（ニューラルスタイル転写）:

```bash
python tools/npr_offline/run_eval.py --frames source/dump --stylizer neural --style path/to/style.jpg \
    --out tools/npr_offline/out --max-size 384 --steps 150
```

主な引数:

| 引数 | 意味 |
|---|---|
| `--frames` | ダンプ連番のディレクトリ |
| `--stylizer` | `neural`（既定の検証）/ `classic`（スモーク） |
| `--style` | neural 用スタイル画像 |
| `--alpha` | 時間ブレンド係数（0..1, 既定 0.6） |
| `--max-frames` | 先頭 N 枚だけ処理 |
| `--steps` / `--max-size` | neural の反復数 / 処理解像度上限 |

## 4. 出力と合否の見方

`--out` に出力:

- `comparison.mp4` … `input | naive | stable` を横並びにした動画。**目視で stable 側のちらつきが減っていれば良い兆候**。
- `sample_*.png` … 同じ横並びの静止画。
- `metrics.json` … 時間誤差。`reduction_percent` が **正に大きいほど安定化が効いている**。

判断:

- **安定化でちらつきが目に見えて減り、`reduction_percent` が有意に正** → neural NPR はこのコンテンツで成立しうる。実機統合（段階0b 以降）へ。
- **素の段階でちらつきが激しく、安定化でも残る** → 実機統合の前にここで方針見直し（条件付け強化＝法線/モーション追加、または手法変更）。

## 設計メモ

- スタイライザは `stylizers/base.py` の `Stylizer` インターフェースで差し替え可能。
- 将来案: **作例ベースのペア学習（diffusion 教師や手描き作例）** は別タスク（Asana エピックB）。
- フロー推定は OpenCV Farneback、遮蔽は前後フロー整合で判定（`temporal.py`）。

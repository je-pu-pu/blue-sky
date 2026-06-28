# Lessons - 過去の失敗と学び

## 記録ルール

- バグを解決したら、ここにパターンと対策を追記する
- 設計上の判断ミスや整合性の注意点も記録する
- 同じ失敗を繰り返さないための知見をまとめる

## MSDF フォントレンダリング

- テキスト色が二乗される問題: シェーダーで色を乗算する際、既にリニア空間の色をさらにガンマ補正してしまっていた
- アウトラインの太さがグリフサイズに依存していた問題: ピクセルレンジをスクリーンピクセル単位に変換して均一化
- fbx-converter.vcxproj のビルドエラーは既存の問題 (C++17フラグ不足、FBX SDK パス不足) で、本体には影響しない
- `common::convert_to_wstring` / `convert_to_string` が UTF-8 ベースに統一済み。新規コードでは直接 `MultiByteToWideChar` を呼ばず、この共通関数を使うこと

## レイヤー移動 (名前空間変更)

- コードを `core::ecs` → `blue_sky::ecs` に移動する際、名前空間が変わると基底クラス・型の非修飾参照が解決できなくなる。移動先の名前空間に `using core::ecs::Component;` 等の using 宣言を追加する必要がある
- `.cpp` ファイルも名前空間を変更した場合、`.h` で using した型はスコープ内で使えるが、直接参照している別の名前空間の型 (例: `math::Vector` → `core::math::Vector`) は修飾が必要

## 親子名前空間での型名の曖昧さ

- `core::graphics::direct_3d_11::Texture` と `core::graphics::Texture` のように、子名前空間の型が親名前空間の同名型を隠す場合、基底クラス経由で親の型が優先されることがある
- 具体例: `core::graphics::direct_3d_11::RenderTargetTexture` が `core::graphics::RenderTargetTexture` を継承しているため、クラス内で非修飾の `Texture` が `core::graphics::Texture` (基底クラスの名前空間) に解決された
- 対策: ヘッダでは完全修飾名 (`core::graphics::direct_3d_11::Texture`) を使い、`.cpp` では `using D3D11Texture = core::graphics::direct_3d_11::Texture;` のようなエイリアスで簡潔に書く

## Python 検証ツールの環境構築 (tools/npr_offline)

- システムの `python` は 3.14 で新しすぎ、torch/opencv 等の wheel が無くインストール失敗する。ツールごとに **uv で隔離 venv** を作る (`tools/<name>/.venv`, Python 3.12)。グローバル pip は汚さない
- `uv` は PATH に乗らないことがある。**`python -m uv`** で叩けば動く
- GPU (RTX 5070 Ti / Blackwell, sm_120) では torch は **cu128 wheel** が必要 (`--index-url https://download.pytorch.org/whl/cu128`)。**CUDA Toolkit の別途インストールは不要** — cu128 wheel が CUDA ランタイムを同梱するので、新しめの NVIDIA ドライバさえあれば `torch.cuda.is_available()` が True になる
- 1080p フレームを 120 枚 float32 で全保持すると RAM が枯渇する (1枚23.7MiB × frames/naive/stable/comparison)。**読み込み時に最長辺をダウンスケール** (`--proc-size`, 既定720) して回避
- Windows コンソールは cp932 で、非ASCII (Wikimedia のファイル名等) を `print` すると `UnicodeEncodeError`。スクリプト先頭で `sys.stdout.reconfigure(encoding="utf-8")` する

## .gitignore のパターン精度

- `/tools/npr_offline/out/` は **`out/` ディレクトリしかマッチしない**。`run_styles.sh` が作る `out_pencil/` `out_brush_starry_night/` 等の `out_*` は無視されず、誤ってコミット対象になる
- 出力ディレクトリを接尾辞付きで量産する場合は **`out_*/` のようなワイルドカードパターン**で ignore すること

## Neural NPR: 「渦」が出ない構造的な理由（重要）

- Gram 行列損失は **局所統計しか測れず、空間構造（渦＝曲がった大スケール配置）を表現できない**。点・星・流れる筆致までは出るが、螺旋の渦は原理的に出ない。これは学習量の問題ではなく**損失の性質**
- PatchGAN は受容野が小さく、渦スケールの大構造を enforce できない（同じ壁）
- 平坦な空には追従すべき勾配が無いので、構造テンソル系（flow_paint）も空には渦を描けない（coherence がノイズで高く出て抑制が効かない）→ flow_paint では下地のうねりを**全面一様**にかけると空にも渦が見える
- 結論: 「色・流れる筆致」は neural で再現できる。「螺旋の渦」が欲しければ ①推論後に flow_paint で手続き的に重ねる（軽い） ②マルチスケール D GAN / SD-LoRA（重い）

## Gatys: スタイルの「筆致サイズ」は style_scale（VGG に通す解像度）で決まる

- `_style_targets` でスタイル画像を **content と同解像度に潰していた**のが筆致が小さく・歪む原因。Gram は CxC でサイズ非依存なので、**スタイルは content と別解像度で通してよい**
- probe（`probe_style_scale.py`）の結果: スタイルを**大きい px で通すほど筆致が大きくなる**。content 512 に対し **style_scale=1.0 付近が筆致最大**、それ以上（1.5〜2.5）は逆に細かくなる
- 直感と逆: scale を下げる（例 0.2）と渦が潰れて小さい点になり、上げる（1.0）と大きな流れる筆致になる
- feed-forward 蒸留（`train_feedforward.py`）でも同じ。スタイルを正方形に潰さず**アスペクト比保持 + style_scale 倍**で通すよう修正（`--style-scale` 追加）。前回 feed-forward が「スタイルが遠い」だったのは教師 Gatys 側の筆致条件がずれていた可能性

## リアルタイム・ボイリング: 決定論ネットでは不可能、確率的ネットが唯一の道（重要）

- 順伝播ネット（`transformer_net.py`）は **決定論的写像＝同じ入力に必ず同じ出力**。だから空など静止領域が止まり、手描きの「ボイリング」が出ない。これは画質や容量の問題ではなく**写像が確定的という性質**
- **入力をいじる方式は構造的に詰み**: 推論時に①加算ノイズ→「絵柄と無関係にノイズが乗る」 ②微小ワープ→「絵が丸ごと泳ぐ」。両方ボイリングにならない（手描きは「同じ絵を各フレーム独立に少し違う線で描き直す」＝入力ではなく生成過程の揺れ）
- **容量（n_blocks/base_ch）を上げても筆致はほぼ変わらない**（Gram の壁）。5/32→8/48 で content 損失が少し下がりノイズが微減する程度。速度は約2倍重くなる（121fps→52fps）。**小容量で十分**＝速度を取れ
- **Gatys はどう削ってもリアルタイム不可**（最速 steps20/256 で 5.5fps、本命 50/384 で 1.5fps）。オフライン教師専用
- **正解 = 確率的ネット**（`transformer_net_stochastic.py` / `train_stochastic.py`）: フレーム毎ノイズを **残差ブロックの特徴へ StyleGAN 流に注入**（入力ピクセルでなく筆致が作られる層で揺らす）。推論時に**空間は白色・時間だけ平滑**なノイズを与えると筆致が描き直される＝リアルタイム(107fps)でボイリング
- **知覚損失だけだとノイズ無視（決定論）へ縮退する**ので **mode-seeking 多様性損失**で「乱数が違えば出力も違え」を強制（noise_scale が 0→上昇すれば成功）
- **多様性を測る VGG 層は浅く（relu1_1=層0）**。深い層（relu3_2=層13）で測ると構造が変わり**道路・球が消える**（多様性＝低レベル筆致テクスチャに限定すべき）。content_weight も上げる（6→10）

## 色保持: Gram は全体パレットを場所無視で転写する → 局所色を取り戻す後処理

- Gram スタイル損失は星月夜の**全パレット（青/黒/黄/肌色）を場所を問わず**転写するので、平坦な空に黒塊・黄色・肌色が混入する。**局所の色（空の青）が無視される**のが原因
- 対策＝**輝度転写 + 非対称カラースプレッド**（後処理、リアルタイム維持、再学習不要）。`infer_stochastic.py` の `recolor_lab`:
  - L(明度)= ネット出力の筆致。`dark_clip`(L単位)で暗くなりすぎだけ防ぐ→**黒塊が消える**
  - 色(Lab a/b)= 入力の局所色を基準に、**b 軸の青方向(db<0)だけ増幅・黄方向(db>0)はゼロ**→「青の中で広くばらける／黄色は入らない」
  - 確定値: blue=3.0 yellow=0 a=0.5 dark=60

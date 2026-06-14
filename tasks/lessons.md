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

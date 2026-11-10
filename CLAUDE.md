# CLAUDE.md

このファイルは Claude Code (claude.ai/code) がこのリポジトリで作業する際のガイダンスを提供します。



## ビルド環境

- **Visual Studio 18** を使用。MSBuild の場所は環境依存のため**ハードコードせず `vswhere` で解決**する（ドライブ／エディション／バージョン非依存）。
- `vswhere` は VS Installer に同梱され、`${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe` に必ず存在する（VS 2017 以降の公式ロケーター）。

## ビルドコマンド

> 例は PowerShell。まず MSBuild のパスを一度だけ解決し、以降は `$msbuild` を使う。

**MSBuild のパスを解決 (セッションで一度だけ):**
```powershell
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe"
```

構成は **x64**（Win32 は使わない）。出力は `source\.build\x64\<Debug|Release>\`。

**ゲームとテストをビルド (Debug):** ソリューション全体ではなく、ゲーム（exe）とテストを別々にビルドする
```powershell
& $msbuild source\blue-sky.sln -t:blue-sky-exe -p:Configuration=Debug -p:Platform=x64 -v:minimal -nologo
& $msbuild source\blue-sky.sln -t:blue-sky-test -p:Configuration=Debug -p:Platform=x64 -v:minimal -nologo
```

**リビルド:** 古い .obj でリンクに失敗したら、そのプロジェクトだけ `-t:blue-sky:Rebuild` などで作り直す（ファイルを戻したときに更新時刻が古いままで作り直されないことがある）

**テスト実行 (Google Test):**
```powershell
& .\source\.build\x64\Debug\blue-sky-test.exe
# 既定では走らないベンチマーク（ECS の 1 万・10 万個など）
& .\source\.build\x64\Debug\blue-sky-test.exe --gtest_also_run_disabled_tests --gtest_filter=EcsBenchmark.*
```

PDB のエラー（C1041 など）が出たら、`mspdbsrv` を止めて `source\.build\x64\Debug\blue-sky.pdb` を消す。

## プロジェクト構成

ソリューションは6つのプロジェクトで構成:

- **common** (`extern/common/`) - 基本ユーティリティ (ログ, 例外。シングルトンはやめた : 依存は作り手・組み立ての根から渡す)
- **game** (`extern/game/`) - 抽象ゲームフレームワーク → common に依存
- **win** (`extern/win/`) - Windows ユーティリティ → common に依存
- **blue-sky** (`source/blue-sky.vcxproj`) - メインエンジンライブラリ → common, game, win に依存
- **blue-sky-exe** - ゲーム実行ファイルのエントリポイント
- **blue-sky-test** - Google Test ユニットテスト

## アーキテクチャ

### レイヤー階層

```
blue_sky (ゲーム固有)     ← アプリケーション層
    ↓
core (エンジン)           ← エンジン層
    ↓
extern/common, game, win  ← 基盤層
```

**注意:** core 層は blue_sky 層に依存しない（core から blue_sky への include は 0。新しく足さない）

### ECS (Entity Component System)

**ゲームの物体はすべて ECS で作る。** 以前の継承の仕組み（`ActiveObject`・`ActiveObjectManager`・`GameObject` と Player・Robot などの派生クラス）は 2026-11 に無くした。新しい物体も「部品（Component）の組み合わせ＋システム（System）」で書き、物の種類を継承の木で表さない。経緯と決めたことは `doc/ECS移行計画.md`。

- 土台は `source/core/ecs/`:
  - **EntityManager** (`EntityManager.h`) - 全 Entity・Component・System を管理する。GameMain が 1 つ持ち、シーンは `get_entity_manager()` で受け取る（テストは自分で作る）
  - **Entity** (`Entity.h`) - 一意の `EntityId`（二度と使わない通し番号）。ほかの Entity を指すときは `Entity*` を持ち続けず `EntityId` で持ち、`is_alive( id )`・`find( id )` で引く
  - **Component** (`Component.h`) - データの部品の基底。部品は型ごとの置き場（`StablePool`）に値で置かれ、外すまで場所が動かない
  - **System<...ComponentTypes>** (`System.h`) - その部品の組を持つ Entity を密な配列で回す。`update()` は System ごとに 1 回、中は `update_entity` を非仮想で回す
- 規模の前提 : Entity は最終的に **1 万を超える**想定で設計する（効率の点検と計測は `doc/ECS移行計画.md` の 6 章。ベンチマークは `test/ecs`）
- 本編の物体の部品とシステムは `source/blue_sky/ecs/component/`・`system/`（プレイヤー・敵・風船・スイッチ・目標地点への移動・当たり・描画のデータなど）。core 側は `source/core/ecs/component/`・`system/`
- **物体を作る** : `blue_sky/ObjectFactory.h`（種類名から作る `create_object( "medal" )`・名前で引く・種類で探す）。Lua とステージからは `GameObjectRef`（EntityId を包む値）で触る
- **本編の更新の順** : `blue_sky/ObjectUpdater.h` が本編のシステムを決まった順で動かす（`EntityManager::update()` では動かさないシステムがある。理由は計画書の段階 1・2）。当たりの決まりは `blue_sky/ecs/ContactRules.h`

**使用例:**
```cpp
// シーンの中（EntityManager は Scene::get_entity_manager() で受け取る）
auto* entity = get_entity_manager()->create_entity();
auto* transform = entity->add_component< TransformComponent >();
// システムは必要なものをコンストラクタで受け取る（priority の後の引数が渡る。シーンの中なら近道で得る）
get_entity_manager()->add_system< RenderSystem >( 1000, get_graphics_manager(), get_time_manager() );
// 描く System（is_render_system）は update() ではなく、シーンの render() で get_entity_manager()->render() から動く
```

### ステージ

- ステージは **TOML**（`source/media/stage/*.stage.toml`・`media/stage_ext/`）。物体は `[[object]] type = "…"`、スイッチの動きはスイッチの中に `on_turn_on = [ { do = "…", … } ]`、場所と関係ない条件は `[[event]] when = …`。書き方は `doc/ステージ形式の設計.md`
- 読む所 `blue_sky/stage/TomlStageParser`（ファイル → `StageData`、書き間違いは「ファイル:行」付きでログ）、当てる所 `StageBuilder`、動きは型付きの `Action`
- 以前の独自形式（`.stage`）は無い。古いファイルは `tools/stage_convert/convert_stage.py` で変換できる

### リソースと物理

- **リソースはハンドルで持つ** : モデル・メッシュ・テクスチャ・音は `core::ResourceHandle`（`ModelHandle`・`TextureHandle`・`SoundHandle` など）で持ち、使うたびに `resolve_*` で引く（解放済みなら nullptr）。生ポインタを持ち続けない。持ち主が 1 つのもの（剛体・定数バッファなど）は `unique_ptr`／RAII
- **物理は `core::physics::PhysicsManager` 1 つ**（Bullet を隠している）。Bullet の型を core/physics の外に出さない。剛体は RAII の `core::physics::RigidBody` を部品が持つ。物理エンジンが物体を指す印は `core/physics/EntityTag.h`（EntityId のタグ）

### メインゲームループ

- **App** (`blue_sky/App.h`) - ウィンドウ/アプリケーション管理。WinMain が作り、GameMain を持つ（シングルトンではない）
- **GameMain** (`blue_sky/GameMain.h`) - メインゲームクラス、全サブシステムを統括
- エントリポイント: `source/Main.cpp` (WinMain)

### サブシステムマネージャー

| マネージャー | 場所 | 役割 |
|-------------|------|------|
| GraphicsManager | `blue_sky/graphics/GraphicsManager.h` | モデル読み込み、描画 |
| PhysicsManager | `core/physics/PhysicsManager.h` | 物理の世界（Bullet を隠す。本編も ECS のシーンも同じ 1 つ） |
| SoundManager | `core/sound/SoundManager.h` | 音声リソース管理（ハンドルで引ける） |
| SceneManager | `blue_sky/SceneManager.h` | シーン遷移 |
| ScriptManager | `blue_sky/ScriptManager.h` | Lua スクリプティング (Sol2) |

### グラフィックスパイプライン

- **Direct3D11** ラッパー: `source/core/graphics/Direct3D11/`
- ゲーム固有グラフィックス: `source/blue_sky/graphics/Direct3D11/`
- HLSL シェーダー: `source/blue_sky/graphics/shader/`（読み込むのは `source/media/shader/*.fx`。Effects11 はやめ、自前の EffectParser で読む）

### サービス（マネージャー）の探し方

依存は**作り手・組み立ての根から受け取る**。サービスロケーター（以前の `core::Service`）は無い。core 層は GameMain を知らない。

- **組み立ての根**は `GameMain`（と `App`・`Main.cpp`）。マネージャーを作り、使う側に渡す
- **シーン**は `Scene( GameMain& )` で受け取り、`Scene::get_*_manager()`・`get_input()`・`get_time_manager()` などの近道で GameMain から得る
- **ECS のシステム**は必要なもの（物理・入力・時間・描画）をコンストラクタで受け取る。シーンが `add_system< T >( priority, get_physics_manager(), ... )` のように渡す（`add_system` は priority の後の引数をコンストラクタに渡す）
- **描画を使うロジック**（ECS のシステム・ステージ・UI）は GraphicsManager の全体ではなく、使う分だけの小さな窓口で受け取る : `graphics::ResourceResolver`（ハンドルから引くだけ）・`ResourceLoader`（読む・作る）・`RenderSettings`（空・地面・色・線・紙）・`Canvas2D`（2D と文字）。GraphicsManager が全部実装する。テストの偽物は `test/graphics/FakeGraphics.h`（doc/描画の窓口の分割.md）
- **描画の部品**は作り手の GraphicsManager から受け取る。シェーダーは `create_shader< T >( ... )` / `create_named_shader< T >( name, ... )` がコンストラクタの先頭に GraphicsManager を渡す（clone は引き継ぐ）。`Model::create_shader( GraphicsManager* )`・Line・MeshBuffer・Sprite も作り手が渡す
- **ゲームの物体のシステム**は `GameObjectContext`（物理・音・経過時間・delta_time）を ObjectFactory・ObjectUpdater から受け取る
- **新しく書くロジック**も、依存を引数・コンストラクタで受け取る（例: `Scene/OptionsController`）。シングルトンに触れないのでテストで動かせる
- テストでは偽物を引数で渡す。update を呼ばないなら nullptr でよい（例: `test/ecs/EnemyAiSceneEcsTest.cpp`）

### 失敗の扱い

- **続けられない失敗**（デバイス・シェーダー・起動に必須のファイル）は例外を投げる。型は `common::exception_base`（`std::exception` の派生。`common::exception<T>` もこれを継承）を基にする。受け取る側は `const std::exception&` で受ければよい
- **素材 1 つの失敗**（UI の画像・効果音など）は `nullptr`／`false` を返し、`debug_log` に理由を残す。黙って無視しない（未登録のシーン・未知の設定キー・保存の失敗も同じ）
- ログは `debug_log`（`log/blue-sky.log` とデバッガ）に出す。`std::cout` も GameMain が生きている間は同じところへ流れる

### 名前空間規則

- `core::` - エンジンコア
- `core::ecs` - Entity Component System
- `core::graphics::direct_3d_11` - D3D11 実装
- `core::physics`, `core::sound`, `core::animation` - サブシステム
- `blue_sky::` - ゲーム固有コード
- `common::` - ユーティリティライブラリ

## 主要な外部依存ライブラリ

- Direct3D 11, DirectInput, DirectSound (Windows)
- Bullet Physics 3.25
- Lua 5.4.3 + Sol2 3.2.3 (スクリプティング。今は主に開発用のコマンド入力)
- toml++ 3.4.0 (ステージの TOML)
- imgui 1.91.0 (デバッグUI)
- FBX SDK（ヘッダ 2020.2・lib 2020.3.9。モデルインポート）
- PortAudio・libogg・libvorbis・TinySoundFont・libremidi (音)
- FreeType 2.14.1・msdfgen 1.13.0 (文字の描画)
- ONNX Runtime 1.24 + DirectML (Neural NPR。exe と一緒に DLL を配る)
- Oculus SDK 1.43.0 (VR。今は `OCULUS_RIFT_ENABLED` が未定義で、リンクしていない)

ライブラリと素材のライセンス・配布のときにすることは `doc/ライセンス.md`、配布物に同梱する表示は `THIRD_PARTY_NOTICES.txt`。**ライブラリや素材を足したら、両方を更新する**。

## ゲームを自動で起動して確かめるとき

自動の確かめ（スクリプト・エージェント）でゲームを起動するときは、**必ず環境変数 `BLUE_SKY_BACKGROUND=1` を付ける**。使っている人のほかの作業を邪魔しないため（2026-11-11、利用者の指示）。

- 付けると : 窓を前面に出さずフォーカスも取らない（一番後ろに置く）・カーソルを隠さず閉じ込めない・入力を受け取らない・音を出さない・開発用のコンソールを開かない。描画とフレームの保存（`BLUE_SKY_CAPTURE_FRAMES`）は続く
- 確かめのスクリプトで、ゲームの窓を前面に出す処理（`SetForegroundWindow`・`AttachThreadInput`・Alt キーの送信など）を使わない
- ゲームは二重に起動できない（ミューテックス）。利用者が手でゲームを使っている間は自動の起動を止める
- 閉じるときは `CloseMainWindow` で閉じる（ログは `source/log/blue-sky.log`、落ちたときは `source/log/crash.log`）

## Python ツール / 検証ハーネス (tools/)

C++ 本体とは別に、検証用 Python ツールが `tools/` 配下にある。**環境は隔離必須**（システムの pip を汚さない）。

- システムの `python` は **3.14** で新しすぎ、一部 wheel が入らない → ツールごとに**プロジェクト内 venv** を使う。
- `uv` は PATH に無い。**`python -m uv`** で叩く。
  - venv 作成: `python -m uv venv tools/<name>/.venv --python 3.12`
  - 依存追加: `python -m uv pip install --python tools/<name>/.venv/Scripts/python.exe <pkg>`
- **Neural NPR オフライン検証** (`tools/npr_offline/`, Asana エピックB / 段階0a):
  - 使い方は同梱 `README.md`。venv は Python 3.12 + **torch cu128**（RTX 5070 Ti / Blackwell 対応。CUDA Toolkit 別途インストール不要、ドライバのみでよい）。
  - スタイライザ: `classic` / `pencil`（古典・モデル不要）と `neural`（VGG Gatys、`--style` 画像必須）。
  - 入力フレームはエンジン側ダンパーが生成: `source/Scene/DebugScene.cpp`（imgui「Frame Dump (Neural NPR)」→ `dump/color_%04d.png`）。
  - 進捗は `tasks/todo.md` の「Neural NPR」節。作業ブランチ `feature/neural-npr-offline-validation`。

## コードスタイル

- 命名規則: メソッド・メンバは `snake_case` (例: `get_instance()`, `component_list_`)
- クラス名: `PascalCase` (例: `GameMain`, `PhysicsManager`)
- クラス名の省略語: **2文字は全大文字、3文字以上はパスカルケース** (例: `UI`Renderer, `Msdf`Font, `Midi`Sequencer, `Fbx`FileLoader)
- メンバ変数: 末尾アンダースコア (例: `device_`, `width_`)
- ヘッダガード: `#pragma once`
- スマートポインタ: `std::unique_ptr` を推奨、`std::make_unique` を使用

### セッション継続

作業を再開するときは、まず以下を読むこと

- `tasks/todo.md` - 未着手タスクと進捗
- `tasks/lessons.md` - 過去の失敗と学び

変更があった場合、上記を更新すること。

### チーム編成

セッション継続の情報をもとに、チーム編成（最大3人）を行い並列作業せよ

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

**ソリューション全体をビルド (Debug):**
```powershell
& $msbuild source\blue-sky.sln -p:Configuration=Debug -p:Platform=Win32 -v:minimal
```

**ライブラリのみビルド:**
```powershell
& $msbuild source\blue-sky.vcxproj -p:Configuration=Debug -p:Platform=Win32 -v:minimal
```

**リビルド:**
```powershell
& $msbuild source\blue-sky.sln -p:Configuration=Debug -p:Platform=Win32 -v:minimal -t:Rebuild
```

**テスト実行 (Google Test):**
```powershell
& .\source\x64\Debug\blue-sky-test.exe
```

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

**注意:** core層はblue_sky層に依存すべきではない（現在TODOコメント付きで違反箇所あり）

### ECS (Entity Component System)

`source/core/ecs/` に配置:

- **EntityManager** (`EntityManager.h`) - 全Entity、Component、Systemを管理する。GameMain が 1 つ持ち、シーンは `get_entity_manager()` で受け取る（テストは自分で作る）
- **Entity** (`Entity.h`) - 一意のID保持
- **Component** (`Component.h`) - データコンポーネントの基底クラス
- **System<...ComponentTypes>** (`System.h`) - 特定のComponent群を操作するシステムの可変長テンプレート

**使用例:**
```cpp
// シーンの中（EntityManager は Scene::get_entity_manager() で受け取る）
auto* entity = get_entity_manager()->create_entity();
auto* transform = entity->add_component<TransformComponent>();
// システムは必要なものをコンストラクタで受け取る（priority の後の引数が渡る。シーンの中なら近道で得る）
get_entity_manager()->add_system<RenderSystem>( 1000, get_graphics_manager(), get_time_manager() );
// 描く System（is_render_system）は update() ではなく、シーンの render() で get_entity_manager()->render() から動く
```

**コンポーネント:** `source/core/ecs/component/` (TransformComponent, RigidBodyComponent, ModelComponent 等)

**システム:** `source/core/ecs/system/` (RenderSystem, PhysicsSystem, ParticleSystem)

### メインゲームループ

- **App** (`blue_sky/App.h`) - ウィンドウ/アプリケーション管理。WinMain が作り、GameMain を持つ（シングルトンではない）
- **GameMain** (`blue_sky/GameMain.h`) - メインゲームクラス、全サブシステムを統括
- エントリポイント: `source/Main.cpp` (WinMain)

### サブシステムマネージャー

| マネージャー | 場所 | 役割 |
|-------------|------|------|
| GraphicsManager | `blue_sky/graphics/GraphicsManager.h` | モデル読み込み、描画 |
| PhysicsManager | `core/physics/PhysicsManager.h` | Bullet Physics ラッパー |
| SoundManager | `core/SoundManager.h` | 音声リソース管理 |
| SceneManager | `blue_sky/SceneManager.h` | シーン遷移 |
| ScriptManager | `blue_sky/ScriptManager.h` | Lua スクリプティング (Sol2) |

### グラフィックスパイプライン

- **Direct3D11** ラッパー: `source/core/graphics/Direct3D11/`
- ゲーム固有グラフィックス: `source/blue_sky/graphics/Direct3D11/`
- HLSL シェーダー: `source/blue_sky/graphics/shader/`

### サービス（マネージャー）の探し方

依存は**作り手・組み立ての根から受け取る**。サービスロケーター（以前の `core::Service`）は無い。core 層は GameMain を知らない。

- **組み立ての根**は `GameMain`（と `App`・`Main.cpp`）。マネージャーを作り、使う側に渡す
- **シーン**は `Scene( GameMain& )` で受け取り、`Scene::get_*_manager()`・`get_input()`・`get_time_manager()` などの近道で GameMain から得る
- **ECS のシステム**は必要なもの（物理・入力・時間・描画）をコンストラクタで受け取る。シーンが `add_system< T >( priority, get_physics_manager(), ... )` のように渡す（`add_system` は priority の後の引数をコンストラクタに渡す）
- **描画の部品**は作り手の GraphicsManager から受け取る。シェーダーは `create_shader< T >( ... )` / `create_named_shader< T >( name, ... )` がコンストラクタの先頭に GraphicsManager を渡す（clone は引き継ぐ）。`Model::create_shader( GraphicsManager* )`・Line・MeshBuffer・Sprite も作り手が渡す
- **ゲームのオブジェクト**は `GameObjectContext`（物理の世界・音・経過時間・delta_time）を ActiveObjectManager から受け取る
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
- Lua 5.3.4 + Sol2 3.2.3 (スクリプティング)
- imgui 1.91.0 (デバッグUI)
- FBX SDK 2020.2 (モデルインポート)
- Oculus SDK 1.43.0 (VRサポート)

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

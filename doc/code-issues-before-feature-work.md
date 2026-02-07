# 高優先度フィーチャー実装前に整理すべきソースコードの問題

高優先度の 6 機能（ポーズメニュー、オプション画面、チュートリアル、HUD 充実、ローディング画面、ダメージフィードバック）を実装する前に、解決または整理しておくべき問題をまとめる。

---

## 1. アーキテクチャ上のブロッカー

### 1.1 シーンスタックの欠如（深刻度: 致命的）

**現状**: シーンは常に完全に置き換えられる。シーンの重ね合わせ（スタック）ができない。

```
// source/blue_sky/GameMain.h:136
std::unique_ptr< Scene > scene_;  // シーンは1つだけ

// source/blue_sky/GameMain.cpp:709-718 (setup_scene)
scene_.reset();                    // 現在のシーンを破棄
core::ecs::EntityManager::get_instance()->clear();  // ECS もクリア
scene_.reset( SceneManager::get_instance()->generate_scene( scene_name ) );
```

**影響を受ける機能**: ポーズメニュー、オプション画面、チュートリアル

**必要な対応**: シーンスタック機構の導入。ゲームプレイを維持したまま、その上にメニューシーンを重ねられるようにする。

---

### 1.2 メインループが一時停止に非対応（深刻度: 致命的）

**現状**: `GameMain::update()` はすべてのサブシステムを無条件に更新する。

```
// source/blue_sky/GameMain.cpp:304-379 (update)
time_manager_.update( main_loop_->get_elapsed_sec() );
get_sound_manager()->update();   // 常に更新
direct_input_->update();          // 常に更新
input_->update();                 // 常に更新
scene_->update();                 // 常に更新（物理・AI含む）
render();                         // 常に描画
```

**影響を受ける機能**: ポーズメニュー（ポーズ中も物理演算・AI が動き続ける）

**必要な対応**: ゲーム状態（Playing / Paused / Loading 等）に応じてサブシステムの更新を選択的に制御できるようにする。

---

### 1.3 入力システムに消費・優先度の概念がない（深刻度: 高）

**現状**: 入力はグローバルに処理され、UI が入力を「消費」してゲームプレイに渡さないようにする仕組みがない。

```
// source/blue_sky/Input.h:58
unsigned int state_[ static_cast< int >( Button::MAX ) ];  // 生の状態配列

// source/blue_sky/Input.cpp:210
GetAsyncKeyState()  // Windows API を直接使用、イベントキューなし
```

**影響を受ける機能**: ポーズメニュー・オプション画面の操作中もプレイヤーが動いてしまう

**必要な対応**: 入力レイヤー（UI 層 → ゲーム層）の優先度制御。UI がアクティブなとき、ゲームプレイ層への入力をブロックする。

---

### 1.4 UI レイヤーの不在（深刻度: 高）

**現状**: ゲーム UI はすべて `GamePlayScene::render_sprite()` にハードコードされている。

```
// source/Scene/GamePlayScene.cpp:89
ui_texture_ = get_graphics_manager()->load_named_texture( "ui", "media/image/item.png" );

// source/Scene/GamePlayScene.cpp:1773-1819
// ロケット・傘・石・スコープの表示がマジックナンバーで直接配置
get_width() - src_rect.width() - 5, get_height() - src_rect.height() - offset - 5
```

- ボタン、スライダー、パネル等の UI 部品がない
- 各シーンが独自に描画処理を書いており、再利用できない
- テキスト描画はフォントサイズ 64pt 固定 (`DirectWrite.h:31`)

**影響を受ける機能**: HUD、オプション画面、チュートリアル、ポーズメニュー

**必要な対応**: 再利用可能な UI フレームワーク（またはゲーム用 ImGui 利用の拡張）を構築する。

---

## 2. コード品質の問題

### 2.1 巨大ファイル / 神クラス（深刻度: 高）

| ファイル | 行数 | 問題 |
|----------|------|------|
| `Scene/GamePlayScene.cpp` | 1,862行 | ゲームループ、描画、サウンド管理、コマンド実行、ステージ読込、物理をすべて1ファイルで処理 |
| `GameObject/Player.cpp` | 1,208行 | 移動、衝突、アニメーション、アイテム管理、状態管理を1クラスに集約 |

**新機能への影響**: HUD やダメージフィードバックを追加すると、これらのファイルがさらに肥大化する。

**必要な対応**: 責務の分割。GamePlayScene は描画・コマンド・物理をそれぞれ別クラスに委譲すべき。

---

### 2.2 GameObject / ECS ハイブリッド状態（深刻度: 中）

**現状**: 2つのオブジェクト管理システムが混在している。

- **GameObject 継承階層**: Player, Robot, Balloon 等 18 クラスが ActiveObject を継承
- **ECS**: コアに Entity-Component-System があるが、補助的にしか使われていない

```
// source/GameObject/GameObject.h:41
/// @todo ECS に移行する
```

既に `doc/リファクタリング計画.md` に ECS 移行計画があるが未完了。

**新機能への影響**: 新機能を追加する際、どちらのシステムに実装すべきか判断が必要。

**必要な対応**: 新機能を書く前に方針を決める。高優先度機能は ECS を前提にするか、既存 GameObject に乗せるか。

---

### 2.3 マジックナンバー / ハードコード値（深刻度: 中）

```
// source/GameObject/Player.h:90-103
float get_height_to_die() const { return 15.f; }
float_t get_max_run_velocity() const { return 4.f; }
float_t get_rocket_initial_velocity() const { return 10.f; }

// source/GameObject/Player.cpp
std::min( 3.f, ... )           // Line 136: バルーン速度制限
action_timer_ >= 2.5f          // Line 153: ロケットタイムアウト
< -7.5f, > 10.f, < 30.f       // Line 182: 各種閾値

// source/Scene/GamePlayScene.cpp
static float a = 0.0025f;     // Line 1412: デバッグ用回転値が本番に残存
action_bgm_after_timer_ = 2.f // Line 1346: BGM フェードタイミング
```

**必要な対応**: 定数クラス or Config 値に抽出。ゲームバランス調整も容易になる。

---

### 2.4 デッドコード / コメントアウトされたコード（深刻度: 中）

主要な箇所:

| ファイル | 行 | 内容 |
|----------|-----|------|
| `Player.cpp` | 53 | コメントアウトされた剛体初期化 |
| `Player.cpp` | 253 | `// v *= 1.01f;` ロケット速度倍率 |
| `Player.cpp` | 468-469 | `// test` コメント付きの物理テストコード |
| `Robot.cpp` | 200-224 | 25行以上の巡回ロジックがコメントアウト |
| `GamePlayScene.cpp` | 157-161 | `#if 0` で無効化されたアンロード処理 |
| `GamePlayScene.cpp` | 1360-1370 | コメントアウトされたオーディオミキシング |
| `GamePlayScene.cpp` | 1412 | `static float a = 0.1f; a += 0.0025f;` デバッグコードが本番に残存 |

---

### 2.5 関数名のタイポ（深刻度: 低、修正容易）

```
// source/GameObject/Robot.cpp
bool Robot::caluclate_target_visible() const   // → calculate
bool Robot::caluclate_target_lost() const      // → calculate
bool Robot::caluclate_collide_object_to_swtich_off()  // → calculate, switch
```

---

### 2.6 生ポインタ / メモリ管理の不一致（深刻度: 中）

`unique_ptr` と生の `new/delete` が混在している:

```
// source/Scene/GamePlayScene.cpp
stage_config_( new Config() )       // Line 64: unique_ptr に入れるが
player_ = new Player();             // Line 96: 生ポインタで保持

// source/GameObject/ActiveObject.cpp
object_constant_buffer_( new ObjectConstantBuffer() )  // Line 32
delete object_constant_buffer_;                         // Line 70: 手動 delete

// source/Scene/GamePlayScene.cpp
delayed_command_list_.push_back( new DelayedCommand(...) )  // Line 631: 生ポインタのリスト
delete *i;  // Line 172-175, 1302: 手動ループ削除
```

---

### 2.7 null チェックの欠落（深刻度: 中）

```
// source/Scene/GamePlayScene.cpp:1442
get_sound_manager()->get_sound( "fin" )->get_current_position()
// get_sound() が nullptr を返すとクラッシュ

// source/Scene/GamePlayScene.cpp:1396
const_cast< Balloon* >( player_->get_balloon() )->update();
// get_balloon() が nullptr の場合のチェックなし
```

---

### 2.8 コードの重複（深刻度: 低）

```
// source/GameObject/Player.cpp:311-342, 361-393
// 足場チェックの4方向レイキャストが2箇所で約50行重複
check_on_footing( center, ray_length ) ||
check_on_footing( center - right - front, ray_length ) ||
check_on_footing( center + right - front, ray_length ) || ...
```

---

## 3. 外部ライブラリ / ビルドの問題

### 3.1 Bullet Physics の VS2010 バイナリ（深刻度: 中）

```
// source/lib.cpp:20-28
#pragma comment( lib, "BulletCollision_vs2010_debug.lib" )
#pragma comment( lib, "BulletDynamics_vs2010_debug.lib" )
```

VS2022 で VS2010 ビルドのバイナリをリンクしている。ABI 互換性のリスクあり。

### 3.2 非推奨の DirectInput（深刻度: 中）

```
// doc/20210227 - 西田さんソースレビュー.md:67-68
// "DirectX8よりもXInputを使いましょう。"
```

DirectInput は非推奨。XInput への移行が推奨されているが未対応。

### 3.3 非推奨の自作スマートポインタ（深刻度: 低）

```
// source/extern/common/auto_ptr.h, safe_ptr.h
[[deprecated("please use std::unique_ptr")]]
```

使われている箇所がないか確認し、削除すべき。

---

## 4. 推奨アクション（優先順位付き）

高優先度フィーチャーの実装に必要な**最小限の前準備**として、以下の順番で取り組むことを推奨する。

### フェーズ 0: すぐ片付けられるクリーンアップ

| # | 作業 | 対象ファイル | 工数目安 |
|---|------|-------------|---------|
| 0-1 | デッドコード削除（コメントアウト・`#if 0`） | Player.cpp, GamePlayScene.cpp, Robot.cpp | 小 |
| 0-2 | 関数名タイポ修正 (`caluclate` → `calculate`) | Robot.cpp, Robot.h | 小 |
| 0-3 | 本番コード内のデバッグ残骸削除 (`static float a`) | GamePlayScene.cpp:1412 | 小 |
| 0-4 | 非推奨スマートポインタの使用箇所確認と削除 | auto_ptr.h, safe_ptr.h | 小 |

### フェーズ 1: 高優先度フィーチャーの前提条件（必須）

| # | 作業 | ブロックしている機能 | 工数目安 |
|---|------|---------------------|---------|
| 1-1 | **シーンスタック機構の導入** | ポーズメニュー、オプション画面、チュートリアル | 中 |
| 1-2 | **ゲーム状態管理の追加**（Playing/Paused/Loading） | ポーズメニュー、ローディング画面 | 中 |
| 1-3 | **入力レイヤーの導入**（UI 層の入力消費） | ポーズメニュー、オプション画面 | 中 |
| 1-4 | **UI フレームワークの構築**（ボタン・パネル・テキスト） | オプション画面、HUD、チュートリアル | 大 |

### フェーズ 2: 品質改善（並行作業可能）

| # | 作業 | 効果 |
|---|------|------|
| 2-1 | GamePlayScene の責務分割 | HUD・ダメージフィードバック追加時の保守性向上 |
| 2-2 | マジックナンバーの定数化 | バランス調整の容易化 |
| 2-3 | 生ポインタ → unique_ptr 統一 | メモリ安全性の向上 |
| 2-4 | null チェックの追加 | クラッシュ防止 |

---

## 依存関係図

```
ポーズメニュー ─── 1-1 シーンスタック
               ├── 1-2 ゲーム状態管理
               ├── 1-3 入力レイヤー
               └── 1-4 UI フレームワーク

オプション画面 ─── 1-1 シーンスタック
               ├── 1-3 入力レイヤー
               └── 1-4 UI フレームワーク

チュートリアル ─── 1-1 シーンスタック
               ├── 1-3 入力レイヤー
               └── 1-4 UI フレームワーク

HUD 充実 ──────── 1-4 UI フレームワーク
               └── 2-1 GamePlayScene 分割（推奨）

ローディング画面 ─ 1-2 ゲーム状態管理
               └── 1-4 UI フレームワーク

ダメージフィードバック ── （既存コード内で対応可能、ブロッカーなし）
```

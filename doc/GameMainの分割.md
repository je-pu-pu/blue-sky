# GameMain の分割（設計上の問題 D2）

2026-11-11、利用者が決定 :「GameMain が窓口のまま、中の手順だけを部品に移す」形で分割する。呼び出す側（シーン・Lua・開発用の環境変数）の使い方は変えない。

## 1. 背景

- `source/blue_sky/GameMain.cpp` は 1639 行（整備の間に 1462 行から増えた）。部品の組み立て・毎フレームの流れ・シーンの切り替え・Lua への登録・開発用の機能（環境変数 11 種類）・imgui の開発用の窓が 1 つに入っている。
- 変更が集中する（直近 1 か月のコミットの約 3 割が GameMain.cpp を触った）。シーンの切り替えはロジックが多いのにテストできない。開発用の機能が増えるたびに GameMain が太る。
- 呼び出す側の今の使い方（確かめ済み）: シーンは基底（`Scene.h`）の近道 28 個で GameMain の機能を使う。シーンの切り替えはシーンの基底に要求を出し、GameMain がフレームの区切りで集めて実行する（GameMain の外から切り替えの関数を直接呼ぶ所は無い）。開発用の機能は環境変数で動き、ほかのコードからは呼ばれない。

## 2. 分けた後の形

| クラス（場所） | 受け持ち | GameMain から移すもの |
|---|---|---|
| `SceneStack`（blue_sky/SceneStack.h・.cpp） | ベースのシーンとオーバーレイを持ち、切り替えの要求を集めてフレームの区切りで実行する（積む・外す・置き換える・読み込み中の画面・ESC） | `scene_`・`overlay_scene_stack_`・`scene_requests_`・`pending_base_request_`・`next_scene_arguments_`・`is_loading_frame_`・起動時のオーバーレイの待ち。`collect_scene_requests`・`process_scene_requests`・`replace_base_scene_with_loading_screen`・`handle_escape`・`setup_scene( 名前, 引数 )`・`generate_scene`・`push_overlay_scene`・`pop_overlay_scene` |
| `ScriptBindings`（blue_sky/ScriptBindings.h・.cpp） | Lua に関数を登録する | `setup_script_command` |
| `DevOptions`（blue_sky/dev/DevOptions.h・.cpp） | 開発用の環境変数を起動時に 1 回だけ読む値の入れ物（BLUE_SKY_START_SCENE・START_STAGE・START_OVERLAY・CAPTURE_FRAMES・CAPTURE_DIR・CAPTURE_GLYPH_ATLAS・DEBUG_DISPLAY_CHANGE・DEBUG_FIXED_STEP_MSEC・DEBUG_FRAME_TIME・DEBUG_SCENE_SEQUENCE など） | 各所に散らばった環境変数の読み取り |
| `DevTools`（blue_sky/dev/DevTools.h・.cpp） | 開発用の機能（フレームの保存・フレームの時間の記録・シーンを順に替える・解像度の変更の試し・経過時間の固定） | `setup_frame_capture`・`capture_frame_if_requested`・`setup_frame_time_measurement`・`record_frame_time`、シーンを順に替える処理と関連のメンバ |
| `DebugConsole`（blue_sky/dev/DebugConsole.h・.cpp） | imgui の開発用の窓（コマンド入力・シーンの一覧） | `render_console_window`・`render_scene_list_window`、コマンド入力の状態（`is_command_mode_`・`user_command_`） |

GameMain に残すもの : 部品の組み立て（コンストラクタ）・毎フレームの流れ（update・render から各部品を呼ぶ）・設定と表示の設定・入力の受け渡し・シーンが使う近道。GameMain.cpp は 700〜800 行ほどになる見込み。

## 3. 守ること

1. **GameMain が窓口のまま** : 切り出した部品は GameMain の持ち物（unique_ptr）にし、外に見せない。シーンや Lua が部品を直接取り出す窓口（`get_scene_stack()` など）は作らない。シーンの近道 28 個と切り替えの要求の出し方、Lua の関数、環境変数の名前は変えない。
2. **テストできる形** : `SceneStack` はシーンを作る関数（名前と引数 → `unique_ptr< Scene >`）などの依存を引数・コンストラクタで受け取り、偽のシーンでテストできるようにする（積む・外す・置き換える・ESC・読み込み中の画面・未登録のシーン・起動時のオーバーレイ）。`DevOptions` は環境変数の読み取りを関数で受け取れるようにしてテストする。
3. **作る順・壊す順** : GameMain のデストラクタで、`SceneStack`（シーン）を最初に壊す（今の「最初にシーンを壊す」決まり。設定・文字列表・ECS より先）。部品が使う GameMain の持ち物より、部品が先に壊れるようにメンバの順を決め、コメントに書く。
4. **振る舞いを変えない** : 切り出すたびにビルド・テスト・撮り比べ（全シーン・シーンを順に替える確かめ・本編の開発用の確かめ）を変える前と同じにする。

## 4. 進め方

1. `DevOptions`（環境変数を 1 か所で読む）
2. `SceneStack`（＋テスト）
3. `ScriptBindings`
4. `DevTools`・`DebugConsole`
5. 結果を 5 章に書く（行数・確かめた結果）。doc/設計上の問題の洗い出し.md の D2 を完了にする

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

## 5. 結果（2026-11-11 完了）

4 章の順に 4 つのコミットで分けた。どの段階もビルド・テスト・撮り比べを変える前と同じにしてからコミットした。

| 段階 | コミット | したこと | 足したテスト |
|---|---|---|---|
| 1 | ba86aa83 | `DevOptions`（blue_sky/dev）: GameMain の 6 か所でその場で読んでいた開発用の環境変数 10 種類を、起動時に 1 回だけ読む | DevOptionsTest 6 件（偽の環境変数で読み方・形の違う値・空の値） |
| 2 | 62b88cf5 | `SceneStack`（blue_sky）: ベースとオーバーレイ・要求のキュー・読み込み中の画面・ESC・起動時のオーバーレイ・シーンの引数・ステージ名 | SceneStackTest 14 件（偽のシーンで積む・外す・置き換える・ESC・読み込み中の画面と残りの要求の順・未登録・作れないシーン・引数・起動時のオーバーレイ・壊す順） |
| 3 | 58a41c34 | `ScriptBindings`（blue_sky）: Lua の関数の登録 | ScriptBindingsTest 3 件（切り替えの関数が引数つきの要求を出す・nil はスクリプトのエラー） |
| 4 | d758043c | `DevTools`・`DebugConsole`（blue_sky/dev）: フレームの保存・フレームの時間・シーンを順に替える・解像度の変更の試し・経過時間の固定と、imgui の開発用の窓 | DevToolsTest 5 件（保存するフレーム・解像度の変更のフレーム・シーンを順に替える間隔） |

テスト 585 → 613 件、すべて成功。

### 行数

| ファイル | 分ける前 | 分けた後 |
|---|---|---|
| GameMain.cpp | 1638 | 925 |
| GameMain.h | 310 | 278 |
| SceneStack.cpp・.h | — | 351・129 |
| ScriptBindings.cpp・.h | — | 155・42 |
| dev/DevOptions.cpp・.h | — | 129・53 |
| dev/DevTools.cpp・.h | — | 215・91 |
| dev/DebugConsole.cpp・.h | — | 218・50 |

GameMain.cpp は見込み（700〜800 行）より多い 925 行。残りは部品の組み立て（コンストラクタ 約 230 行。シェーダーの読み直し・imgui の初期化・代わりのフォント・設定ファイルの整理・テーマを含む）・毎フレームの流れ・表示の設定・入力の受け渡し・カーソル・部品へ渡す仕事（Hooks を作る関数 2 つ 約 110 行）。設定と表示（設定ファイルの整理・テーマ・表示の変更）は 2 章で「GameMain に残す」としたので、今回は分けていない。

### 決めたこと

1. **部品の仕事は Hooks（std::function の入れ物）で受け取る** : SceneStack・DevTools は GameMain を知らない。シーンを作る・音のグループを替える・片付ける・読み込み中を表示する・セーブする・カーソル・終える・効果音・描画結果の保存・解像度の変更は GameMain が渡す。テストでは偽物を渡し、設定しない仕事は何もしない。
2. **ステージ名は SceneStack が持つ** : ベースの置き換えの要求で変わり、作ったシーンに渡すため。GameMain::get_stage_name・set_stage_name は残し、中で SceneStack に頼む（シーンの近道 get_stage_name はそのまま）。
3. **外から呼ばれていなかった GameMain::push_overlay_scene・pop_overlay_scene・generate_scene は消した** : 切り替えは request_scene（とシーンの set_next_scene・Lua）だけで出す。request_scene・has_overlay_scene・get_current_scene・get_next_scene_arguments は残した。
4. **Scene に GameMain なしのテスト用のコンストラクタ（protected）を足した** : SceneStack を偽のシーンで確かめるため。Scene が持つ GameMain を参照からポインタにした（テスト用のコンストラクタでは nullptr。近道・update() は使えない）。
5. **壊す順** : scene_stack_ を GameMain の最後のメンバにし、その前に dev_tools_・debug_console_ を置いた。デストラクタでも scene_stack_ → debug_console_ → dev_tools_ の順に明示して壊す（シーンは設定・文字列表・ECS を使う）。
6. **DevOptions は GameMain が読む環境変数だけ** : シーンが読む開発用の環境変数（BLUE_SKY_DEBUG_DUMP_STAGE・PRESS_SWITCHES など）はシーンのもので、シーンに DevOptions を渡す窓口を作らないため入れなかった。App が読む BLUE_SKY_BACKGROUND も App のまま（GameMain より先に要る）。
7. **ScriptBindings は登録の後に持ち続けない** : 登録した関数が窓口（描画・音・物体・物理のデバッグ表示・切り替えの要求）を直接持つので、GameMain のメンバにしていない（ScriptBindings::bind だけ）。物体の窓口を直接持たせるため、登録を ObjectFactory を作った後に動かした。
8. **core::logger を inline にした** : ヘッダで定義していて、2 つ目の .cpp（DebugConsole）が含めると二重定義になったため。

### 確かめた結果

- 経過時間を固定して（BLUE_SKY_DEBUG_FIXED_STEP_MSEC=16）、変える前の exe と各段階の exe を撮り比べた（前の担当のスクリプトを元にした C:\Users\ando\AppData\Local\Temp\d2\ の scenes.ps1・seq.ps1・run.ps1・all.ps1）。
  - 全シーン 18 項目（16 シーン＋options の引数つき＋未登録のシーン。BLUE_SKY_START_SCENE・START_STAGE・START_OVERLAY）・本編の 7 ステージ（0-0・0-1・1-1・1-2・2-0・2-1・2-3）: 画素もログも同じ。canvas_test・music_game_prototype・particle_system_test は変える前の exe 同士でも同じだけ揺れる。2-0 の 240 フレーム目・ending の 120 フレーム目などの小さな差（8 を超える画素 0.05〜1.3%）は、変える前の exe も 2 つの状態を行き来するので揺れ。
  - シーンを順に替える（BLUE_SKY_DEBUG_SCENE_SEQUENCE。10 回の置き換え・11 枚）: ログが同じ、画面は差の平均 0.14 以下。
  - フレームの保存（BLUE_SKY_CAPTURE_FRAMES・CAPTURE_DIR）・解像度の変更の試し（BLUE_SKY_DEBUG_DISPLAY_CHANGE。BLUE_SKY_SETTINGS_DIR を別にして）・フレームの時間（BLUE_SKY_DEBUG_FRAME_TIME。ログの形とシーン名が同じ、数値は時間なので違う）。
  - 本編の開発用の確かめ（BLUE_SKY_DEBUG_DUMP_STAGE・PRESS_SWITCHES・VISIT_OBJECTS・THROW_STONES・PLAYER_SCRIPT）: 数と結果の行が同じ。石投げの 1-1 だけ、実時間で動かすとスイッチの入・切が変わったが、経過時間を固定すると変える前の exe も同じ「入」になった（実時間の違いによる揺れ）。
  - どの起動も閉じると終了コード 0・crash.log なし。
- 確かめていないこと : imgui の開発用の窓（F1 のコンソール・シーンの一覧）は、窓を前に出さずに開く手段が無く、手で開いていない。ESC・F キーも手で押していない（SceneStack の ESC はテストで確かめた）。BLUE_SKY_CAPTURE_GLYPH_ATLAS は DevTools のテストで確かめ、実機では動かしていない。

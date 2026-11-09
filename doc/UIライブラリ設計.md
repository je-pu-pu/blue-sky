# UI ライブラリ設計（オプション画面・設定画面の基盤）

- 作成: 2026-11-08（第 2 版：サブエージェントによる厳格レビュー 25 件を反映）
- ブランチ: `feature/ui-library`（`develop` から分岐）
- 目的: 一般的な PC ゲームのオプション画面・設定画面を「部品を並べて、設定項目に結びつけるだけ」で組めるようにする

---

## 1. 現状調査（2026-11-08 時点のコード）

### 1.1 既存資産

| 場所 | 内容 |
|---|---|
| `core/ui/Widget.h` | ウィジェット基底（座標・サイズ・フォーカス・ヒント文字列） |
| `core/ui/WidgetContainer` | 縦一列に並べ、上下キー／マウスホバーでフォーカス移動 |
| `core/ui/Button` / `Slider` / `SelectBox` | 部品 3 種 |
| `core/ui/VerticalMenu` | テキストだけの縦メニュー（WidgetContainer と別系統） |
| `core/ui/Renderer.h` | 抽象描画 IF（`draw_text` / `draw_rect` / 仮想座標変換） |
| `blue_sky/ui/Renderer` | 上記の具象。仮想 1920x1080 → 物理ピクセル |
| `Scene/OptionsScene` / `PauseMenuScene` | 上記を使った画面 |
| `GameMain` | `overlay_scene_stack_` によるオーバーレイ積み上げ、ESC のグローバル処理 |
| `MsdfTextRenderer` | `measure_text_width/height` を持つが `GraphicsManager` からは非公開だった |

### 1.2 問題点（実コードで確認済み）

**A. 不具合**

| # | 問題 | 根拠 |
|---|---|---|
| A1 | **`Button::A` に「左クリック」「Enter」「パッド A」が 1 ビットに混ざっている**ことによる不具合群。① フォーカス中の Button は画面のどこをクリックしても押される ② `VerticalMenu`（ポーズ画面）も同様 ③ `Slider` は Enter を押すとマウスの x 位置へ値が飛ぶ／別の行の高さでもトラックの x 範囲をクリックするとドラッグが始まる ④ `SelectBox` は Enter 時にマウスが矢印上にあると値が変わる | `Button.cpp`、`VerticalMenu.cpp:111`、`Slider.cpp:108`、`SelectBox.cpp:77`、`Input.cpp`（A=MOUSE_L+VK_RETURN+パッド） |
| A2 | ポーズ → オプション → 戻る で**ゲームに戻る**（ポーズに戻らない） | オーバーレイ遷移が `pop + push` |
| A3 | ESC で閉じても変更が**無条件に保存**される | `~OptionsScene()` で `save_settings()`。さらに `~GameMain()` が実状態で `audio.mute` / `graphics.full_screen` を上書き保存 |
| A4 | 解像度を**左右キーで送るたびに即変更**、確認なし | `resolution_select_.set_on_change` |
| A5 | FOV を変えてもプレイ中のカメラに反映されない | FOV スライダーに `on_change` 無し。`GamePlayScene` 生成時（`GamePlayScene.cpp:88`）にのみ読む |
| A6 | コード上の既定値がバラバラ | FOV: `OptionsScene`=90 / `GamePlayScene`=90、音量: `GameMain`=1.0。設定ファイルはローカル管理外 |
| A7 | **現在の解像度が DXGI モード一覧に無いと、画面を閉じるだけで最小解像度が保存される** | `find_current_resolution` が 0 を返し、`save_settings` がそれを書く |
| A8 | **設定がストアを経由せず変わる経路**が複数ある：F2（ミュート）・F5（フルスクリーン）、DXGI 既定の Alt+Enter。オプション画面のフルスクリーンボタンは `App::set_full_screen` を呼ばず App のフラグが古いまま | `GameMain.cpp` の F キー処理、`OptionsScene.cpp` |
| A9 | 仮想座標のスケールが**縦横別**で、16:9 以外の解像度で UI が歪む。`font_size==0`（既存部品すべて）は物理 64px のまま拡縮されない | `blue_sky/ui/Renderer.cpp` |

**B. 設計上の問題**

| # | 問題 |
|---|---|
| B1 | `VerticalMenu` と `WidgetContainer` の 2 系統がフォーカス・ホバー処理を重複実装 |
| B2 | レイアウトが縦一列・固定高さのみ。スクロール・タブ・見出し・横並びのボタン行が作れない |
| B3 | UI からテキスト計測ができない |
| B4 | 色が部品ごとにハードコード。テーマが無い |
| B5 | UI 専用の入力が無い（ゲーム用ボタン流用）。キャンセル・タブ切替・キーリピート・ホイールが無い |
| B6 | ESC がシーン名で分岐するグローバル処理。UI が横取りできない |
| B7 | 設定値の窓口が無い。「保留／適用／元に戻す／既定値」の概念が無い |
| B8 | 文字列が英語ハードコード。操作ヒントが部品のコンストラクタに埋め込み |
| B9 | キー割り当て変更ができない（設定ファイルは 1 文字＋3 名のみ解釈） |
| B10 | 無効化状態・説明文欄・モーダルダイアログが無い |
| B11 | `core::ui` のテストがゼロ |

---

## 2. 要件

### 2.1 必須（今回実装する）

| ID | 機能 | 受け入れ条件 |
|---|---|---|
| R1 | タブ | グラフィックス／サウンド／操作 を Q/E・LB/RB・タブ見出しクリックで切替 |
| R2 | スクロールするリスト | フォーカス移動で自動スクロール、ホイール対応、はみ出しは描画しない |
| R3 | 部品 | 見出し・ラベル（簡易折り返し）・ボタン・トグル・スライダー・選択肢・キー割り当て |
| R4 | ナビゲーション | 縦リスト＋横ボタン行の混在、キーリピート、ホバーでフォーカス、**クリックはカーソル下の部品にのみ** |
| R5 | 決定／キャンセル | 決定 = Enter / パッド A、キャンセル = Esc / パッド B、クリック = 左ボタン（決定とは別）。右クリックは使わない |
| R6 | 設定モデル | 保留値／確定値／既定値、スナップショットとロールバック、部品は値を持たずストアに結びつく |
| R7 | 反映方式 | 即時（音量・感度・FOV・キー割り当て：触った瞬間に反映、戻るときに自動確定）と保留（解像度・フルスクリーン：「適用」で反映） |
| R8 | 表示変更の確認 | 適用後「維持しますか？（15）」→ 時間切れ・戻すで旧値へロールバック |
| R9 | モーダルダイアログ | 背面の入力を遮断。コールバックから次のダイアログを開ける |
| R10 | キー割り当て | 決定 → キー待ち → 押したキーを主キーに。Esc で中止。他ボタンの主キーと重複したら入れ替え |
| R11 | テーマ | 色・フォントサイズ・余白・行高を 1 か所で定義 |
| R12 | 文字列テーブル | キーで引く。未登録はキーを表示。UTF-8 |
| R13 | 操作ヒント | フォーカス部品が使える操作を申告し、最後に使ったデバイス（キーボード／パッド）の表記でフッターに出す |
| R14 | オーバーレイを積む | ポーズ → オプション → 戻る でポーズに戻る |
| R15 | 既存不具合の解消 | A1〜A9 |
| R16 | テスト | `core::ui` / `core::setting` を GPU・ウィンドウ無しで |

### 2.2 見送り（後続）

アニメーション、画像／9-slice、パッドのボタンアイコン、XInput（現状 winmm）、テキスト入力欄、ハードウェアシザー（今回はソフトクリップ）、MSDF グリフキャッシュの O(1) 化、Alt+Enter の抑止（`MakeWindowAssociation`）、排他フルスクリーン中の `ResizeTarget` 対応、キー待ち中のグローバルホットキー抑止、`GetAsyncKeyState` が非アクティブ時も入力を拾う既存問題。

---

## 3. 設計

### 3.1 方針

- **保持型ウィジェットツリー**（状態の多いオプション画面に素直）。imgui はデバッグ専用のまま。
- **`core::ui` はプラットフォーム・D3D 非依存**。依存は `core/type.h`・`core/graphics/TextStyle.h`・`core/ui/*`・標準ライブラリのみ。テストから全機能を叩ける。
- **UI 入力はゲーム入力と分離**。ウィジェットは毎フレーム `UIInput`（値型スナップショット）を受け取る。**UI のキーは固定の生キー**（矢印・Enter・Esc・Q/E・PageUp/Down）とパッドの生ボタンで、ゲームのキー割り当てとは無関係（割り当てを変えてもメニュー操作は壊れない）。
- **設定は UI から独立したモデル**（`core::setting::SettingStore`）。部品は値を持たず、getter/setter で結びつく。
- **再入の禁止**: コールバック内で層やツリーを直接書き換えない。`Screen::post()` に積み、`update()` の最後にまとめて実行する。

### 3.2 レイヤー

```
blue_sky
  Scene/OptionsScene, PauseMenuScene     … 画面の中身
  blue_sky/ui/UIOverlayScene             … Screen を持つオーバーレイシーン基底
  blue_sky/ui/UIInputBuilder             … Input の生状態 → UIInput（キーリピート・デバイス判定）
  blue_sky/ui/Renderer                   … 描画・計測・ソフトクリップ・等方スケール
  blue_sky/GameSettings                  … 設定項目の登録と適用先への配線（既定値の唯一の定義場所）
  blue_sky/KeyNames                      … キー名 ⇔ 仮想キーコード
  blue_sky/Input                         … 生キー／生パッド API、キー割り当て API を追加
core
  core/ui/       Screen, Widget, Stack, ScrollView, TabView, 部品, Dialog, Theme, StringTable, NavRepeater, Rect, UIInput
  core/setting/  SettingStore
extern/game      game::Config
```

### 3.3 `UIInput`

```cpp
enum class InputDevice { KEYBOARD_MOUSE, GAMEPAD };

struct UIInput {
    bool up, down, left, right;      // キーリピート適用済み
    bool confirm;                    // Enter / パッド A（マウスは含まない）
    bool cancel;                     // Esc / パッド B
    bool tab_prev, tab_next;         // Q,PageUp / E,PageDown / LB / RB
    float pointer_x, pointer_y;      // 仮想座標
    bool pointer_moved, pointer_pressed, pointer_down, pointer_released;   // 左ボタン
    int  wheel;                      // + 奥 / - 手前（ノッチ数）
    int  pushed_key;                 // このフレームに押された「割り当て可能キー」（0 = なし）
    InputDevice last_device;
};
```

- 生成は `blue_sky::ui::UIInputBuilder`。`Input` に追加する生状態 API（全 256 仮想キーの今／前フレーム状態を毎フレーム更新、パッドの生ボタン・方向）だけを使い、`Button::A` 等は使わない（A1 の根本対策）。
- **オーバーレイ生成直後のフレームは入力を捨てる**（`Screen::suppress_input( 1 )`）。ESC でポーズを開いた同じ ESC でポーズが閉じるのを防ぐ（レビュー致命 1）。
- `pushed_key` は前フレーム差分なので、キー待ちを始めた Enter は「押しっぱなし」で拾われない。さらに `Screen` はキー待ちを始めたフレームの `pushed_key` を無視する。

### 3.4 `Renderer`

```cpp
// 既存に追加
virtual float_t measure_text_width( const char_t* text, float_t font_size ) const = 0;   // 仮想座標
virtual float_t measure_line_height( float_t font_size ) const = 0;
virtual void push_clip( const Rect& ) = 0;   // 入れ子は交差
virtual void pop_clip() = 0;
```

- **ソフトクリップ**（レビュー高 4）: エフェクトのパスが `RASTERIZERSTATE = Default` を毎回適用するため、外から設定したシザーは効かない。今回は ① 矩形は交差計算で正確に切る ② テキストは描画矩形がクリップに完全に収まるときだけ描く。ScrollView はスクロール位置を**行の境界にスナップ**するので、テキストの欠けは起きない。
- **等方スケール**（A9）: `scale = min( 物理幅 / 1920, 物理高 / 1080 )`、余白は中央寄せ。テキスト・矩形・マウス逆変換すべて同じ変換。`font_size==0` は既定サイズ × scale。

### 3.5 ウィジェット

```cpp
class Widget {
    // ツリー（親が子を unique_ptr で所有）
    Widget* parent();  size_t child_count();  Widget* child_at( size_t );
    // 状態
    visible / enabled / focusable / focused
    // レイアウト（2 パス、Screen が dirty のときだけ実行）
    virtual float_t measure_height( float_t width, const UIContext& );
    virtual void arrange( const Rect&, const UIContext& );
    // 入力（フォーカス部品から。消費したら true）
    virtual bool on_nav( NavDir, UIContext& );        // 左右を値変更に使う部品は消費
    virtual bool on_confirm( UIContext& );
    virtual bool on_cancel( UIContext& );            // キー待ち中の中止など
    virtual bool on_pointer( const PointerEvent&, UIContext& );   // ヒットした部品へ
    virtual bool on_wheel( int, UIContext& );
    virtual bool on_key_captured( int, UIContext& );
    virtual void update( float_t dt, UIContext& );
    // 描画・ヒント
    virtual void render( Renderer&, const UIContext& ) const;
    virtual void collect_hints( std::vector< Hint >& ) const;
    // 説明文（説明欄に表示する文字列キー）
    description_key
};

struct UIContext { const Theme& theme; const StringTable& strings; const Renderer& renderer; Screen& screen; };
```

- `UIContext` に `Renderer`（計測用）を含める（レビュー中 18）。
- 生成は `parent->add< Slider >( ... )` で型付き非所有ポインタを返す。

### 3.6 コンテナ

| クラス | 役割 |
|---|---|
| `Stack` | 縦／横に子を並べる。横は等分割。`wrap`（端で反対端へ、既定 off） |
| `ScrollView` | 子 1 つ。スクロールは子 Stack の行境界にスナップ。`ensure_visible`、ホイール、描画時 `push_clip`。ナビゲーションは透過 |
| `TabView` | タブ見出し行（クリック可・フォーカス不可）＋ページ。子として見えるのは現在ページだけ（ナビ・ヒット・描画の対象外を自然に実現）。ページごとにフォーカスを記憶 |
| `Panel` | 背景＋タイトル＋中身＋説明欄＋ヒント行 |

### 3.7 フォーカスとナビゲーション（`Screen`）

`Screen` はルート層と、その上の**モーダル層スタック**を持つ。入力は最前面の層だけが受け取る。

`Screen::update( const UIInput&, float_t dt )` の順序:

1. 入力抑止フレームなら入力を捨てる（3.3）
2. **キー待ち中**: `cancel` → 部品の `on_cancel`（中止）、`pushed_key` → `on_key_captured`。他の入力は無視
3. **ポインタ**: `pointer_moved` → ヒットした部品にフォーカス（理由 = POINTER）。`pressed/down/released` → ヒットした部品へ（ドラッグ中は押した部品にキャプチャ）。**ヒット判定は祖先のクリップ矩形と交差を取る**（隠れた行に当たらない）
4. **ホイール**: カーソル下の祖先 `ScrollView`（無ければフォーカス部品の祖先）
5. **タブ**: フォーカス部品の祖先、無ければ層内最初の `TabView`
6. **方向**: フォーカス部品の `on_nav` → 消費されなければ**階層ナビゲーション**
7. **決定**: `on_confirm`
8. **キャンセル**: `on_cancel` → 消費されなければ層の処理（ダイアログなら cancel ボタン扱い、ルートなら「戻る」コールバック）
9. 各部品の `update( dt )`
10. **`post()` されたコマンドを実行**（層の push/pop、画面遷移、ツリー差し替え）
11. 必要ならレイアウト（dirty）

**階層ナビゲーション**（レビュー高 5）: 方向 d の移動は、フォーカス部品から祖先へ遡り、軸が d と一致する最初の `Stack` の中で「次の兄弟」を探す（兄弟がコンテナならその中の入口へ降りる）。見つからなければさらに上の祖先へ。ルートまで無ければ移動しない（`wrap` の Stack は自分の中で反対端へ）。

- 入口の選び方: 同じ軸のコンテナには端（下・右へなら先頭、上・左へなら末尾）から入る。直交軸のコンテナには、直前の位置に中心が最も近い子から入る。
- 画面外にスクロールされた行も「並び順の次」なので飛ばされない。

**フォーカス理由**（レビュー高 6）: NAV / POINTER / PROGRAM。`ensure_visible` は NAV と PROGRAM のときだけ。ホイールで内容が動いたら、次のポインタ移動までホバーを再評価しない。

### 3.8 部品

| クラス | 値（getter/setter） | 操作 |
|---|---|---|
| `Label` | 文字列キー or 直接文字列 | HEADER / NORMAL / SMALL、簡易折り返し（`\n` と幅による貪欲折り返し、コードポイント単位） |
| `Button` | — | 決定、**ヒットしたクリック**（押して離した位置も部品上のとき） |
| `Toggle` | bool | 決定・左右・クリックで反転 |
| `Slider` | float（min/max/step） | 左右（リピート）、トラックのクリック・ドラッグ（行の矩形内でのみ開始） |
| `Choice` | int（index、-1 = 選択肢に無い現在値） | 左右・矢印クリック。`wrap` 指定可。-1 のときは呼び出し側が与える表示文字列を出す |
| `KeyBind` | 主キーのコード | 決定 → キー待ち（次フレームから受付）→ キー入力で setter。Esc で中止 |

ラベル付き行は共通基底 `LabeledRow`（左にラベル、右に値エリア。比率は Theme）。
部品は値を**持たない**（レビュー高 8）。描画のたびに getter を読むので、ストアの revert・既定値・入れ替え・外部変更が自動で画面に出る。

### 3.9 ダイアログ

```cpp
screen.show_dialog( DialogSpec {
    message,                                  // 表示文字列（{0} 置換は呼び出し側）
    buttons = { { "common.keep", on_keep }, { "common.revert", on_revert } },
    cancel_index = 1,                         // Esc / B のときに押したことにするボタン
    timeout_sec = 15.f,                       // 0 = なし。時間切れで cancel_index
    message_provider                          // 毎フレーム文字列を作る（残り秒数表示用、任意）
} );
```

- ボタンが押されたら、**まずダイアログを閉じるコマンド、次にコールバックを `post()`** する。コールバックから次のダイアログを開いても正しく積まれる（レビュー致命 2）。
- コールバックが捕まえるのは `this`（画面オブジェクト）か値。ローカル参照は禁止。

### 3.10 テーマ・文字列

- `Theme`: 色（背景暗幕・パネル・行・行フォーカス・文字・淡色文字・無効文字・アクセント・見出し）、フォントサイズ（タイトル・通常・小）、行高・余白・間隔・ラベル比率。
- `StringTable`: `key = 値`（`#` コメント、BOM 可、`\n` エスケープ、`{0}` 置換）。※ `game::Config` の `key 値` 形式とは意図的に別（値に空白や `=` を含めたい）。
- ファイル: コード上のパス `media/text/ja.txt`、実体 `source/media/text/ja.txt`（既存の `media/font/...` と同じく実行時カレント `source/` で解決）。

### 3.11 操作ヒント

部品が `{ HintAction, 文字列キー }` を返し、`Screen` が層共通（戻る・タブ）を足して、`last_device` に応じた操作名（`hint.key.confirm` = `Enter` / `hint.pad.confirm` = `A` など、文字列テーブル）で 1 行に整形する。

### 3.12 設定モデル `core::setting::SettingStore`

```cpp
enum class ApplyMode { IMMEDIATE, DEFERRED };
using Value = std::variant< bool, int, float, string_t >;

class SettingStore {
    // 登録
    void add_bool  ( key, bool def, ApplyMode, ApplyFn = {} );
    void add_int   ( key, int def, int min, int max, ApplyMode, ApplyFn = {} );
    void add_float ( key, float def, float min, float max, float step, ApplyMode, ApplyFn = {} );  // step に量子化して保存
    void add_string( key, string_t def, ApplyMode, ApplyFn = {} );
    void add_apply_group( std::vector< string_t > keys, std::function< void() > fn ); // DEFERRED の複数キーをまとめて 1 回
    void set_on_set( key, std::function< void( SettingStore&, const Value& old_value ) > hook );  // 制約（キー入れ替え等）

    // 編集
    T get< T >( key ) const;                 // 保留値
    void set( key, Value );                  // クランプ・量子化。IMMEDIATE は ApplyFn を即呼ぶ。on_set フック（フック内の set はフックを呼ばない）
    bool has_pending_changes() const;        // 全種（float は ε 比較）
    bool has_pending_deferred() const;       // DEFERRED の未適用

    void commit_immediate();                 // IMMEDIATE の保留を確定に（戻るときの自動確定）
    bool apply_deferred();                   // DEFERRED の保留を確定、グループ／個別 ApplyFn。変化があれば true
    void revert();                           // 保留を確定値へ。IMMEDIATE で変わっていたものは ApplyFn を呼んで戻す
    void reset_to_defaults();                // 保留を既定値へ（IMMEDIATE は即反映）

    Snapshot snapshot() const;               // 確定値の写し
    void rollback( const Snapshot&, const std::vector< string_t >& keys );   // 確定・保留を戻し、変化した分の ApplyFn／グループを呼ぶ

    void sync_from_runtime( key, Value );    // 外部変更（F2/F5）を確定・保留の両方へ取り込む。ApplyFn は呼ばない

    void load( const game::Config& );        // 壊れた値・範囲外は既定値／クランプ
    void save( game::Config& ) const;        // 確定値を書く
    void apply_all_individual();             // 起動時: 個別 ApplyFn だけ呼ぶ（表示系グループは App 初期化済みなので呼ばない）
};
```

- **ApplyFn の捕捉対象の寿命**（レビュー中 15）: ストアは `GameMain` が所有。ApplyFn はシーンを捕まえない。FOV は「適用」で config 値を更新し、`GamePlayScene` は毎フレームではなく **設定変更通知**（`GameMain::get_settings_revision()` が増えたら読み直す）で反映する。
- **表示系の適用はフレーム末に予約**（レビュー中 14）: グループ ApplyFn は `GameMain::request_display_change()` を呼ぶだけ。実際の `set_size` / フルスクリーン切替は `GameMain::update` の最後に行う（UI コールバック中に WM_SIZE → リサイズが走らない）。

**オプション画面の流れ**:

| 操作 | 処理 |
|---|---|
| 値を変える | `set`（IMMEDIATE は即反映） |
| 適用（DEFERRED 変更があるときだけ有効） | `snap = snapshot()` → `apply_deferred()` → `commit_immediate()` → `save` → 表示系が変わったら確認ダイアログ（15 秒）。維持 = 何もしない／戻す・時間切れ = `rollback( snap, 表示系キー )` → `save` |
| 戻る（Esc / ボタン） | DEFERRED の未適用があれば「適用して戻る／破棄して戻る／キャンセル」。破棄 = DEFERRED だけ `revert`。いずれも IMMEDIATE は `commit_immediate()` → `save` → 閉じる（レビュー中 17：音量だけの変更で確認を出さない） |
| 既定値に戻す | 確認 → `reset_to_defaults()` |

**キー割り当て**（レビュー高 9）: 値は文字列（`input.key.left = "A LEFT"` の既存形式）。主キー = 先頭トークン。`set_on_set` フックで「他ボタンの主キーと同じなら、そちらの主キーを旧キーに入れ替え」を **set の時点**で行う。IMMEDIATE で `Input::set_key_bindings` に反映。

**解像度**（A7）: 選択肢 = DXGI モード一覧（重複除去）。現在値が一覧に無ければ「カスタム（W x H）」を先頭に足す。index ではなく `graphics.screen_width/height` の 2 キーに書く。

**外部経路**（A8）: F2/F5 は `sync_from_runtime`。`~GameMain` の個別保存はストアの `save` に置換。

### 3.13 シーン統合

- オーバーレイが `set_next_scene( "push:options" )` → 自分を残して上に積む（R14）。`"pop"`・名前だけ（置換）は従来通り。
- オーバーレイがあるときは `GameMain` は ESC を処理しない（UI の cancel）。
- `UIOverlayScene`: `Renderer`・`UIInputBuilder`・`Screen` を持つ。生成直後 1 フレームの入力抑止。`Screen` のサウンドイベント（移動・決定・キャンセル・値変更）を `play_sound` に変換。

### 3.14 `Input` の追加 API

- 生キー: `is_key_down( vk )` / `is_key_pushed( vk )`（全 256 を毎フレーム更新、ウィンドウ非アクティブ時は全 false）
- 生パッド: `is_pad_button_down( i )` / `is_pad_button_pushed( i )`、`is_pad_dir_down( dir )`
- `get_pushed_bindable_key()`: 割り当て可能キーのうち今フレーム押された最初のもの
- キー割り当て: `get/set_key_bindings( Button )`、`get_key_bindings_text` / `parse_key_bindings_text`、ユーザー割り当てと固定キー（矢印・Enter・Esc）を分離
- 割り当て不可: ESC（キャンセル予約）、F1/F2/F5（グローバルホットキー）、汎用 SHIFT/CTRL/ALT（左右別コードで割り当て）

### 3.15 移行

`WidgetContainer` → `Screen`+`Stack`、`VerticalMenu` → `Stack`+`Button`、`SelectBox` → `Choice`、旧 `Widget::update( InputManager*, Renderer& )` は廃止。使用箇所は `OptionsScene` / `PauseMenuScene` のみ（確認済み）。

---

## 4. テスト計画（`blue-sky-test`、GPU 不要）

`MockRenderer`（描画呼び出しを記録。計測は **UTF-8 コードポイント数** × font_size × 0.5）と手組みの `UIInput` で検証。新規テストは `blue-sky-test.vcxproj` と `.filters` に登録する。

| 対象 | ケース |
|---|---|
| NavRepeater / StringTable / KeyNames | （実装済み） |
| 階層ナビ | 縦リスト上下・端で止まる・wrap、リスト末尾→下でボタン行、ボタン行の左右、ボタン行→上でリストへ、無効／非表示をスキップ、スクロール外の行も順に辿る |
| ポインタ | ホバーでフォーカス、**ヒットしない位置のクリックで Button/Slider/Choice が反応しない（A1 回帰）**、Enter で Slider が飛ばない、ドラッグのキャプチャ、クリップ外の行に当たらない |
| Slider / Choice / Toggle | 値変更・クランプ・量子化・-1（選択肢外）表示・getter からの再表示 |
| KeyBind | キー待ち開始フレームの pushed_key を無視、確定、Esc で中止（画面は閉じない） |
| ScrollView | NAV で ensure_visible・POINTER では動かない・行境界スナップ・ホイール |
| TabView | 切替でページ入れ替え、フォーカス記憶、非表示ページはナビ・ヒット対象外 |
| Dialog | 背面に入力が届かない、cancel_index、タイムアウト、**コールバックから次のダイアログを開ける**（致命 2 回帰） |
| Screen | 入力抑止フレーム（致命 1 回帰）、キャンセル優先順、post の遅延実行、ヒント |
| SettingStore | クランプ・量子化・ε 比較、IMMEDIATE 即反映、revert、DEFERRED は apply まで呼ばれない、グループ 1 回、snapshot/rollback、on_set による入れ替え、sync_from_runtime、Config 入出力・壊れた値 |
| Label 折り返し | 幅超過で改行、`\n`、日本語 |

実機確認: ポーズ → オプション → 戻る、ESC でポーズを開いてすぐ閉じない、タブ、スクロール、解像度の確認ダイアログとロールバック、キー割り当て、16:10 での表示。

---

## 5. 実装段階（レビュー中 21 を反映）

| 段階 | 内容 | 完了条件 |
|---|---|---|
| A | `Input` 生 API・`UIInput`・`Screen`（層・post・入力抑止・階層ナビ・ポインタ）・`Stack`・`Label`/`Button`/`Toggle`/`Slider`/`Choice`・`Dialog`・`Theme`・`SettingStore`・`GameSettings`・`UIOverlayScene`、`OptionsScene`/`PauseMenuScene` 置換、旧 UI 削除 | A1〜A9 解消、テスト緑 |
| B | `ScrollView`（ソフトクリップ）・`TabView`・説明欄・Label 折り返し、オプション画面をタブ化 | テスト緑 |
| C | `KeyBind`・キー割り当てタブ・デバイス別ヒント | テスト緑、実機確認 |

---

## 5.1 第 3 版の追加規則（再レビュー 12 件の反映。3 章より優先する）

| # | 規則 |
|---|---|
| 再1 | **層・ツリーの変更は必ず `post()` 経由**（`show_dialog`・層の pop・画面遷移・ツリー差し替えは内部で post）。部品のコールバックは直接呼んでよい。ダイアログのボタンは「コールバックを**値でコピー**して post → 閉じる処理を post」の順で積む。キューは添字ループで処理し、**実行中に積まれたものも同じフレームで最後まで処理**する。時間切れ・キャンセルも同じ経路 |
| 再2 | 「適用して戻る」で表示系が変わったら、**確認ダイアログを先に出す**。維持 → 閉じる／戻す・時間切れ → `rollback` して**画面に残る** |
| 再3 | 「破棄して戻る」は `revert_deferred()`（実装済み）。IMMEDIATE は `commit_immediate()` |
| 再4 | FOV は IMMEDIATE。`GamePlayScene` は `SettingStore::get_change_count()` の変化を見て**ストアの保留値**を読む。`Camera` には「既定 FOV だけ更新し、ズーム倍率は保つ」API を足す |
| 再5 | 有効・無効は述語 `set_enabled_if( std::function< bool() > )`（固定値も可）。Screen は毎フレーム、フォーカス中の部品がフォーカス不可になっていたら**ツリー順で最も近いフォーカス可能な部品**へ移す（理由 PROGRAM） |
| 再6 | Stack は**最後にフォーカスした子**を記憶し、入口の最優先にする。入口候補がフォーカス不可・空なら並び順で次の候補へ再帰。wrap は「祖先へ抜けられなかったときだけ」 |
| 再7 | ScrollView は**完全に収まる子だけ描画・ヒット対象**にする（子単位の間引き）。ピクセルクリップは使わない |
| 再8 | 終了時の保存直前と毎フレーム、実際のフルスクリーン状態を `sync_from_runtime` で取り込む（Alt+Enter 対策） |
| 再9 | `UIOverlayScene::on_resume()`: 最前面に戻ったときも `UIInputBuilder` をリセットし 1 フレーム入力を抑止 |
| 再10 | Renderer に `fill_screen( Color )`（物理画面全体）。ポインタ座標はクランプしない（範囲外は何にも当たらない） |
| 再11 | （段階 C）キー割り当ては主キー 1 つだけを編集し、割り当て時に 2 番目以降のトークンは削除。ウィンドウが再アクティブになったフレームは「今押された」を出さない |
| 再12 | 段階 A に「Renderer（計測・等方スケール・`fill_screen`）」「Panel（タイトル・説明欄・ヒント行）」を含める |

## 6. レビュー指摘への対応表

| # | 重大度 | 指摘 | 対応 |
|---|---|---|---|
| 1 | 致命 | ESC でポーズを開いた同じ ESC で閉じる | 3.3 入力抑止フレーム／3.13 |
| 2 | 致命 | コールバック内の層変更・寿命 | 3.1 / 3.7 手順 10 の `post()`、3.9 |
| 3 | 致命 | クリックと決定が区別できない | 3.3・3.14 生 API、UI は固定生キー |
| 4 | 高 | シザーが効かない | 3.4 ソフトクリップ＋行スナップ |
| 5 | 高 | 空間ナビとスクロールの破綻 | 3.7 階層ナビ |
| 6 | 高 | ホバーと自動スクロールの衝突、隠れた行へのヒット | 3.7 フォーカス理由・クリップ考慮ヒット |
| 7 | 高 | ロールバック用の旧値が無い | 3.12 snapshot/rollback と流れ表 |
| 8 | 高 | 部品とストアの二重管理 | 3.8 部品は値を持たない |
| 9 | 高 | 入れ替えの置き場所・文字列値 | 3.12 `set_on_set`・`add_string` |
| 10 | 高 | キー待ちの前状態・除外キー | 3.3・3.14 |
| 11 | 高 | 解像度が勝手に書き換わる | A7・3.12 カスタム選択肢 |
| 12 | 高 | A1 の範囲が狭い | A1 を一般化、テスト拡大 |
| 13 | 高 | ストア外の変更経路 | A8・3.12 sync_from_runtime（Alt+Enter は 2.2 へ） |
| 14 | 中 | 解像度の適用方法・タイミング | 3.12 フレーム末に予約（ResizeTarget は 2.2 へ） |
| 15 | 中 | ストアの所有・ApplyFn の寿命 | 3.12 |
| 16 | 中 | 浮動小数の比較 | 3.12 量子化＋ε |
| 17 | 中 | 音量だけで破棄確認が出る | 3.12 流れ表 |
| 18 | 中 | 計測手段・レイアウト API | 3.5 UIContext に Renderer、dirty 時レイアウト |
| 19 | 中 | 縦横別スケール | A9・3.4 等方スケール |
| 20 | 中 | 説明文の折り返し | 3.8 Label 簡易折り返し |
| 21 | 中 | 範囲が大きすぎる | 5 章 段階 A/B/C |
| 22 | 中 | 計測の負荷 | dirty 時のみレイアウト。LRU 改善は 2.2 |
| 23 | 低 | 文字列ファイルの置き場所 | 3.10 |
| 24 | 低 | 命名規則・依存の記述 | `UIInput` 等に改名、3.1 依存を修正、Rect は float_t |
| 25 | 低 | 計測の数え方・vcxproj 登録 | 4 章 |

---

## 7. 実装状況（2026-11-08）

段階 A・B・C をすべて実装した。

| 区分 | ファイル |
|---|---|
| 設定 | `core/setting/SettingStore.*` |
| UI 基盤 | `core/ui/UIInput.h` `NavRepeater.h` `Rect.h` `Theme.h` `StringTable.*` `Renderer.h` `Widget.*` `Screen.*` |
| 部品 | `core/ui/Stack.*` `Label.*` `Button.*` `LabeledRow.*` `Toggle.*` `Slider.*` `Choice.*` `KeyBind.*` `ScrollView.*` `TabView.*` `Dialog.*` `Panel.*` |
| blue_sky | `blue_sky/ui/Renderer.*` `blue_sky/ui/UIInputBuilder.*` `blue_sky/KeyNames.*` `blue_sky/GameSettings.*` `blue_sky/Input.*`（生キー・割り当て API） `Scene/UIOverlayScene.*` `Scene/OptionsScene.*` `Scene/PauseMenuScene.*` |
| 文字列 | `source/media/text/ja.txt` |
| テスト | `test/ui/*`（75 件） |

### 設計からの変更点

- **ScrollView のクリップ**: 「行境界へのスナップ」ではなく、`Widget::get_view_clip()` による**子単位の間引き**（再レビュー 7 の方式）。範囲に完全に収まらない末端の部品は描画もヒット判定もしない。`Widget::render_clipped()` と `Screen::hit_test()` が担当。
- **Stack::set_flex()** を追加（タブの中身を残りの高さいっぱいに広げ、ボタン行を下端に置くため）。
- **Screen::focus_into()** を追加（タブ切替後に、そのページで最後にフォーカスしていた位置へ戻す）。
- **開発用設定 `debug.start_overlay`**: 起動時にオーバーレイ（`pause_menu` / `options`）を開く。ベースシーンを 10 フレーム更新してから開く。
- キー割り当ては**主キー 1 つだけ**を編集する（割り当て時に 2 番目以降のキーは削除）。F1 / F2 / F5 も割り当て不可（グローバルなホットキー）。

### 実機確認（enemy_ai_test 上で、キー入力を自動送信して撮影）

| 確認項目 | 結果 |
|---|---|
| ポーズ画面の表示（日本語・フォーカス・操作ヒント） | OK |
| ポーズ → オプション → ESC でポーズに戻る（A2）／もう一度 ESC で閉じる（アプリは終了しない） | OK |
| タブ切替（E）・操作タブで下へ進むと自動スクロール・はみ出した行は描かれない | OK |
| キー割り当て: 前進に D → 右へ移動（D）と入れ替わり W に | OK |
| 解像度を変えて適用 → ウィンドウが変わり確認ダイアログ（残り秒数）→ 15 秒で元に戻る → 保存も元の値 | OK |
| 適用ボタンが無効になったらフォーカスが隣へ移る | OK |
| 16:9 以外（1280x768）で UI が歪まない（等方スケール・中央寄せ） | OK |
| パッド操作・フルスクリーン切替 | 未確認 |

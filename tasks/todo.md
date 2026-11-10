# TODO - 未着手タスクと進捗

## 運用ルール

1. タスクを追加するときはチェックボックス形式で書く
2. 完了したら `[x]` にする
3. セクションが全て完了したら、セクションごと削除してよい

## UI ライブラリ（オプション画面の基盤）  ブランチ: feature/ui-library（develop から）

設計書: `doc/UIライブラリ設計.md`（第 3 版。サブエージェントの厳格レビュー 2 回を反映）

- [x] 現状調査・問題点の洗い出し（A1〜A9 / B1〜B11）・要件・設計
- [x] 設計レビュー 2 回（指摘 25 件＋再レビュー 12 件）を設計書に反映
- [x] core::setting::SettingStore（保留／確定／既定、IMMEDIATE / DEFERRED、グループ反映、snapshot/rollback、入れ替えフック、Config 入出力）
- [x] core::ui 基盤: UIInput / NavRepeater / Theme / StringTable / Renderer（計測・fill_screen）/ Widget / Screen（層・post・入力抑止・階層ナビ・クリップを考慮したヒット判定・フォーカス修復・ヒント）
- [x] 部品: Stack / Label（折り返し）/ Button / Toggle / Slider / Choice / KeyBind / ScrollView / TabView / Dialog / Panel
- [x] blue_sky 側: Renderer（等方スケール）/ UIInputBuilder / Input の生キー API・キー割り当て API / KeyNames / GameSettings / UIOverlayScene
- [x] OptionsScene（3 タブ・適用／既定値／戻る・解像度の確認とロールバック）/ PauseMenuScene 置換、旧 VerticalMenu / WidgetContainer / SelectBox 削除
- [x] GameMain: オーバーレイを積む（push:）・ESC を UI に任せる・表示変更をフレーム末に・F2/F5/Alt+Enter の同期・終了時の保存をストア経由に
- [x] テスト 75 件追加（62 → 137 件。136 成功、失敗 1 件は作業前から失敗していた NeuralNprTest.DmlInferenceSmoke）
- [x] 実機確認（2026-11-08）: ポーズ→オプション→戻るでポーズへ／タブ／スクロール／キー割り当ての入れ替え／解像度の適用→15 秒でロールバック
- [x] 16:10（1680x1050）・ウルトラワイド（2560x1080）での表示 : BLUE_SKY_SETTINGS_DIR で解像度を変えて起動し、オプション画面が画面に収まり正しく並ぶことを描画結果で確かめた（2026-11-10）
- [x] 目標地点への移動（壁・ゴールが動くイベント）: 0-1・1-1・1-2・2-0・2-1 で着いて止まることを確かめた（5ebff4eb の BLUE_SKY_DEBUG_PRESS_SWITCHES）
- [x] キー割り当ての前進と右が入れ替わっていた : コードの既定（前進 W・右 D）は正しく、以前のキー入れ替えの確認の結果が古い設定ファイル（source/settings.config）に残り、%LOCALAPPDATA% へ引き継がれていた。2 行を消して既定に戻した（2026-11-10）
- [x] フルスクリーンをボーダーレス（画面いっぱいの縁なしウィンドウ）にした（2026-11-10）: 排他フルスクリーンは 4K モニター 3 台の開発機で、切り替え 1 回に 17〜50 秒かかり、抜けるときに止まって終了できないことがあった。ボーダーレスでは切り替えは一瞬、フルスクリーンのまま 1280x720 → 1600x900 に変えられ、Alt+Enter の往復・終了も確かめた
- [x] 実機（機器）が要る確認 : パッド操作は設計整備の対象外とした（2026-11-10、利用者の判断。開発機にパッドが無い）。パッドを使う作業（XInput など）を始めるときに、つないで確かめる
- [x] 設計書 2.2 の後続のうち対応したもの : Alt+Enter の抑止（MakeWindowAssociation、86ed6344）、MSDF グリフキャッシュの O(1) 化（f404201f）、キー待ち中のホットキー抑止（34541b28）
- [ ] 機能の追加（設計上の問題ではない後続）: XInput、パッドのボタンアイコン、ハードウェアシザー、HUD への適用
- [x] タイトルシーンが起動直後に落ちる不具合 : 67dca1c5 で直した。全 15 シーンを起動して落ちないことを 2026-11-10 に確かめた

### 設計上の問題の洗い出し（2026-11-10）

一覧: `doc/設計上の問題の洗い出し.md`（UI ライブラリ・描画基盤・アプリへの組み込みの 3 範囲、60 件）

対応状況は一覧の冒頭の「対応状況」節が正（番号とコミット）。

- [x] A（今すぐ直す不具合 11 件）: すべて対応。ただし A9 はデバイスロストを知らせて終了するまで（復旧は B20 と一緒に）
- [x] B（構造の問題 28 件）: 困りごとはすべて対応、A9 の復旧も（B23 の非同期の読み込みは後続。下の見直しを参照）。B19 は d686edf3（抽象層に D3D11 の型なし）と 1bba4f84（Effects11 をやめて自前で読む）。B3 の 2 次元の計測は 4cad6f57、B27 の Raw Input は a0a074bf・9f2521ec。B28 は 7a418118・12888db6・60d9b151・be50ed76・87abd1c4 で core::Service・Direct3D11::get_instance・すべてのシングルトンを削除
- [x] C（品質の改善）: すべて対応（c3fbbd9d・a259d986・dcb91898・8f1df39f・aeac9c46・5331914b・d5b11a22・89074b8d・797a1683）。項目ごとの根拠は一覧の「3 章の各項目と、対応した根拠」
- [x] 一覧の外で見つけた不具合 : ECS のシーンの 3D が真っ黒（95aee6ab）、canvas_test が落ちる（87abd1c4）
- [x] 39 件の見直し（2026-11-10 14:35）: 「一部だけ」だったものを直した。B11 クリック判定を ClickTracker に（a777beb0）、B26 設定ファイルの 2 つ目以降のキーも割り当てられるか判定（64a0fd37）、A9 ResizeBuffers のデバイスロストも復旧の経路へ（5dd8b997）、A3 設定ファイルの読めない行・値をファイル名と行番号付きでログに（57d2af07）、A5 シェーダーの監視を開発ビルドだけに（71899d62）、B20 create_named の説明の誤り（228d76e3）、B7 遷移の要求に引数（"push:options?tab=controls"、518344cb）。テストは 332 件が通る
- [ ] B23 の残り（後続）: 非同期の読み込みは仕組み（AsyncLoader・start_texture_load_async）だけで、まだ使っていない（3bdcf18d で説明に明記）。シーンの読み込みを別スレッドにするときに使う
- 入れないと決めた直し方の案（理由は一覧の対応状況の表）: B4 の TextKey 型・代わりの言語、B5 の on_value_changed、B9 の LabeledRow の継承をやめる・タブの見出しを部品に
- テストは 284 件が通る。全 15 シーンを起動して落ちない。Effects11 の置き換えの前後を全 15 シーンで画素ごとに比べて違いなし（2026-11-10）
- [x] Raw Input で OS の入力が届くこと : 開発用の BLUE_SKY_RAW_INPUT_SINK（2a2dc0ec）で背面でも受け取るようにし、SendInput のマウスの移動と F24 のキーが WM_INPUT → ゲームまで届くことをログで確かめた（2026-11-10。画面のロック中のため前面での確認の代わり）
- [x] 画面で確かめる: 文字の色（9ed89881）、文字の大きさ 160%（b9f40890）、横長画面（76fda1fc）。描画結果の PNG 保存（BLUE_SKY_CAPTURE_FRAMES）で、画面がロック中でも確認した
- [x] アウトライン付きの文字の端の 1 画素幅の縦線 : MSAA で UV が外挿されていた。centroid で直した（1945a242）
- 確認の手順（画面がロック中でも使える）: 環境変数 BLUE_SKY_START_SCENE / BLUE_SKY_START_OVERLAY / BLUE_SKY_CAPTURE_FRAMES / BLUE_SKY_CAPTURE_DIR を付けて起動すると、指定フレームの描画結果が PNG になる。落ちたら log/crash.log に呼び出し履歴

### 画像による装飾（2026-11-10）

記録: `doc/UI画像対応と既存コードの修正.md`（何がまずくてどう直したか。レビュー指摘 19 件中 15 件を対応）

- [x] Brush（単色／画像・9 分割・切り出し・切り取り・代用）、Image 部品、テーマの Brush 化と用途別の面、テーマファイル（専用パーサ）、先読み
- [x] UI 用のテクスチャ読み込み（乗算済みアルファ・ミップなし・UNORM）と sprite_premultiplied technique
- [x] 既存コードの修正: is_transparent を GameMain が無視していた／Sprite の整数の切り出し・null・重複・既定引数／sprite.hlsl の文字化け／矩形の丸め／9 分割の角の太さ
- [x] 9 分割の辺・中央を並べて埋める（`edge` / `center` = stretch / repeat / round）。模様のある枠が引き伸ばしで崩れないように
- [x] サンプルテーマ（`media/ui/sample_theme.txt`、設定 `ui.theme` で有効）と生成スクリプト `tools/ui_sample/make_ui_sample.py`。テスト 53 件追加（190 件中 189 成功）
- [x] 再レビュー（12 件）を反映: Sprite の Map 前ガード、オーバーレイの描画範囲を純粋関数化（隠れた不透明も考慮・ベース非描画時のクリア）、丸め規則の共有（PixelSnap.h）、切り取りの物理ピクセル揃え、中央を描かない（center = none）ほか
- [x] 後続のうち対応したもの : Sprite のバッチ化（c1f1d86c）、画像のハンドル化・寿命管理（ResourceManager のハンドル、979b2c32）
- [ ] 機能の追加（設計上の問題ではない後続）: シザーによるクリップ、部品別スタイル、@2x 素材
- [x] 調査: enemy_ai_test の操作タブのキー欄の上の細い縦線 : 操作タブを描画結果で撮り、画素で探して出ていないことを確かめた（2026-11-10）。MSAA で文字の UV が外挿されていた件（1945a242）と同じ原因だった

## 敵AI（システミック）縦スライス  ブランチ: feature/enemy-ai-perception

設計は `doc/システム階層スタック.md` / `doc/動詞とゲームシステム.md`。S2 知覚 + S3 警戒状態機械 + パトロールの最小実装。

- [x] 判定の純関数化＋単体テスト（`blue_sky/ai/EnemyPerception.h`：視界錐内外・awareness・状態遷移、`test/ai/EnemyPerceptionTest` 12ケース）
- [x] Component（Perceivable/Perception/Alert/Patrol）＋ System（TargetRegistry→Perception→Alert→Patrol）を ECS 実装
- [x] 検証シーン `EnemyAiTestScene`（scene 名 `enemy_ai_test`）＋視界錐/経路/最終視認の Bullet デバッグ可視化。起動は `blue-sky.config` の `debug.start_scene` を `enemy_ai_test` に（ローカル）
- [x] **ECS 自動登録バグ修正**（`ComponentTypeId` 32bit 切り捨てで x64 の on_add_component 経路が全滅していた）。回帰テスト `test/ecs/EnemyAiSceneEcsTest`。詳細は lessons.md
- [ ] 実機で挙動確認・パラメータ調整（移動速度/視界角/awareness の増減/捜索時間）
- [ ] navmesh 統合（Recast & Detour）でパトロール/追跡を経路化（現状は平面直線）
- [ ] 聴覚（音イベント）・対空タイプの例外知覚
- [ ] S4 プレイヤー動詞（隠れる/逃げる/隙を突く/道具）との接続、死体隠し・警報伝播

## blue-sky 64bit 化（x64 ポート）— NPR リアルタイム統合の前提

ONNX Runtime + DirectML（NPR を実機リアルタイム実行する経路）は **x64 公式バイナリのみ**。
だが現状エンジンは **Win32(x86) 専用**（sln で x64→Win32 にされ、依存 lib も x86 のみ）。
よって「64bit 化 → その上に ORT+DirectML」の順。作業ブランチ `feature/x64-port`。

方針: **VR(Oculus) は一旦外す**（コンパイルスイッチで無効化。ベンダー依存を1つ減らす）。

### 依存ライブラリの x64 化（source/lib/x64 へ配置）
- [x] Bullet・libogg / libvorbis / libvorbisfile・PortAudio・boost（1.90）・FBX SDK（libfbxsdk / zlib / libxml2）・Recast / Detour・ONNX Runtime : source/lib/x64/debug にある（2026-11-10 に確認。x64 Debug でビルドと全シーンの起動ができている）
- [x] imgui・libremidi / msdfgen / freetype : ソースから x64 でビルドしている
- [x] Effects11 : 不要になった（1bba4f84 で自前の実装に置き換えた）
- [x] lua — x64 版が既存（`lib/x64/common/lua.lib`）
- [~] Oculus LibOVR — VR を外すので不要化

### エンジン側
- [x] x64 ビルド構成の調査: blue-sky/exe/test は既に x64→x64 マッピング済み。common/game/win は vcxproj に x64 構成あり（ExternalIncludePath も4構成OK）。freetype/msdfgen は x64 構成なし
- [x] sln マッピング修正: common/game/win を x64→x64 + Build.0 に（Python で該当3 GUID のみ書換）
- [x] common / game / win を x64 で個別ビルド成功（SolutionDir 明示。単体ビルドは SolutionDir 未定義で ExternalIncludePath が壊れる点に注意）
- [x] **blue-sky 本体ソースの x64 コンパイルがクリーン（0 error、164MB の blue-sky.lib 生成）**＝コードは概ね 64bit クリーン（uint_t typedef のおかげ）。BuildProjectReferences=false で検証
- [x] freetype / msdfgen の vcxproj に x64 構成を追加（Win32 ブロックを複製）→ sln マッピングも x64 に。x64 ビルド成功（freetype.lib / msdfgen.lib）
- [x] 全プロジェクト(common/game/win/blue-sky/exe/test)の x64 ItemDefinitionGroup を Win32 と同期（自動生成された x64 IDG が空/最小で RuntimeLibrary 未設定→ /MD 既定になり LNK2038 不一致。Win32 の中身をコピーして解消）
- [x] x64 リンク試行で punch-list 取得: **コンパイルは全proj 0エラー**。リンクは LNK2038=0、残るは LNK1104(lib が開けない)のみ＝**third-party x64 lib 不足だけが課題**。リンカは最初の不足libで止まるので芋づる式に出る（まず LibOVR / libxml2-mt）
- [x] Oculus(VR) を一旦無効化: `OculusRift.cpp` を `#ifdef OCULUS_RIFT_ENABLED` で囲み未定義時はスタブ（ovr_呼び出し無し）/ `lib.cpp` の LibOVR pragma 無効化。結果 x64 で 0コンパイルエラー・未解決0・LibOVR参照0
- [x] x64 ライブラリ検索パスは設定済みと判明: `lib\$(PlatformShortName)\$(Configuration)` = x64では `lib\x64\Debug`（vcxproj修正不要）。**`source/lib/x64/Debug` と `Release` に x64 lib を置くだけ**
- [x] exe/test の x64 PropertyGroup に **LibraryPath が無かった**のを追加（VC++ Directories は IDG 同期対象外だった。これが「lib\x64\Debug を検索せず全 third-party lib が開けない」真因）。lib 検索パスは `lib\$(PlatformShortName)\$(Configuration)` = `lib\x64\Debug`
- [x] test/lib.cpp にも LibOVR pragma があったので無効化（source/lib.cpp と別ファイル。見落としていた）
- [x] FBX SDK 2020.3.9(VS2022) インストール → x64 lib(`libfbxsdk-mt`/`libxml2-mt`/`zlib-mt`)+`libfbxsdk.dll` を `source/lib/x64/{debug,release}` にコピー。**FBX リンク解決**（注: 2020.3.9 は x86 lib 非同梱。ヘッダは 2020.2 のままだが x64 リンクは通った。lib バイナリは gitignore 済）
- [x] OSS lib の x64 ビルド＆配置（VS同梱 cmake、静的CRT /MTd・/MT、生成名をlib.cpp期待名へリネーム。cmake新しすぎ対策で `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` 必須）:
  - [x] libogg 1.3.5 → `libogg.lib`
  - [x] libvorbis 1.3.7 → `libvorbis_static.lib` / `libvorbisfile_static.lib`（OGG_INCLUDE_DIR/OGG_LIBRARY 指定）
  - [x] Bullet 3.25（USE_MSVC_RUNTIME_LIBRARY_DLL=OFF）→ `BulletCollision/Dynamics/LinearMath_vs2010_debug.lib`（Release は `_vs2010`）
  - [x] PortAudio v19.7.0（PA_DLL_LINK_WITH_STATIC_RUNTIME=ON）→ `portaudio_static.lib`
  - 作業ディレクトリ: scratchpad/osslibs
- [x] **boost を 1.90 へ更新**（ユーザー判断 B）の lib ビルド完了: D:\osslibs\boost_1_90_0 ソースから **自作 CMakeLists**（D:\osslibs\boostbuild、署名済み cmake/MSBuild）で filesystem/chrono/timer/regex/atomic を x64 静的(/MTd・/MT)ビルド。生成: D:\osslibs\boostbuild\build\{Debug,Release}\boost_*.lib
  - **重要な環境制約**: b2.exe は自前ビルド未署名 exe → **Device Guard がブロック**して実行不可。boost 公式 .7z ソースは CMakeLists 非同梱、prebuilt .exe は 7z 展開不可。→ 自作 CMake が唯一通った道（cl/link は署名済みで実行可）
  - filesystem は `std::atomic_ref` 使用のため **C++20 必須**（CMAKE_CXX_STANDARD 20）
  - boost 1.90 では system / regex は概ねヘッダオンリー（regex src は posix ラッパのみ）
- [x] **boost ヘッダ 1.75→1.90 差し替え**（`source/lib/include/boost`、git追跡 約15,000ファイルの巨大diff）→ blue-sky 再コンパイルで API 差異を確認・修正。**差異は軽微**:
  - `App.h`: `<functional>`/`<unordered_map>` を直接 include（boost 1.75 の推移的 include に依存していた箇所）
  - `FbxFileLoader.cpp` / `FbxConverterMain.cpp`: 廃止された `boost/filesystem/convenience.hpp` → `boost/filesystem.hpp`
  - 自動リンク名 = `libboost_<lib>-vc143-mt-sgd-x64-1_90.lib`（VS18→vc143 タグ）。ビルドした boost lib をこの名にリネーム配置
- [x] Effects11(FX11) を x64 ビルド → `effects11d.lib`/`effects11.lib`
- [x] portaudio を `PA_USE_WDMKS=OFF` で再ビルド（KSDATAFORMAT_SUBTYPE_MIDI 重複定義 LNK2005 回避）
- [x] **CRT 不一致の解消**: 既存 x64 `lua.lib`(common) が /MD でビルドされ `__imp_` CRT 参照→未解決。**Lua 5.4.3**(ヘッダが5.4.3。CLAUDE.mdの5.3.4は古い)を静的CRT(/MTd・/MT)で再ビルドし `lib/x64/{debug,release}` に配置。ogg/vorbis も CMP0091=NEW で /MT 再ビルド
- [x] **x64 で blue-sky.lib / exe / test がビルド成功（0エラー）**。`blue-sky-exe.exe` は x64(AMD64) 33MB。実行時用に `libfbxsdk.dll`(x64) を `.build/x64/Debug` へ配置
- [x] **boost ヘッダを git 管理外に変更**（一貫性回復＋肥大化解消）: 従来 boost ヘッダ(約14,694ファイル)を追跡していたが、lib バイナリは非追跡で「ヘッダだけ追跡しても clone でビルド不可」＝非対称だった。`.gitignore` に `/source/lib/include/boost/` を追加し `git rm -r --cached` で追跡解除（ファイルは残置）。再現手順を **`tools/x64-deps/`（README + build_oss_libs.sh）** に整備
- [x] **x64 exe 起動確認（実機・成功）**。Device Guard 通過、起動OK。
  - 起動時に PortAudio エラー（`Pa_OpenStream`：Incompatible host API specific stream info）が出たが、これは x64 化の副作用ではなく `SoundEngine.cpp` の WASAPI コードの潜在バグ（`Pa_GetDefaultOutputDevice()`＝既定ホストAPI(MME/DS)のデバイスに WASAPI 用ストリーム情報を渡していた）。**WASAPI ホストAPI の既定出力デバイスを明示取得**するよう修正して解決（要コミット、SoundEngine.cpp はユーザーWIP）。
## ② NPR リアルタイム統合（ONNX Runtime + DirectML）  ブランチ: feature/neural-npr-engine-integration

- [x] Phase1: ONNX Runtime DirectML 1.24.4 + DirectML 1.15.4 を NuGet から取得（展開のみ＝Device Guard 安全）。headers→`lib/include/onnxruntime`(非追跡), `onnxruntime.lib`→`lib/x64`, dll(onnxruntime/providers_shared/DirectML)→`lib/x64`＋exe出力先。手順は `tools/x64-deps/README ③`
- [x] Phase2: ORT+DirectML スモークテスト（`source/test/graphics/NeuralNprTest.cpp`）成功。**starry_boil_small.onnx を DirectML でロード・推論できることを実機確認。256x256 で 2.89ms/frame=346fps**（CPU往復込み）。`test/lib.cpp` に onnxruntime.lib pragma 追加
- [~] Phase3: 推論ラッパクラス化 → ポストエフェクト経路に差し込み。
  - [x] `core::graphics::NeuralStylizer` 実装＆コミット済（53a7b15e）
  - [x] 段階1: `DebugScene` に NeuralStylizer メンバ＋モデルロード(失敗時安全に無効化)＋imgui トグル `neural_npr_enabled_`。モデルは `media/neural/starry_boil_small.onnx`(gitignore)。**実機でモデルロード成功確認**（トグル表示）。トグルはまだ動作なし（段階2で実装）
    - ⚠️ **教訓**: starry_boil_small.onnx は**外部データ形式**で重みが別ファイル `starry_boil_small.onnx.data`(6.7MB) にある。ORT は `.onnx` と同じディレクトリに `.onnx.data` が無いと `file_size` 系の `std::filesystem::filesystem_error` を投げる。`.onnx` だけでなく `.data` も配置すること（`*.onnx.data` も gitignore 済み）
  - [x] 段階2(CPU方式): `DebugScene::render` で neural 有効時に ①`render_result_texture_` を staging 読み戻し(`read_texture_2d_to_rgba8` ヘルパ) ②RGBA8→NCHW float[0,1]＋フレーム毎ノイズ(空間白色+時間EMA平滑→std≈1正規化) ③`stylize()` ④NCHW→RGBA8→`direct_3d_11::Texture` へ UpdateSubresource→`render_post_effect` で表示。**実機でスタイル化＋ボイリング動作確認**。ノイズ平滑は `neural_noise_temporal_`(0.35)
    - ⚠️ **fps 60→6 に急落**（CPU 往復のコスト顕在化、予定どおり）。内訳推定: GPU→CPU 読み戻しの同期待ち＋フル解像度推論＋毎フレ全画素の CPU 処理(normal_distribution ノイズ生成・RGBA8↔NCHW 変換)
- [x] Phase4(高速化B): **処理解像度を下げる**（長辺 `neural_proc_long_side_`、16の倍数にスナップ、imgui スライダ）。推論は小解像度・表示は GPU が拡大。推論を try/catch で保護。**6→16fps**（デバッガ有無は無関係）。スライダ `proc res` / `boil`
    - 📌 **判明: 残るボトルネックは「フル解像度の GPU→CPU 読み戻し(`Map` 同期待ち)」**。proc res を下げても読み戻しは常にフル解像度なので頭打ち → Phase5(C) で解決すべき部分。
    - 📌 **DirectML の `_com_error` は無害な first-chance 例外**（初回グラフコンパイル時に D3D12 内部で投げて自分で握りつぶす。スモークテストは全解像度で成功、推論は毎フレーム成功＝catch 未発動を実機ログで確認）。デバッガ下でのみ「投げられた瞬間」が見えるだけ。デバッグなし実行 or 例外設定で非表示にできる。
- [x] **計測で高速化の本丸を特定**（imgui に ms 表示を実装）。proc1080 の内訳: read 1.8 / pre 27.9 / infer 29.5 / post 8.2。
    - 📌 **読み戻しは 1.8ms = ボトルネックではない** → 当初想定の C(GPU直結/非同期読み戻し)は**不要**と確定（measure-first で無駄打ち回避）。infer はモデル計算コストで GPU 直結でも縮まない。
- [x] Phase5(高速化・実施分): **ノイズ生成を高速化**（事前計算 N(0,1) テーブル＋1パス＋固定スケール。`normal_distribution` 毎画素呼び＋正規化2パスを廃止）。**pre 27.9→4.4ms**。既定 proc res を 768 に（画質≈1080・速度2倍の "ひざ"）。**768 で 33fps**（read1.6/pre4.4/infer21.6/post4.3）、1080 で 21fps
- [x] **Phase6(高速化・fp16): モデルを fp16 化**（`tools/npr_offline/to_fp16.py`、入出力 fp32 維持＝C++無改修・モデル差し替えのみ。重みインライン化で `.data` 不要、3.5MB）。**実機 infer 21.6→6.5ms（約3.3倍速）/ 768 で 33→57fps**（ほぼ60fps！）。画質ほぼ同等。imgui `fp16 (off=fp32)` トグルで切替（変化時のみ再ロード）。既定 fp16
    - 📌 fp16 で infer が小さくなり、now のボトルネックは **CPU オーバーヘッド(read+pre+post=10.3ms > infer 6.5ms)** に逆転。さらに速くするなら pre/post 最適化 or 完全 GPU 化が今度こそ効く（ただし 57fps で十分実用）
- [ ] Phase7(任意・伸び小): pre/post の CPU パスを SIMD/並列化 or 完全 GPU 化。infer が小さくなった今は overhead 削減が効く（57fps→さらに上）
- [x] **Phase8(色保持 HLSL化＋うねり解消)**: オフライン `recolor_lab` を HLSL 移植（`post_effect/recolor.hlsl`＋`RecolorShader`、Lab で L=暗化制限・a控えめ・b青増幅/黄カット、t0=ニューラル出力/t1=元シーン）。imgui トグル `color preserve`＋`blue`/`dark clip`。**手描きシェーダの二重掛けも解消**: neural 表示時のみ post-effect を recolor(色保持) or default(素通し)に一時差し替え→描画後 restore（非NPRシーンに影響させない）。既定値は blue=3/yellow=0/a=0.5/dark=60
- [x] **CityGenerator クラッシュ修正**（NPR とは別件・既存バグ）: `road_node_list_`(vector<unique_ptr<RoadNode>>) の一括破棄で `~RoadNode`→`unlink()`が解放済み隣接を触る use-after-free。`~CityGenerator()` で破棄前に全ノードのリンクを null 化して回避。シーン遷移クラッシュ解消

### その後（②）
- [ ] ONNX Runtime + DirectML を導入し、`starry_boil_small.onnx` をポストエフェクトとして実機推論
- [ ] 色保持(recolor_lab)を HLSL 後処理で実装、フレーム毎ノイズ供給

---

## 完了済み

- [x] blue_sky / core レイヤー分析を実施
- [x] core が必要とする CB (ObjectCB, BoneCB) を core 層に移動
- [x] ゲーム固有コンポーネント (FpsPlayer 等) を blue_sky に移動
- [x] UI ウィジェット群を core 層に移動
- [x] BaseShader → GameMain 依存を解消 (core::Service 経由に変更)
- [x] ParticleRenderSystem → blue_sky 依存を解消 (シェーダーをコンストラクタ注入)

---

## Neural NPR オフライン検証 (エピックB / 段階0a)

実機統合前に「シンプルなレンダ結果を NPR 調へ変換したとき、時間的にちらつかず成立するか」を
オフラインで検証する。Asana エピックB。ブランチ `feature/neural-npr-offline-validation`。

### 完了
- [x] エンジン側フレームダンパー実装 (`source/Scene/DebugScene.cpp` の `dump_texture_2d_to_png` + imgui パネル、`dump/color_%04d.png`)
- [x] Python 検証ハーネス `tools/npr_offline/` 実装 (run_eval / io_utils / temporal / stylizers)
- [x] 隔離環境を uv で構築 (Python 3.12 venv、torch 2.11+cu128、RTX 5070 Ti / CUDA 認識OK)
- [x] スタイライザ: classic(セル調) / neural(VGG Gatys) / pencil(古典・ドッジ+XDoG) を実装
- [x] 評価実行・結果取得 (120フレーム、source/dump):
  - 安定化(reproject+EMA)は全スタイルで約40〜65%の時間誤差削減を確認
  - 鉛筆(pencil古典)が最も時間安定 (naive 0.0054)、neural系は素のちらつきが大きい (0.06〜0.09)
  - 参照画像版: 鉛筆=黒鉛デッサン(carmiencke) naive0.032→stable0.012(63%)、ペン画=Van Gogh葦ペン naive0.079→stable0.029(63%)
- [x] 成果のコミット完了:
  - フレームダンパー (DebugScene) = `c6ba6e56`（BGM 変更とは分離してコミット）
  - ハーネス一式 + CLAUDE.md 新規(MSBuild を vswhere でポータブル化) + todo/lessons/.gitignore = `6d580d90`

### ★方針転換と最終確定（2026-06）
**美的目標が反転した**: 当初は「ちらつきを消す（時間安定）」前提だったが、ユーザー判断で目標は
**手描きアニメの「ボイリング」＝画面全体が毎フレーム少し揺れる手描きの生命感**だと確定。
ただし「ノイズ（粒状チラつき）」は不可で、「落ち着き＋なめらかな微動」が正解。

確定までの探索（すべて brush_starry_night / sw=1e11 / 120f / 384px、目視で判断）:
- ちらつきの正体は2成分: ①各フレーム独立最適化のチラつき(boil=0でも残る粒状ノイズ) ②boil(初期値への白色ノイズ)。①は安定化でしか消えない。
- boil(白色ノイズ初期値)・極弱boilまで: どれも「粒状ノイズ」で不採用。質が手描きの揺れと違う。
- post-boil(安定化後ワープ)・輪郭マスク合成: いずれも方向違い/ノイズ過多で不採用。
- 残像問題: 安定化(warp+EMA, α高)で「画面が大きく変わる所に残像」。犯人はboilでなくwarp(新出領域で前フレームを引きずる)。`--warp-reject`(warp前後の食い違いが大きい画素を安定化から除外し今フレームで描き直す)+`--occ-feather`で解消。reject系のαソフト減衰/崖型はいずれも「部分的に動きすぎ」で不採用、warp-rejectのハードカット(boil=0前提)が正解。
- **コヒーレント・ボイリング(`--coherent-boil`)が答え**: 最適化初期値に「空間/時間的になめらかな揺れ場」を注入。白色ノイズと違い筆致のかたまりがゆっくりウネる。時間的になめらかなのでwarpが追従でき、安定化を生き延びつつ残像にならない。

**確定設定（brush_starry_night, 出力 `out_coh_wr0.07`）**:
`--style-weight 1e11 --flow gbuffer --alpha 0.9 --warp-reject 0.07 --occ-feather 15 --coherent-boil 0.06 --coherent-space 3 --coherent-time 2`（boil=0）
（warp-reject は 0.15→0.07 に強化して残像をさらに削減。0.05まで試したが 0.07 が残像消去と自然さのバランス最良）

- [ ] 残タスク: ①他スタイルでも確認(スタイルは将来オリジナルに差し替える前提) ②この成果のコミット(TortoiseGit) ③実機(リアルタイム)統合の検討。なお味系パラメータ(style/content/tv/coherent/steps)はリアルタイム速度に無関係(順伝播ネット化で最適化ループが消えるため)。リアルタイムで効くのは解像度/ネット本体/フロー源(gbufferで対処済)。

### フェーズ1（リアルタイム化＝順伝播ネット）試行と中断（2026-06-21）
**目的**: Gatys最適化(1フレーム数秒)を順伝播ネット(forward一発)に置換しリアルタイム化。新規コード:
`transformer_net.py`(Johnson系 encoder-decoder+残差), `train_feedforward.py`(VGG知覚損失で学習・早期終了・ONNX書出), `infer_feedforward.py`(推論・速度計測)。学習データはCOCO val2017(5000枚, `data/`=gitignore)。モデルは`models/`(gitignore)。
- **速度は完全クリア**: 720x405で約8ms/frame=**122fps**(RTX 5070 Ti)。リアルタイム余裕。容量増(残差5→8)でも9.4ms。
- **重要な学び**:
  - Gatysの重み比(style/content=1e11/1)は順伝播ネットに通用しない。Gatysは「content画像から最適化開始」で形をタダで得るが、順伝播はゼロから生成するためcontentをずっと強く効かせる必要(正規化Gram使用で style 1e10だとstyle項がcontentの約2000倍支配)。
  - content層は浅いrelu2_2("8")が細部保持に有利(conv4_2は構図のみ)。
  - **データ多様性が必須**: ダンプ120枚(似たフレーム)だと「入力無視で一定テクスチャを出す」過学習に陥り内容が出ない。COCOで解消。
  - 512px学習は num_workers=0 だとデータ読込ボトルネックで1epoch>2.5h(事実上停止)。`--workers`並列化で384px=150s/epochに正常化。
- **中断理由(ユーザー判断「全然だめ」)**: weight(1e5〜1e7収束)・容量(残差5/8)・解像度(256/384)のどのレバーでも、**Gatysの「渦巻く方向性の筆致」は出ず「点/星が散る質感」止まり**。これは「Gram統計を順伝播で学ぶ」方式の構造的限界(Gramは特徴の共起は測るが並び/方向は測らない)。
- **次にやるなら③ GAN**: 識別器に「本物のスタイルらしさ(渦構造)」を判定させる。推論速度は据置(生成器のみ)だが学習実装が重い。今回は未着手で保留。
- 暫定ベストモデル(渦は出ないが content保持+画風+122fps): `models/coco_sw3e6_conv.pth`(style3e6,relu2_2,5block,収束)。
- 新規 .py 群(transformer_net/train_feedforward/infer_feedforward)と tools/npr_offline/.gitignore は**未コミット**。
- [x] (旧探索の記録) warm-start=ボケ不採用 / temporal-loss(`--temporal-weight`)=固まり不採用 / tv正則化(`--tv-weight`)=高sw崩壊対策で実装済(今回は不使用)。詳細は上記。
- [x] (案B/段階0b) エンジンのモーションベクトルで warp 正確化 — **実装完了・データ取得済・確定設定で使用中**。
  - 当初の背景: temporal-loss(tw2000)で warp が「動くべき部分が固まる」破綻。原因は推定フロー(Farneback)の誤り→エンジンの正確なモーションベクトルで根治。最終的にこの gbuffer フローが安定化(warp-reject)の基盤になった。
  - エンジン側実装(blue-sky.lib/exe ビルド通過):
    - 定数バッファ末尾追記: ObjectCB に `prev_world`、FrameCB に `prev_view`/`prev_projection`（HLSL `common_cbuffer.hlsl` も）。既存オフセット不変。
    - 前フレーム行列の保持: `ActiveObject::update_render_data`(prev_world)、`DebugScene::render`(prev_view/projection)。初回は cur=prev で速度0。
    - 速度パス: `media/shader/main.fx` に technique `velocity`(VS が現/前クリップ位置→PS が NDC 速度を RG 出力)、`common_state.hlsl` に `VelocityDepth`(LESS_EQUAL/書込なし=シーン深度再利用)。`VelocityShader.h` 新規、`setup_default_shaders` で登録。`GraphicsManager::render_active_objects_velocity()`。
    - ダンプ: `DebugScene` の imgui「Dump depth + camera (G-buffer)」ON で、`dump/motion_%04d.raw`(RG float32, NDC速度) を出力。MSAA 可(色フォーマットなので resolve)。深度+カメラ(`depth_*.raw`/`cam_*.txt`)も併記ダンプ(再投影クロスチェック用、MSAA 切れば有効)。
    - 制約: 静的メッシュ(入力レイアウト"main")のみ。スキンメッシュ未対応(このシーンは静的なので可)。
  - ハーネス側実装: `gbuffer.py`(motion_raw→cur→prev 画素フロー+遮蔽マスク, `feather`対応)、`temporal.py` に `temporal_errors_pre`/`stabilize_pre`(`reject_sigma`/`reject_power`/`warp_reject`/`post_boil`/`coherent_boil_fields`)、`run_eval.py` に `--flow gbuffer`/`--warp-reject`/`--occ-feather`/`--coherent-boil`等。
  - データ取得済（カメラ前進ダンプ source/dump、color/motion/depth/cam 各120）。確定設定で gbuffer フロー使用中。
- [ ] (将来案④) 作例ベースのペア学習 (diffusion 教師/手描き作例) — Asana エピックB に起票済み

### フェーズ2（リアルタイム・ボイリング達成＝確率的ネット）（2026-06-28〜29）
**フェーズ1の「渦が出ない／ボイリングが出ない」を、確率的ネットで突破。リアルタイムで手描きの揺れを実現。**
探索順（ユーザー指示 3→2→1）と結論は `lessons.md`「リアルタイム・ボイリング」節に集約。要点:
- **決定論順伝播ネットは原理的にボイリング不可**（同入力→同出力）。入力への加算ノイズ＝ノイズが乗る／微小ワープ＝絵が泳ぐ、で両方却下。
- **容量↑（5/32→8/48）は筆致をほぼ変えない**（Gram の壁）。速度半減（121→52fps）だけ。→ 小容量採用。
- **Gatys はリアルタイム不可**（最速5.5fps）。ただし jitter init（style_scale 1.5, boil 0.15〜0.3）で**ボイリングの正解見本**を確定。
- **確率的ネットで達成**: `transformer_net_stochastic.py`（残差特徴へ StyleGAN 流ノイズ注入）＋ `train_stochastic.py`（mode-seeking 多様性損失、**多様性は浅層 relu1_1 で測る**＝構造を壊さない、content_weight 10）。推論 `infer_stochastic.py`（空間白色・時間平滑ノイズ）。**107fps(5/32) でボイリング＋内容保持**。
- **色保持**（`recolor_lab`）: Gram の全パレット混入（黒塊/黄/肌色）を後処理で除去。輝度転写＋Lab b 軸の青方向だけ増幅。確定 blue=3.0 yellow=0 a=0.5 dark=60。
- 採用モデル: `models/starry_boil_small.pth/.onnx`（5/32, 107fps）。big(8/48)は筆致の向きが少し複雑だが差は小、small 許容。
- **残**: ①別スタイルでの確認（次にやる。スタイルは将来オリジナルへ差し替え前提） ②実機統合（ONNX Runtime + DirectML、色保持は後処理/HLSL）。
- 新規/変更ファイル（このフェーズ）: `transformer_net_stochastic.py` `train_stochastic.py` `infer_stochastic.py` `probe_*.py`、`infer_feedforward.py`(coherent_noise/boil/warp 追加)。検証成果は `out.old/` へ退避（gitignore）。

### 補足（このセッションの状態）
- 作業ブランチ: `feature/neural-npr-offline-validation`（develop 未マージ）。マージは `--no-ff`、マージ後にブランチ削除。
- 作業ツリーに残るのはユーザーの変更のみ（`media` / `source/Scene/DebugScene.cpp` の BGM 残り / `source/core/sound/PortAudio/SoundEngine.cpp`、未追跡の `doc/*.md`・`devlog/*.png` 等）。私のタスク対象外。
- Git コミット運用: ユーザーが TortoiseGit で実行。Claude は `TortoiseGitProc.exe /command:commit /logmsgfile:<txt>` でメッセージ設定済みダイアログを出すまで（詳細は user スコープ `~/.claude/CLAUDE.md`）。
- 保留: `asana-experimental` MCP の OAuth 認証（再起動 + /mcp）。

---

## 重要度: 高 / バグリスク・アーキテクチャ違反

### ~~Shader.h が Direct3D11 ConstantBuffer を直接参照~~ [完了]
- [x] 対応済み — 抽象 ConstantBuffer に update() 追加、DynamicSlotConstantBuffer 新設、ファクトリ経由に切り替え

### ~~テクスチャアンロード後の dangling pointer~~ [完了]
- [x] 対応済み（979b2c32）— 紙テクスチャを ResourceManager のハンドルで持ち、bind_paper_texture は resolve して無ければ使わない

### ~~ウィンドウリサイズ時に RenderTargetTexture が再作成されない~~ [完了]
- [x] 対応済み — 遅延再作成 (Lazy Resize) 方式で解決。画面サイズ依存 RTT は activate()/clear() 時にサイズ不一致を検出して自動再作成。固定サイズ RTT (シャドウマップ等) は対象外。

### ~~スカイボックスのテクスチャ名が汎用的すぎる~~ [完了]
- [x] 対応済み — Model::get_texture_resource_name で登録名を決め、SkyBox は "skybox/front" のように接頭辞を付ける（読み込み・set_sky_box のアンロードとも）

---

## 重要度: 中 / 設計改善・拡張性

- [ ] 設定ファイルの形式（2026-11-10 検討）: 今の「キー 値」形式のままにする。UI のテーマ・キーの複数割り当て・セーブデータなど、まとまったデータを扱い始めるときに TOML（toml++）を入れ、新しいデータから使う
- [ ] 次の段階の課題 D1〜D5（doc/設計上の問題の洗い出し.md の 6 章）: D1 ゲームの物体を ECS に寄せる・D4 ハンドルの導入（どちらも利用者が決定・希望）。移行計画を立ててから進める
  - [x] ECS 移行の段階 0「土台の修理」完了（2026-11-10、doc/ECS移行計画.md の 3 章）: destroy_entity がシステムの対象から外してコンポーネントも外す（794d545a）・is_alive / find( EntityId )（dc031ebf）・PhysicsSystem が対象から外れたとき・remove_system・clear で剛体を解放（16826f67。以前はシーンをまたいで物理の世界に溜まっていたのをテストで確かめた）・GameObject の ComponentMap を削除（f3a810ca）。テスト 339 件。残り: シーンをまたぐ確かめは単体テストだけ（実機でシーンを替えての確認はしていない）
  - [x] ECS 移行の H1「ハンドルの土台」完了（2026-11-10、doc/ECS移行計画.md の 3 章）: ResourceManager に replace（番号を変えずに中身を差し替え、古い中身を解放。102d2107）・モデル・メッシュ・テクスチャを GraphicsManager からハンドルで引ける（load_model_handle・resolve_model・replace_model など。4163ac1c）・音を SoundManager からハンドルで引け、同じ名前で別のファイル・reload・ステージの load_sound の replace は差し替え（476fe98d）。型の食い違いは、モデル・メッシュを core の型（core::graphics::Model / Mesh）で管理し、ハンドルも core の型にして解いた（core 層の ModelComponent が blue_sky 層を知らずに持てる。引く入口は core の GraphicsManager::resolve_model）。テスト 345 件。残り: 使う側の生ポインタ（ActiveObject の Model* など）は段階 1 以降で移す。GraphicsManager のハンドルの関数は偽物の GraphicsManager が無いので単体テストは ResourceManager の段まで（起動して確かめた）
  - [x] ECS 移行の段階 1「描画データをコンポーネントへ」完了（2026-11-10、doc/ECS移行計画.md の 3 章）: ObjectRenderComponent・AnimationComponent・ObjectRenderDataSystem を足した（6b0b526c、単体テスト 9 件）。ActiveObject が Entity を 1 つ持ち、モデル（ModelHandle）・定数バッファ・前フレームの world・AnimationPlayer を部品へ移した。GraphicsManager は ActiveObjectManager ではなく ObjectRenderDataSystem の描く対象を回す（0ca44ae3。ActiveObject・ActiveObjectManager への依存を外した）。描く順は作った順。テスト 354 件。画面の見比べ（1-1・1-2・2-1・0-0 の 60・120 フレーム目）は変える前と同じ（揺れの範囲）、スイッチのログも 1-1・1-3 で同じ数・同じ座標。残り: 位置と描くかどうかの正は ActiveObject のまま（一方向に写す。位置は段階 3、表示の旗と kill は派生を部品にする段階 4・5 で移す）。2-1 のスイッチを全部押す確かめは、押せる数が起動ごとに変わる（押す順が unordered_set のアドレスの順）ので数の一致では比べられない
  - [x] 段階 1 で見つけた不具合を修正（2026-11-10、edf12d65）: BLUE_SKY_DEBUG_PRESS_SWITCHES で押せるスイッチの数が起動ごとに 2・5・13 個と変わっていた。物体の一覧が unordered_set< ActiveObject* >（アドレスの順）で、2-1 では最後に書いた switch-m の turn-on がほかの全部のスイッチを break するため、switch-m が早く回ると残りが「入らなかった」になっていた。一覧を vector（加えた順）にし、更新・スイッチを押す所は添字で回す（イベントで物体が加わっても壊れない）。2-1 を 3 回起動して 3 回とも入った 13・目標地点へ動かします 24・着きました 23 で同じ
  - [x] ECS 移行の段階 2「目標地点・向きへの移動」完了（2026-11-10、fd200d78、doc/ECS移行計画.md の 3 章）: TargetLocationComponent・TargetDirectionComponent・TargetMoverComponent（窓口 TargetMover を ActiveObject が private で実装）・MoveToTargetSystem を足し、ActiveObjectManager の 3 つの unordered_map をやめた。set_target_* は窓口として残る。ActiveObjectManager::update の以前と同じ所から update_targets を呼ぶ（EntityManager::update では動かさない）。TargetLocationTest を MoveToTargetSystemTest に移して 12 件足した（テスト 366 件）。0-1・1-1・1-2・2-0・2-1 のスイッチ・目標地点のログは移す前と同じ数・同じ座標。残り: 剛体の速度は ActiveObject を通して触る（段階 3・H2 で部品から直接に）。向く先の物体は EntityId で持つが、生きているかは Entity で見る（kill した物体は今までどおり追う）
  - [x] ECS の効率の点検（2026-11-10、doc/ECS移行計画.md の 6 節。前提 : Entity は最終的に 1 万を超える）: ベンチマーク EcsBenchmark（1 万・10 万個。既定では走らない。--gtest_also_run_disabled_tests --gtest_filter=EcsBenchmark.*、54290822）・型の番号を通し番号にし get_component が何も作らない（039b39e7。Release で部品を引くのが約 4 分の 1、作るのが約半分）・Component::get_type_id() const を削除（b956c756）・remove_system の erase（01cfeefa）・ObjectRenderDataSystem がアニメーションを持つ物体だけを回す（BaseSystem に付け外しを知りたい部品の仕組み、9c0ecc9b）・RenderSystem の map とシェーダーを使い回す（d5e71fd2）・PerceptionSystem の表引きを 1 回にし近くない対象にレイを飛ばさない（722df99f）・ParticleRenderSystem は 2 つ以上でも壊れないことを確かめ、前提を static_assert で守る（e4c0289c）。テスト 375 件。残り: System の update（Release で 1 個 約 20 ns、10 万個で 2 ms）と作る・消すは構造の問題のまま
  - [x] 部品の持ち方の作り替え 完了（2026-11-10、doc/ECS移行計画.md の 6.6）: 置き場 StablePool（値をページに詰めて置き、場所は外すまで動かない。剛体の userPointer・GraphicsManager のカメラ・シーンのメンバが部品のポインタを持ち続けるため）と EntityId → 添字の表 SparseIndex（unordered_map より Release で引くのが約 7〜15 倍速い）（72bb2ae5）・EntityManager が部品と Entity を置き場に値で持つ（3bc263d2）・System の対象を密な配列に（d2542cb2）・1 体ごとの仮想関数をやめ System 単位で一度だけ呼ぶ（1c3ab399）。Release 10 万個の update 2.22 → 0.165 ms、作る 48.5 → 11.6 ms、引く 3.41 → 0.76 ms、消す 32.4 → 3.0 ms。テスト 375 → 385 件。1-1・2-1 のスイッチのログは基準と同じ
  - [x] ObjectRenderDataSystem・MoveToTargetSystem の一覧を、対象が 1 つ増減するたびに全部作り直して並べ替えるのをやめた（2026-11-10、9c9989ee、doc/ECS移行計画.md の 6.6）: core::ecs::EntityIdOrderedList（足すときはたいてい末尾に足すだけ・外すときは印を付けて次に読むときにまとめて詰める）。1 万個で 1 つ足す 0.838 → 0.000 ms・1 つ消す 1.385 → 0.050 ms（Debug）。テスト 385 → 392 件
  - [x] ECS 移行の段階 3「物理の同期と当たりのイベント」と H2「剛体の持ち主を 1 つに」完了（2026-11-10、doc/ECS移行計画.md の 3 章）: 剛体を RAII の持ち主 core::physics::RigidBody にし世界の生の new を unique_ptr に（480d2ad8）・ECS の剛体の userPointer を EntityId のタグに（34cb8cbb）・本編の剛体もタグにし、ecs::CollisionEventSystem が当たりを集めて互換の層（ActiveObjectManager）が二重ディスパッチを呼ぶ、物体の性質を CollisionTraitsComponent に（3f70cb95）・位置の正を TransformComponent に、PhysicsSyncSystem が剛体と同期、本編の剛体の持ち主を PhysicsBodyComponent に（fa636bda）・開発用 BLUE_SKY_DEBUG_VISIT_OBJECTS（プレイヤーをメダル・風船・梯子・ゴールの所へ置いて結果をログに出す、7ae506c0）。テスト 392 → 413 件。スイッチ・目標地点のログは 1-1・2-1・1-2・0-1・2-0 で移す前と同じ数、画面もほぼ同じ。0-1・2-0 でメダル・風船・梯子・ゴールの当たりが起きることを開発用の手段で確かめた
  - [x] MoveToTargetSystem::update_targets が目標を持たない物体も含め全対象を毎フレーム回す（1 万個で Debug 1.1 ms）。目標を持つ物体だけの一覧にした（2026-11-10、e721902e。1 万個で 1.42 → 0.000 ms）
  - [x] 物体のゆらぎの種が物体のアドレスで、起動ごとに画面が変わる（2-3）。Entity の番号を種にした（2026-11-10、2acea460）
  - [x] ECS 移行の段階 4「単純な種類を ECS だけに」一部完了（2026-11-10、doc/ECS移行計画.md の 3 章）: ObjectFactory・GameObjectRef（Lua の "GameObject"）・EntityObject を足し、動かない物と動く床（a9c8f7fe）・メダル（421d2f60）・傘（d6662315）・ロケット（e5e70543）・ゴール（378f4e00）を ECS だけの物体にした。使っていない Missile を消した（329e5ad5）。テスト 413 → 435 件。スイッチのログ・画面・BLUE_SKY_DEBUG_VISIT_OBJECTS の結果は移す前と同じ
  - [x] ECS 移行の段階 4 の残り（梯子・石・スイッチ・空間スイッチ）完了（2026-11-10、doc/ECS移行計画.md の 3 章の「4」の行）: 開発用 BLUE_SKY_DEBUG_THROW_STONES（石を全部拾い、スイッチ・敵・風船へ投げて結果をログに出す。並行の作業の docs のコミット ede57ac9 に一緒に入った）・相手側を GameObjectRef・EntityId で受け取る形に（on_collide_with_stone・on_collide_with_ladder、Player の石の一覧は GameObjectRef、間引きは EntityId。6812b8ae）・梯子（LadderComponent・ContactCacheComponent・ContactCacheSystem。87e3bfe3）・石（ThrowableComponent・SpinComponent の degree_per_frame。7bb44c3d）・スイッチと空間スイッチ（SwitchComponent・SwitchSystem・ObjectFactory::create_area_switch、ECS だけの物体どうしの当たり on_collide_entity_objects、デバッグの dynamic_cast< BaseSwitch* > をやめた。0a4f8a3a）。Ladder・Stone・BaseSwitch・Switch・AreaSwitch クラスを消した。テスト 435 → 448 件。スイッチ・目標地点のログ（1-1・2-1・1-2・0-1・2-0）・物体巡り・石投げの結果は移す前と同じ。残り : 石を投げる・当てるのは開発用の手段だけで確かめた（手で遊んではいない）。ActiveObject の種類は Player・Girl・Robot・Balloon（段階 5・6）
  - [x] ステージの形式を独自の .stage から TOML（.stage.toml）へ移した（2026-11-10、doc/ステージ形式の設計.md の 6 章。設計 3.3 の段階 1〜5）: 開発用 BLUE_SKY_DEBUG_DUMP_STAGE（53bd888b）・読む所と当てる所を分ける StageData / LegacyStageParser / StageBuilder（5a93a87e）・toml++ v3.4.0 と TomlStageParser・動き Action・DelayedCommand が Action を持つ（86f40eea）・変換ツール tools/stage_convert/convert_stage.py と 16 個の .stage.toml（d418eb56）・ゲームとステージ選びが .stage.toml を読む（d12a97c6）・古い形式を消した（1e193ee7）。テスト 448 → 461 件。16 個すべてで古い形式と作った物体の一覧・スイッチ・物体巡り・石投げ・画面が同じ（揺れの範囲）。残り : [environment]・[sounds]・when の条件・部品の上書き（段階 6）、.intro・.outro の TOML 化
  - [x] ECS 移行の段階 5「Balloon・Girl・Robot」完了（2026-11-10、doc/ECS移行計画.md の 3 章の「5」の行）: 風船（BalloonHoldComponent・BalloonHoldSystem、点滅は描画の部品の is_hidden。a1b5327c）・少女（GirlComponent・GirlSystem・FlickerComponent。3b626c62）・敵（行動をまとめた RobotBehaviorComponent・RobotBehaviorSystem、テクスチャは TextureHandle、巡回の地点は PatrolComponent。da5e50cd）を ECS だけの物体にし、Balloon・Girl・Robot クラスを消した。プレイヤーの状態は窓口 ecs::BalloonHolder・ecs::ChaseTarget をプレイヤーが実装して渡す。開発用 BLUE_SKY_DEBUG_LOG_ACTORS を足した。テスト 461 → 481 件。作った物体の一覧・様子のログ・スイッチ・物体巡り・石投げ・画面は移す前と同じ（揺れの範囲）。残り : 敵の注目 → 追う・プレイヤーが傷つくは単体テストだけで確かめた（遊んでいない）。Perception・Alert・PatrolSystem への寄せは物理の世界を 1 つにするとき（理由は計画書）。ActiveObject の種類は Player だけ（段階 6）
  - [x] ECS 移行の段階 6「Player」完了（2026-11-11、doc/ECS移行計画.md の 3 章の「6」の行と「消せる条件」）: 入力を値（PlayerInputComponent）にし開発用 BLUE_SKY_DEBUG_PLAYER_SCRIPT（決まった入力の並びを流してプレイヤーの様子をログに出す）を足した（e09cadd9）・プレイヤーを行動をまとめた部品（PlayerBehaviorComponent・PlayerBehaviorSystem・PlayerInputSystem）で ECS だけの物体にし Player・BalloonHolder・ChaseTarget を消した（e2452eb1）・ActiveObject・ActiveObjectManager・GameObject を消し ObjectUpdater と ecs::contact_rules に分けた（36758535）・持ち物を PlayerInventoryComponent・PlayerInventorySystem に切り出した（3b1c5a4d）。テスト 481 → 494 件。移す前（25a4df1b）と入力の並び・スイッチ・物体巡り・石投げ・物体の一覧・様子のログ・画面が揺れの範囲で同じ。残り : 行動の部品から移動・梯子・風船とロケット（動作の状態）・視点はまだ分けていない（理由は計画書）。手で遊んでは確かめていない。ActiveObjectPhysics は物理の世界の統一（下）まで残す
  - [x] 本編の物理の世界（ActiveObjectPhysics）と ECS の物理の世界（PhysicsManager）を PhysicsManager に統一する（決定 1）完了（2026-11-11、doc/ECS移行計画.md の 3 章の「7」の行）: PhysicsManager に地面・OBJ の当たりの形・自分と相手を除くレイ・接触の判定・置いた箱の判定・形の共有を足し、RigidBody に剛体の操作を足した（fb58ec0e）・本編を PhysicsManager 1 つ（GameMain が持ち、シーンを替えるときに clear）に移し ActiveObjectPhysics を消した（42903a1f）・BulletPhysics を消し RigidBody::get を private にした（de0e9039）。値（重力・刻み・摩擦・形・振り分け）は変えていない。テスト 494 → 508 件。移す前と入力の並び・スイッチ・物体巡り・石投げ・物体の一覧・様子・画面が揺れの範囲で同じ、ECS のテストシーンも落ちない。Bullet の型が core/physics の外に残るのは core/math の変換と BulletDebugDraw だけ。残り : 手で遊んでいない。シーンを続けて替える確かめは実機でしていない。敵の Perception・Alert・PatrolSystem への寄せはしていない（理由は計画書の「5」の行）
  - [x] D3「描画の窓口が大きすぎる」完了（2026-11-11、doc/描画の窓口の分割.md）: GraphicsManager の関数を 11 の役割に分けて使う所を数え、使う側の使い方に合わせて小さな窓口 4 つに分けた（GraphicsManager が全部実装する）。ResourceResolver（引くだけ）・ResourceLoader（読む・作る）・RenderSettings（空・地面・色・線・紙）・Canvas2D（2D と文字）。run_action の set・fade の描画の部分は stage::StageLook に移した。使われていない関数 8 個を消した（4a65f31e・91d17036・446ccf2b・a2b51ac0・824d2e94）。偽物（test/graphics/FakeGraphics.h）でテスト 27 件（508 → 535 件）。全シーンと 7 ステージを移す前の exe と撮り比べ、揺れの幅に収まる。残り: フレームの流れ・フェード・デバッグ表示・低レベルは分けていない（使うのはシーン・GameMain・シェーダーだけ。シーンをテストするときに使う側に合わせて作る）
  - [x] 見つけた不具合（H1 とは別。~GameMain の最初でシーンを壊して直した）: game_play のシーンを開いたまま閉じると終了時に落ちる。GameMain のメンバの破棄の順で settings_ が scene_ より先に壊れ、GamePlayScene の fov_subscription_ が壊れた SettingStore から購読を外そうとする（crash.log : SettingStore::unsubscribe ← ~GamePlayScene ← ~GameMain）

### ~~extern/game の Sound/SoundManager を core に統合~~ [完了]
- [x] 対応済み（cbae6686）— 実装は core だけだったので、game::Sound・game::SoundManager の中身を core::Sound・core::sound::SoundManager に取り込み、extern/game から削除

### ~~ライト方向がハードコード~~ [完了]
- [x] 対応済み — GraphicsManager::set_light_direction / get_light_direction / reset_light_direction（core::graphics::LightDirection）。既定は以前と同じ。Lua は set_light_direction( x, y, z )
- 残り: シーンが変えた向きは次のシーンへ持ち越す（Scene の切り替えで reset するかは未決）。DebugScene・GamePlayScene は今も定数バッファに自前で光を書いている

### ~~シェーダーホットリロード後に effect_technique が古いまま~~ [完了]
- [x] 対応済み — 277ce0c4（読み直しの後に refresh_all_shaders で全シェーダーの technique を取り直す）と 1bba4f84（自前の Effect は取得済みの technique のオブジェクトを残して中身を入れ替える）。読み直しの成功・失敗を実機で確かめた

### ~~同種 ActiveObject 間でシェーダーを個別保持できない~~ [完了]
- [x] 対応済み — 原因は load_model が名前でモデルを使い回し、モデルのシェーダー一覧も共有していたこと。GraphicsManager::clone_model_with_shaders と ActiveObject::make_model_unique（Lua: make_model_unique( o )）で、そのオブジェクトだけのモデル・シェーダーにできる。Model の複製で線が消えていたのも直した（線は shared_ptr で共有）
- 残り: 複製したモデル・シェーダーは GraphicsManager が持ったまま（シーンを出ても解放しない）

### ~~ConstantBuffer の Usage/更新方式の使い分け~~ [完了]
- [x] 対応済み — 作るときに ConstantBufferUsage::DYNAMIC（Map(WRITE_DISCARD)）を選べる。既定は DEFAULT。フレーム・フレーム描画・共有オブジェクト・ActiveObject の world とボーン・ECS の RenderSystem / ParticleRenderSystem を DYNAMIC にした

### ~~シャドウマップの far clip が固定値~~ [完了]
- [x] 対応済み — core::graphics::ShadowMap::set_clip / set_far_clip（既定 1〜500）。setup_shadow_map で作り直すと既定に戻る

---

## 重要度: 低 / コード整理・最適化

### ~~GraphicsManager.cpp のシェーダー include を別ファイルに分離~~ [完了]
- [x] 対応済み — blue_sky/graphics/shader/AllShaders.h

### ~~setup_default_shaders() の関数名と責務分割~~ [完了]
- [x] 対応済み — create_default_shaders / select_shadow_map_shaders / setup_screen_rectangles に分け、setup_default_shaders はこの順に呼ぶだけ

### ~~2D 描画に法線を使っている~~ [完了]
- [x] 対応済み — 入力レイアウト "2d"（位置・UV。メッシュの頂点から法線を読み飛ばす）を main2d の頂点シェーダーから作り、フェーダー・ポストエフェクト・Flat2D・シャドウマップのデバッグ表示で使う。HLSL は COMMON_2D_INPUT
- 残り: 頂点バッファ自体は法線入りのまま（2D 専用の矩形メッシュは作っていない）

### ~~Entity::get_component() の const 正確性~~ [完了]
- [x] 対応済み（cbae6686）— 足す・外すは非 const、const の Entity からは const のコンポーネント。System は持ち主の EntityManager から得る

### ~~BulletPhysics::get_dynamics_world() を削除~~ [完了]
- [x] 対応済み（cbae6686）— 世界そのものを出さず ray_test・contact_test の問い合わせだけを出す。PhysicsManager の同名の関数（呼び出し元なし）も削除。テスト BulletPhysicsTest

### ~~CollisionShape の共有~~ [完了]
- [x] 対応済み（cbae6686）— 種類と大きさで形をキャッシュして共有する。テスト BulletPhysicsTest

### ~~AnimationPlayer の整理~~ [完了]
- [x] 対応済み（cbae6686・22bb672c）— 不要な別名・定数を削除、値を返さない代入演算子をコピー禁止に。定数バッファは ActiveObject が持ち、AnimationPlayer はボーン行列の計算（calculate_bone_matrices）だけ

### ~~シャドウマップカスケード幅の動的設定~~ [完了]
- [x] 対応済み — core::graphics::ShadowMap::set_cascade_width（既定 10 / 50 / 150 / 9999）

### ~~RenderTargetTexture の MipLevels/SampleDesc 可変化~~ [完了]
- [x] 対応済み — create_render_target_texture( format, RenderTargetTextureOptions )。既定はミップ 1 段・画面と同じ MSAA。ミップ 2 段以上は resolve_for_sampling でミップを作る

### ~~サウンド更新の別スレッド化~~ [完了]
- [x] 対応済み（cbae6686）— SoundManager::start_update_thread（16ms ごと）。一覧は SoundManager の、音の操作は音ごとのロックで守る。メインスレッドが止まっても BGM の補充が続く

### ~~その他の細かい整理~~ [完了]
エンジン本体（source/blue_sky・core・Scene・GameObject・Main.cpp・media/shader）の `@todo` / `TODO` 86 個を一つずつ判断し、すべて片付けた（残り 0 個。案として残すものは、何をすれば良いかの説明に書き換えた）。

- [x] GameMain の ActiveObject の色 — 解決済み。色はシェーダーの color パラメータで変え、1 つだけ変えたいときは make_model_unique で自分だけのモデルにする
- [x] GameMain の create_object() の引数・例外化 — 解決済み（42b42f8e で知らない名前は nil とログ）。さらに対応済み : スクリプトから nil を渡すと落ちていた関数（set_loc・clone_object・clone_model・set_post_effect_shader など 14 個）は、スクリプトのエラー（ScriptError）にする。C++ の関数が投げた例外は sol2 が Lua のエラーに変え、exec が ScriptError として投げることを確かめた（テスト ScriptManagerTest）
- [x] GameMain の雑多な整理 — 解決済み（該当の @todo はもう無い）。ファンクションキーの別名 enum Key は削除し VK_* を直接使う
- 案 : スクリプトからシェーダーを作る（GraphicsManager::create_default_shaders に、やり方を書いた）
- [x] 定数バッファの更新頻度・時間の置き場所 — 解決済み（1 フレーム 1 回の小さな更新。理由をコメントに書いた）
- 案（最適化。全シーンで FPS 60 が出ているので今は不要）: シャドウマップの描画の高速化、定数バッファを必要なステージだけに結ぶ（FlatShader・MatcapShader・LitShader・UnicolorShader・TessellationMatcapShader・post_effect::DefaultShader・GraphicsManager の線の描画）、Model::render をシェーダーごとにまとめる、AnimationPlayer で必要なボーン行列だけ計算する。方針は blue_sky/graphics/shader/Shader.h の説明に 1 つにまとめた
- [x] draw_text_at_center の高速化 — 解決済み（使うのは「読み込み中」の 1 回だけ）
- [x] SkinningAnimationSet の整理 — 対応済み : オフセット行列を new したポインタで持ち、コピーすると二重に解放する形だった。値で持つ（テスト SkinningAnimationSetTest）
- 案 : テクスチャごとのシェーダーステージ。テクスチャはピクセルシェーダーに結び、ほかのステージで使うものは各シェーダーの bind() で結ぶ（Shader.h に説明）
- 案 : スキニングの有無でシェーダーの型を分ける。今はスキニングの定数バッファがあるときだけ結ぶ（各シェーダーに説明）
- [x] Scene の各所 — 対応済み : CanvasTestScene のファイル static 変数（回転・拡大・ペンの状態がシーンを出ても残った）をメンバに。GamePlayScene の風船の const_cast をやめた。GamePlayScene に入るたびに "rectangle" のモデルとメッシュを作り直して溜め、GraphicsManager の画面の矩形の名前も奪っていたのを、使い回しに。ステージ選択・エンディングの cloth.png とストーリーの背景が同じテクスチャ名 "bg" で、先に読んだ画像が後のシーンに出ていたのを、名前を分けた。MusicGamePrototypeScene が入るたびにシェーダーの複製を溜めていたのを直した。GamePlayScene が Direct3D11/Sprite.h を直接使っていたのを抽象の Sprite.h に
- 案 : Scene の描画の GraphicsManager への移行。ほかのシーンでも同じ描画が要るようになったら移す（GamePlayScene.h に説明）
- 案 : BackBufferTexture と Texture の共通化。スワップチェーンへの解決があるので別にしている（BackBufferTexture.h に説明）
- 案 : core::graphics::Shader のパラメータ。名前で設定・取得する形はできている。既定値は数だけで、VECTOR・COLOR の既定値が要るときに型ごとに持たせる
- [x] core::graphics::GraphicsManager を太らせる — コメントを削除（作り手から受け取る今の形で困っていない）
- [x] そのほか @todo で見つかった不具合 — 対応済み :
  - ShadowMap の光の位置・行列が static で、作り直したゲーム（デバイスロストからの復旧）とも共有されていた → メンバに
  - Direct3D11・ShadowMap のデバイス・スワップチェーン・深度ステンシルが生ポインタで、コンストラクタの途中で例外が出るとリークした → com_ptr に
  - 法線の変換がワールド行列をそのまま掛けていて、拡大縮小で法線が傾き長さも変わった → 余因子行列で変換して長さを 1 に（common.hlsl）
  - PortAudio の出力（44100 Hz）で 22050 Hz の音楽 5 曲（road・semi・tokyo-shojo・tokyo-shojo-vocal・washer）が倍の速さで鳴っていた → 読み出しで周波数を合わせる（SampleRateStepper。テスト SampleRateStepperTest）。**耳での確認はまだ**
  - 目標の位置への移動（ステージの set_target_location）が、速いときに目標の周りを行ったり来たりして着かないことがあり、着いた後も最後の速度が残っていた → このフレームで届くなら着いたことにし、目標の位置に合わせて止める（テスト TargetLocationTest）
  - .obj の読み込みで、範囲外の番号でそのまま配列を読み、UV の無い "1//3" を誤読していた → ファイル名と行番号付きの例外、UV が無ければ ( 0, 0 )、多角形は三角形に分ける（テスト ObjFaceVertexTest）
  - MeshBuffer の中身が空の update_vertex_buffer / update_index_buffer（呼んでも何も起きない）を削除
  - CityGenerator の道路の左右の判定の重複を RoadNode::get_side_of に
- 残り（@todo には無かったが、同じ種類の問題）: CityGenerator・GeometryShaderCanvasTestScene も、作るたびに同じ名前のモデル・メッシュを作り直して古いものを溜める

## NPR コード品質改善（ブランチ refactor/npr-code-quality）
- [x] ② テクスチャ読み戻しを `core/graphics/Direct3D11/TextureReadback` に共通化（dump と neural の重複解消）
- [x] ③ 死にコード削除（DebugScene の未使用 hand_drawing_shader_）
- [x] ①④⑤ NPR パイプラインを `blue_sky::graphics::NeuralNprEffect` に抽出（DebugScene から状態24+ロジック150行を撤去、定数化、診断は imgui 内 dev ツールとして維持）
- [x] ⑥ ノイズ生成を `CoherentNoise.h` の `step_coherent_noise()` に自由関数化＋単体テスト（std≈1 検証、CoherentNoiseTest）
- 補足: recolor は HLSL のため C++ 単体テスト対象外。読み戻しは D3D デバイス依存でスモークのみ

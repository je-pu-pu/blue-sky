# TODO - 未着手タスクと進捗

## 運用ルール

1. タスクを追加するときはチェックボックス形式で書く
2. 完了したら `[x]` にする
3. セクションが全て完了したら、セクションごと削除してよい

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

### 補足（このセッションの状態）
- 作業ブランチ: `feature/neural-npr-offline-validation`（develop 未マージ）。マージは `--no-ff`、マージ後にブランチ削除。
- 作業ツリーに残るのはユーザーの変更のみ（`media` / `source/Scene/DebugScene.cpp` の BGM 残り / `source/core/sound/PortAudio/SoundEngine.cpp`、未追跡の `doc/*.md`・`devlog/*.png` 等）。私のタスク対象外。
- Git コミット運用: ユーザーが TortoiseGit で実行。Claude は `TortoiseGitProc.exe /command:commit /logmsgfile:<txt>` でメッセージ設定済みダイアログを出すまで（詳細は user スコープ `~/.claude/CLAUDE.md`）。
- 保留: `asana-experimental` MCP の OAuth 認証（再起動 + /mcp）。

---

## 重要度: 高 / バグリスク・アーキテクチャ違反

### ~~Shader.h が Direct3D11 ConstantBuffer を直接参照~~ [完了]
- [x] 対応済み — 抽象 ConstantBuffer に update() 追加、DynamicSlotConstantBuffer 新設、ファクトリ経由に切り替え

### テクスチャアンロード後の dangling pointer [難易度: 中]
- `GraphicsManager.cpp:430` — `paper_texture_` がアンロード後に無効参照になる可能性
- unload_texture() 時にオブザーバー通知 or weak_ptr 化が必要
- [ ] 対応する

### ~~ウィンドウリサイズ時に RenderTargetTexture が再作成されない~~ [完了]
- [x] 対応済み — 遅延再作成 (Lazy Resize) 方式で解決。画面サイズ依存 RTT は activate()/clear() 時にサイズ不一致を検出して自動再作成。固定サイズ RTT (シャドウマップ等) は対象外。

### スカイボックスのテクスチャ名が汎用的すぎる [難易度: 低]
- `Direct3D11/GraphicsManager.cpp:259` — "front", "back" 等の名前で競合リスク
- プレフィックス付き ("skybox/front" 等) に変更
- [ ] 対応する

---

## 重要度: 中 / 設計改善・拡張性

### extern/game の Sound/SoundManager を core に統合 [難易度: 中]
- `extern/game/SoundManager.h:14`, `extern/game/Sound.h:10`
- game 層と core 層に同名クラスが並存している
- [ ] 対応する

### ライト方向がハードコード [難易度: 中]
- `GraphicsManager.cpp:484` — `Vector(-1, -2, 0.5, 0)` 固定
- ECS のコンポーネント化、またはシーンごとの設定を可能にする
- [ ] 対応する

### シェーダーホットリロード後に effect_technique が古いまま [難易度: 中]
- `GameMain.cpp:92` — reload 時にキャッシュ無効化が不完全
- 開発中にちらつきやクラッシュのリスク
- [ ] 対応する

### 同種 ActiveObject 間でシェーダーを個別保持できない [難易度: 高]
- `GameMain.cpp:277` — シェーダー共有のため個別パラメータ設定不可
- clone / per-instance パラメータの仕組みが必要
- [ ] 対応する

### ConstantBuffer の Usage/更新方式の使い分け [難易度: 中]
- `ConstantBuffer.h:26,54` — 全て DEFAULT + UpdateSubresource 固定
- 頻繁更新の CB は DYNAMIC + Map の方が効率的
- [ ] 対応する

### シャドウマップの far clip が固定値 [難易度: 低]
- `ShadowMap.cpp:109` — far=500 固定、地面が範囲外になる可能性
- [ ] 対応する

---

## 重要度: 低 / コード整理・最適化

### GraphicsManager.cpp のシェーダー include を別ファイルに分離 [難易度: 低]
- `GraphicsManager.cpp:12` — 12個のシェーダーヘッダ include
- [ ] 対応する

### setup_default_shaders() の関数名と責務分割 [難易度: 低]
- `GraphicsManager.cpp:314` — 3つの処理が混在
- [ ] 対応する

### 2D 描画に法線を使っている [難易度: 低]
- `Direct3D11.cpp:410`, `GraphicsManager.cpp:683` — 2D 用 InputLayout が未整備
- [ ] 対応する

### Entity::get_component() の const 正確性 [難易度: 低]
- `Entity.h:43` — const メソッドが非 const ポインタを返す
- [ ] 対応する

### BulletPhysics::get_dynamics_world() を削除 [難易度: 低]
- `BulletPhysics.h:53` — Bullet 内部 API の露出
- [ ] 呼び出し元を確認して削除

### CollisionShape の共有 [難易度: 中]
- `BulletPhysics.h:12` — 同一形状の RigidBody でメモリ重複
- [ ] 対応する

### AnimationPlayer の整理 [難易度: 低]
- `AnimationPlayer.h:16,30` — 不要な type alias、定数バッファの責務分離
- [ ] 対応する

### シャドウマップカスケード幅の動的設定 [難易度: 低]
- `ShadowMap.cpp:103` — 固定値をメンバ変数 + setter に変更
- [ ] 対応する

### RenderTargetTexture の MipLevels/SampleDesc 可変化 [難易度: 低]
- `RenderTargetTexture.cpp:11,30` — コンストラクタ引数追加
- [ ] 対応する

### サウンド更新の別スレッド化 [難易度: 高]
- `GameMain.cpp:310` — 現在メインスレッドで実行
- [ ] 対応する

### その他の細かい整理
- `GameMain.cpp:253` — ActiveObject に color を持たせるか検討
- `GameMain.cpp:262` — create_object() の引数整理
- `GameMain.cpp:268` — 例外化してスクリプト側でキャッチ
- `GameMain.cpp:663,767` — 各所の雑多な整理
- `GraphicsManager.cpp:313` — スクリプトからシェーダー生成
- `GraphicsManager.cpp:446,487` — 定数バッファ更新頻度、時間データの場所
- `GraphicsManager.cpp:498,578` — シャドウマップ高速化、バインド最適化
- `Direct3D11/GraphicsManager.cpp:639` — draw_text_at_center 高速化
- `SkinningAnimationSet.h:89` — 整理
- `Shader.h:21` — テクスチャごとのシェーダーステージ設定
- `FlatShader.h:43,50` / `MatcapShader.h:41,48` / `LitShader.h:42,49` — バインド最適化、スキニング分離
- `UnicolorShader.h:32` / `TessellationMatcapShader.h:57,84` / `post_effect/DefaultShader.h:42` — バインド最適化
- `Scene/*.cpp` 各所 — GraphicsManager への移行、整理
- `BackBufferTexture.h:17` — Texture と共通化
- `core/graphics/Shader.h:18,38` — パラメータ設計
- `core/graphics/GraphicsManager.h:32` — 太らせる

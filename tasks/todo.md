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

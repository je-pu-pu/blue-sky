# ECS 移行計画（ActiveObject の継承をやめ、リソースをハンドルで持つ）

2026-11-10 作成。利用者の決定 :「ゲームの物体は ECS に寄せる」「生ポインタをやめてハンドルを導入する」。
背景は doc/設計上の問題の洗い出し.md の 6 章（D1・D4）。パスは source/ からの相対。「（推測）」以外はコードで確かめた。

## 1. 今の作り

### 1.1 継承の木

```
GameObject
├─ Camera
└─ ActiveObject                 ← 位置・モデル・アニメーション・剛体・行動を全部持つ
   ├─ Player / Girl / Robot（敵）/ Missile（.cpp が空・未使用）/ Rocket / Stone
   ├─ Balloon / Medal / Umbrella / Ladder / Goal
   ├─ BaseSwitch ─ Switch / AreaSwitch
   └─ StaticObject ─ TranslationObject（「動かない物」の派生で「動く物」）
```

### 1.2 ActiveObject が受け持っていること

| 受け持ち | 場所 | 中身 |
|---|---|---|
| 位置 | GameObject.h:57、ActiveObject.cpp:198-225 | 剛体との同期は GameObject.cpp:37-78（update_transform／commit_transform） |
| 物理 | GameObject.h:58 btRigidBody* | 剛体の持ち主は ActiveObjectPhysics。Bullet の userPointer は ActiveObject*（ActiveObjectPhysics.cpp:86） |
| モデルと描画 | ActiveObject.h:48-52、ActiveObject.cpp:282-398 | Model* は生ポインタ（持ち主は GraphicsManager）。GraphicsManager.cpp が ActiveObjectManager の一覧を直接回す（540 定数・616 影・650/671 本体と線・700 速度） |
| アニメーション | ActiveObject.cpp:77-95 | 毎フレームの更新は ActiveObjectManager.cpp:263-266 |
| 行動 | virtual update() | ActiveObjectManager.cpp:256-261。プレイヤーの入力は GamePlayScene.cpp:371-511 から直接呼ぶ |
| 目標地点・向きへの移動 | ActiveObjectManager.cpp:271-305 | 着いたら on_arrive_at_target_location() |
| 音 | GameObject.cpp:335-368 | 名前で引いて物体の位置で鳴らす |
| ステージからの生成・操作 | GamePlayScene_command.cpp:396-611・219-301、ActiveObjectManager.cpp:37-47・148-226、GameMain.cpp:467-485 | ステージは Lua ではなくテキストのコマンド。名前で引いた ActiveObject* を操作する |
| Lua | ScriptManager.h:58-64 | ActiveObject を Lua の型 "GameObject" として登録（model・play_animation・kill） |
| 当たり | GameObject.h:131-145、ActiveObjectPhysics.cpp:124-158 | 型ごとの多重定義による二重ディスパッチ |

使われていない古い橋渡し : GameObject.h の ComponentMap（blue_sky::Component は定義が無い）。段階 0 で消した（f3a810ca）。

### 1.3 派生クラスの大きさ（.cpp + .h の行数）

Player 1170+284・Robot 392+92・BaseSwitch 146+75（Switch 50+39・AreaSwitch 28+44）・Balloon 109+54・Girl 61+66・その他（Stone・TranslationObject・Rocket・Umbrella・Ladder・Goal・Medal・StaticObject）は .cpp が各 25〜37 行。基底は ActiveObject 445+166・GameObject 370+205・ActiveObjectManager 308・ActiveObjectPhysics 204。ActiveObject を参照するファイルは test を除き 47。

### 1.4 ECS 側にあるもの・足りないもの

- ある : core の Transform・RigidBody・Model（core::graphics::Model* の生ポインタ）・Render・Camera・ParticleSystem、PhysicsSystem・RenderSystem・Particle 系。blue_sky/ecs の Alert・Patrol・Perception・Perceivable・FpsPlayer・Balloon・TransformControl。
- **土台の欠陥**（段階 0 で直した） : ① EntityManager::destroy_entity が Entity だけを消し、コンポーネントが残り、システムが指したままになる（EntityManager.h:67-70）。② PhysicsManager::destroy_rigid_body を呼ぶ所が無く、ECS の剛体が溜まる（シーンをまたいで残ることをテストで確かめた）。
- 足りない : アニメーションとボーン行列、本編の描画（影・手描きの線・速度パス）、本編の物理の世界への対応、当たりのイベント、目標地点への移動、ステージからの生成と名前で引く仕組み、Lua から Entity を指す方法、プレイヤーの入力、物体の性質（すり抜け・硬さ・足場）のデータ。
- EnemyAiTestScene は手本になる（依存をコンストラクタで受け取る・型を知らずに印で探す・テストがある）。ただし敵に剛体が無く、描画・物理が本編と別の仕組み。

## 2. 戦略

**(a) 橋渡しで進める。** ActiveObject が Entity を 1 つ持ち、描画データ・目標地点・剛体の参照を順にコンポーネントとシステムへ移す。ActiveObject は薄い窓口になり、最後に種類ごとに ECS だけの物体へ置き換える（この段階は「新しい作り方で作り直す」(b) のやり方）。

- (b) 新しい物体だけ ECS : 物理の世界まで別なので、ECS の物体を本編のステージに置けない。2 系統が長く並ぶ。
- (c) 一気に書き直し : 勧めない。Player とステージのコマンド・当たりが絡み合い、ゲームが動かない期間が長い。本編の振る舞いを守るテストも少ない。

## 3. 段階

各段階でゲームが動き、テストが通る。確かめ方は、テスト・開発用の BLUE_SKY_START_STAGE と BLUE_SKY_CAPTURE_FRAMES による移す前と後の画面の見比べ・BLUE_SKY_DEBUG_PRESS_SWITCHES のログの一致。

| 段階 | 中身 | 確かめ方・危ないところ |
|---|---|---|
| **0 土台の修理** | destroy_entity がコンポーネントを外してシステムに知らせる。is_alive・find(EntityId)。PhysicsSystem が外れたとき・clear のときに剛体を解放する。ComponentMap を消す | EntityManagerTest に「消した後の対象数 0」「剛体の解放」。clear の順番（GameMain.cpp:1325-1329）。**完了（794d545a・dc031ebf・16826f67・f3a810ca）**。剛体はシーンをまたいで溜まっていた（修正前にテストで確かめた）。clear は System を先に壊す順に変えた |
| **H1 ハンドルの土台**（0 と並行可） | ResourceManager に replace（番号を変えずに中身を差し替え）。モデル・メッシュ・テクスチャ・音をハンドルで引けるようにする。ModelComponent（core::graphics::Model）と ResourceManager（blue_sky::graphics::Model）の型の食い違いを解く | 単体テスト（解放後は nullptr・replace 後も同じハンドルで新しい中身）。音は SoundManager が生で delete している（SoundManager.cpp:135-145）。**完了（102d2107・4163ac1c・476fe98d）**。型の食い違いは「管理を core の型にする」で解いた : blue_sky の GraphicsManager は ResourceManager< core::graphics::Model / Mesh > で管理し（中身は blue_sky の Model・Mesh）、ハンドルの型 core::graphics::ModelHandle・MeshHandle・TextureHandle は core の各ヘッダに置く。引く入口 resolve_model は core::graphics::GraphicsManager の純粋仮想（blue_sky が blue_sky の Model* を返す形で上書き）なので、core 層の ModelComponent・RenderSystem も blue_sky 層を知らずにハンドルを持ち・引ける。音は ResourceManager< Sound > で持ち、同じ名前で別のファイル・reload は番号を変えずに差し替え（ステージの load_sound の replace も）。使う側の生ポインタ（ActiveObject の Model* など）はまだ変えていない（段階 1 以降） |
| **1 描画データをコンポーネントへ**（最初の目に見える段階） | ObjectRenderComponent（Handle<Model>・定数バッファ・前フレームの world）・AnimationComponent・ObjectRenderDataSystem。GraphicsManager は ActiveObjectManager ではなく描く対象の一覧を回す（依存を外す） | 画面の見比べ。clone() で Entity とバッファを作り直す。make_model_unique はハンドルの差し替えに。描く順の変化（推測）。**完了（6b0b526c・0ca44ae3）**。部品は blue_sky::ecs の ObjectRenderComponent（ModelHandle・定数バッファ・前フレームの world・描くかどうか）・AnimationComponent、System は ObjectRenderDataSystem（ObjectRenderComponent と core の TransformComponent が対象）。ActiveObject は加えるとき（attach_entity）に Entity を作り、壊すときに消す。EntityManager は GameMain が ActiveObjectManager に渡す。位置と描くかどうかの正は当面 ActiveObject（派生の Balloon・Girl が is_mesh_visible などを上書きするため。kill の死亡と表示の旗も ActiveObject に残した）で、GameMain が描く前に ActiveObjectManager::sync_render_components で一方向に写す。GraphicsManager は set_entity_manager で受け取り、ObjectRenderDataSystem の描く対象（get_render_targets）を回す（ActiveObject・ActiveObjectManager への依存は 0）。描く順は Entity の番号の順（作った順）にした（以前は unordered_set< ActiveObject* > のアドレスのハッシュの順で、起動ごとに変わり得た）。アニメーションは ActiveObjectManager::update の後で update_animations。ObjectRenderDataSystem は EntityManager::update() では動かさない（本編のシーンは呼ばず、呼ぶシーンで二重に進めないため）。GameMain のデストラクタで、EntityManager より先に物体を壊す。画面は 1-1・1-2・2-1・0-0 で変える前と同じ（揺れの範囲）、スイッチのログも同じ。テスト 354 件 |
| **2 目標地点・向きへの移動** | TargetLocation／TargetDirection コンポーネント・MoveToTargetSystem・着いたときのイベント。set_target_* は窓口として残す | スイッチのログが移す前と同じ数・同じ座標 |
| **3 物理の同期と当たりのイベント**（山場） | 位置の正を TransformComponent に。userPointer を EntityId のタグに。当たりを集めるシステム → 互換の層が今の二重ディスパッチを呼ぶ。物体の性質をデータに | 振り分けの単体テスト。風船・梯子・メダル・ゴールのステージ。userPointer を読む所の直し漏れ |
| **H2 剛体の持ち主を 1 つに**（3 と同時か直後） | ムーブだけできる RAII の剛体をコンポーネントが持つ。BulletPhysics・PhysicsManager の生の new を unique_ptr に | |
| **4 単純な種類を ECS だけに** | StaticObject・TranslationObject → Medal・Goal・Ladder・Rocket・Umbrella・Stone → Switch・AreaSwitch（dynamic_cast を SwitchComponent に）。ObjectFactory（種類名 → EntityId）。Lua の "GameObject" は EntityId を包む | 工場のテスト・スイッチのログ・各ステージの画面 |
| **5 Balloon・Girl・Robot** | Robot は Patrol・Perception を本編の物理で動くように広げ、状態ごとのテクスチャは Handle<Texture> | |
| **6 Player** | 入力（GamePlayScene.cpp:371-511）を PlayerInputSystem へ。移動・梯子・持ち物・風船・視点を分ける。Ladder*・Balloon* は EntityId で持つ | 手で遊んで確かめる時間を一番多く取る |

**ActiveObject を消せる条件** : 全ステージで ActiveObject が 0・ActiveObject.h の include が 0・userPointer がすべて EntityId のタグ・GraphicsManager・Lua・GameObjectContext が ActiveObject を知らない・全ステージの画面とスイッチ・ゴールのログが移す前と一致。

## 4. ハンドルにするもの・unique_ptr にするもの

- **ハンドル** : 共有され、差し替え・解放があり、多くの所から指されるもの（Model・Mesh・Texture・Shader・Sound）。
- **unique_ptr／RAII** : 持ち主が 1 つで寿命が同じもの（剛体・定数バッファ・AnimationPlayer・物理の世界）。
- **Entity への参照** : EntityId。番号を二度と使わない（EntityManager.h:45、clear でも戻さない）ので、is_alive で確かめれば世代は要らない。Entity* はシーンの中の一時的な使用に限る。
- 順番 : H1 を先か 0 と並行に（段階 1 で生まれる ObjectRenderComponent が最初から Handle<Model> を持つように）。テクスチャとシェーダーのハンドル化（シェーダーが持つ Texture* など）は範囲が広いので別の流れで後から。

## 5. 決めること

1. 最後に残す物理の世界（推奨 : core::physics::PhysicsManager に統一）
2. 行動の書き方（推奨 : 単純な種類はデータとシステムに分け切り、Player・Robot は当面 BehaviorComponent を許して段階的に分ける）
3. ハンドルの中身（推奨 : 今の ResourceHandle（通し番号と map）を型付きのまま広め、replace を足す。添字＋世代への変更は遅さが測れてから）

決めた結果は、この節に日付付きで書き足す。

### 決定（2026-11-10、利用者）
1. 物理 : core::physics::PhysicsManager に統一する（Bullet を隠す）。足りない機能（OBJ の当たりの形の読み込み・地面・自分を除くレイ判定）は BulletPhysics から移す。移し終えるまで本編は今の世界のまま橋渡しで進める
2. 行動 : 折衷。単純な種類はデータとシステムに分け切る。Player・Robot は当面、行動をまとめて持つ部品（BehaviorComponent）で ECS の中で動かし、1 つずつ切り出す
3. ハンドル : 今の ResourceHandle（型付き・二度と使わない通し番号と表引き）を広げ、replace（番号を変えずに中身の差し替え）を足す。遅さが測れたら中身だけ添字＋世代に替える

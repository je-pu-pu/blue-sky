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
| 目標地点・向きへの移動 | ActiveObjectManager.cpp:271-305（段階 2 で blue_sky/ecs/system/MoveToTargetSystem へ移した） | 着いたら on_arrive_at_target_location() |
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
| **2 目標地点・向きへの移動** | TargetLocation／TargetDirection コンポーネント・MoveToTargetSystem・着いたときのイベント。set_target_* は窓口として残す | スイッチのログが移す前と同じ数・同じ座標。**完了（fd200d78）**。先に、物体の一覧を unordered_set< ActiveObject* > から vector（加えた順）にした（edf12d65。スイッチを押す順がアドレスの順で、2-1 では全部を壊す switch-m が何番目かで押せる数が 2〜13 と変わっていた）。部品は blue_sky::ecs の TargetLocationComponent（地点・速さ。着いたら外す）・TargetDirectionComponent（決まった向き か 向く先の EntityId を 1 つにまとめた。以前は 2 つの map で片方を設定すると片方を消していた。向く先の Entity が消えたら捨てる）・TargetMoverComponent（窓口 TargetMover* を持つ。全 ActiveObject が attach_entity で付ける）。MoveToTargetSystem は System< TargetMoverComponent > で、Entity の番号の順に 目標地点 → 向き の順で回す。剛体の速度は部品に btRigidBody* を持たず、ActiveObject を通して触る（ActiveObject が TargetMover を private で実装。位置の正と剛体はまだ ActiveObject にあり、set_rigid_body の差し替えで古い剛体を指し得るのと、set_location の剛体への写し・set_velocity の起こし方を二重に持たないため。段階 3・H2 で部品を直接触る形に替える）。着いたら置いて速度 0 にし、部品を外してから on_arrive_at_target_location を一度だけ呼ぶ（呼ぶ時点は以前と同じ。外すのを先にしたので、知らせた先で設定した新しい目標は消えない）。EntityManager::update では動かさず、ActiveObjectManager::update が以前と同じ所（物体の update とアニメーションの後・物理の前）から update_targets を呼ぶ。restart で全部の目標を捨てる。テストは剛体なしの偽の TargetMover で 15 件（366 件）。0-1・1-1・1-2・2-0・2-1 のスイッチの数・「目標地点へ動かします」「着きました」の数と座標は移す前と同じ（動かし始めの座標の小数 3 桁目は、移す前の同じ版でも起動ごとに揺れる） |
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

## 6. 効率の点検（2026-11-10）

### 6.1 前提

利用者の指示（2026-11-10）: Entity の数は最終的に **1 万を超える**想定で設計する。今の本編は 1 ステージ 60 個程度だが、それを理由に「効かないから直さない」とはしない。

### 6.2 見つかったこと

小さな直し・不具合の芽（すべて 6.3 で直した。5 は推測が外れた）:

| # | 場所 | 中身 |
|---|---|---|
| 1 | core/ecs の EntityManager・Component・System・BaseSystem | 部品と System の型の番号が typeid( T ).hash_code()。MSVC では呼ぶたびに装飾名を走査して計算する（constexpr と書いてあったが実行時）。理屈の上では衝突もし得る。get_component が operator[] で、読むだけで未登録の型の空の一覧を作る |
| 2 | blue_sky/ecs/system/ObjectRenderDataSystem | アニメーションの無い物体も含め、毎フレーム 2 か所（アニメーションを進める・定数を書く）で find と get_component< AnimationComponent > |
| 3 | core/ecs/system/RenderSystem | 毎フレーム unordered_map< Model*, vector< Matrix > > を作り直し、シェーダーを名前で 3 回引く |
| 4 | blue_sky/ecs/system/PerceptionSystem | 敵ごとに get_system< TargetRegistrySystem >()。視界錐に入った対象すべてにレイを飛ばしてから距離で捨てる |
| 5 | core/ecs/system/ParticleRenderSystem | 頂点バッファが 1 つで最初の Entity で作る → 粒子の Entity が 2 つ以上で壊れる（推測）。**確かめた結果、壊れない** : 粒子の一覧は std::array で大きさが型で決まり、Entity ごとに WRITE_DISCARD で書いてすぐ描く。粒子の Entity を一時的に 2 つにして起動し、描いたパスが 2 → 3 に増え、落ちず assert も出ないことを見た |
| 6 | core/ecs/Component.h | get_type_id() const が typeid( this )（Component* の型）で、どの部品でも同じ値。呼ぶ所は無い |
| 7 | EntityManager::remove_system | 1 引数の erase( remove( ... ) )。一覧に無ければ end() を消す未定義動作（今は system_map_ と揃っているので起きない） |

今は直さなかった構造の問題（1 万を超えると効く。次の作り替えで直す）:

- **部品を 1 つずつ new し、Entity ごとの unordered_map で持つ**（EntityManager::component_list_ は 型 → unordered_map< EntityId, unique_ptr< Component > >）。作る・消すたびに確保と解放、引くたびにハッシュ表を引き、部品はヒープに散らばる。
- **System の対象が unordered_map< EntityId, tuple< T*... > >**（System::component_list_）。走査がハッシュ表の節を飛び飛びにたどり、さらに各ポインタの先（散らばった部品）へ飛ぶ。
- **1 体ごとに仮想関数で update( ComponentTuple& ) を呼ぶ**。中身が小さい System では呼び出しの手間が目立ち、インライン化もできない。
- （ついでに）ObjectRenderDataSystem の描く対象の一覧は、対象が 1 つ増減するたびに全部を作り直して並べ替える。毎フレーム物体が生まれ・消える状況では毎フレーム O(n log n) になる。

### 6.3 直したもの

| # | 中身 | コミット | 足したテスト |
|---|---|---|---|
| 8 | ベンチマーク（1 万・10 万個で 作る＋部品 3 つ付ける・部品を引く・System 2 つの update・消す。既定では走らない） | 54290822 | EcsBenchmark.DISABLED_Entities10k / 100k |
| 1 | 型の番号を型ごとの通し番号（get_component_type_id< T >・get_system_type_id< T >。初めて引いたときに一度だけ振る。全部が 1 つの exe に静的にリンクされ DLL をまたがない）にし、部品の一覧を番号を添字に引く deque にした（MSVC の unordered_map は移動が noexcept でなく vector では伸ばせない）。get_component は find | 039b39e7 | TypeIdsAreStableAndDistinct・GetComponentOfNeverAddedTypeReturnsNull |
| 6 | Component::get_type_id() const を消した | b956c756 | （呼ぶ所が無い） |
| 7 | remove_system を erase( remove( ... ), end() ) に | 01cfeefa | RemoveSystem |
| 2 | BaseSystem に「対象ではないが付け外しを知りたい部品」（is_watched_component_type・on_watched_component_changed）を足し、ObjectRenderDataSystem はアニメーションの付け外しで一覧を作り直す印を立てる。作り直すときに RenderTarget::animation を一度だけ引き、アニメーションを持つものだけの一覧を回す | 9c0ecc9b | 描く対象はアニメーションを持ち_付け外しで一覧が作り直される・System_を後から足してもアニメーションを引ける |
| 3 | RenderSystem の map をメンバにして中身だけ消す（前のフレームで使わなかったモデルは外す）。シェーダーはコンストラクタで一度だけ引く（無ければログを残してポストエフェクトなしで描く） | d5e71fd2 | （Direct3D が要るので起動して確かめた） |
| 4 | PerceptionSystem は update の頭で TargetRegistrySystem を一度だけ引く。dist >= best_dist ならレイを飛ばさない（採る対象は以前と同じ） | 722df99f | PerceptionSystemTest 4 件 |
| 5 | 動きは変えず、「粒子の一覧は大きさが型で決まる配列」という前提を static_assert で守り、可変長にするときの直し方を書いた | e4c0289c | （同上） |

テストは 366 件 → 375 件（ほかに無効 2 件 = ベンチマーク）。

### 6.4 計測の結果

blue-sky-test.exe --gtest_also_run_disabled_tests --gtest_filter=EcsBenchmark.* を 3 回走らせた中央値（ミリ秒）。部品は 3 種類（位置・速度・体力、各 12〜16 バイト）、System は 2 つ（部品 2 種類・3 種類）で、中身はほぼ空（ECS の仕組みの手間だけを測る）。「部品を引く」は全 Entity について 3 種類を引く 1 回ぶん（10 回の平均）、「update」は全 System の update 1 回 = 1 フレーム相当（30 フレームの平均）。x64・開発機。

| 構成 | 個数 | 項目 | 直す前（54290822） | 1 の後（039b39e7） | 全部の後（e4c0289c） |
|---|---|---|---|---|---|
| Debug | 1 万 | 作る＋部品 3 つ | 63.58 | 45.95 | 47.26 |
| Debug | 1 万 | 部品を引く | 5.70 | 3.98 | 3.99 |
| Debug | 1 万 | update（1 フレーム） | 0.251 | 0.248 | 0.239 |
| Debug | 1 万 | 消す | 16.55 | 16.33 | 16.29 |
| Debug | 10 万 | 作る＋部品 3 つ | 640.4 | 470.4 | 516.1 |
| Debug | 10 万 | 部品を引く | 72.8 | 45.4 | 47.5 |
| Debug | 10 万 | update（1 フレーム） | 3.94 | 3.92 | 3.71 |
| Debug | 10 万 | 消す | 197.3 | 201.8 | 199.5 |
| Release | 1 万 | 作る＋部品 3 つ | 9.98 | 4.42 | 4.39 |
| Release | 1 万 | 部品を引く | 1.049 | 0.246 | 0.249 |
| Release | 1 万 | update（1 フレーム） | 0.166 | 0.169 | 0.174 |
| Release | 1 万 | 消す | 1.915 | 1.924 | 1.950 |
| Release | 10 万 | 作る＋部品 3 つ | 99.5 | 46.2 | 44.7 |
| Release | 10 万 | 部品を引く | 10.70 | 2.66 | 2.68 |
| Release | 10 万 | update（1 フレーム） | 2.14 | 2.06 | 2.01 |
| Release | 10 万 | 消す | 25.97 | 20.87 | 20.55 |

読み方:

- 1（型の番号）で、部品を引く手間が Release で約 4 分の 1（1 個 3 種類で 105 ns → 25 ns）、作る手間が約半分になった。2〜7 はベンチマークの範囲の外（本編の System の中）なので、この表では変わらない。
- **update は直していない**（構造の問題のまま）。Release で 1 個あたり約 20 ns（System 2 つ・中身はほぼ空）で、10 万個だと中身の無い System 2 つだけで 1 フレーム（16.7 ms）の約 12% を使う。個数が 10 倍で時間は約 12 倍と、個数より速く増える（ハッシュ表の節と散らばった部品をたどるキャッシュミス）。
- 消すのは Release で 1 個 約 200 ns（全 System に知らせる・全部の型の一覧から消す・delete）。
- Debug は Release の 2〜8 倍。Debug でも 1 万個の update は 0.25 ms で、遊べる範囲。

### 6.5 次にやること

**部品の持ち方の作り替えを、段階 3（物理の同期と当たりのイベント）の前に行う。** 段階 3 以降は位置の正を TransformComponent に移し、物理・当たり・行動の System が毎フレーム全 Entity を回すので、先に土台を速くしておく。

- 部品を型ごとの密な配列で持つ（Entity の番号 → 添字 の表と、添字 → Entity の番号の逆引き。消すときは末尾と入れ替える）。部品は値で持ち、1 つずつ new しない。
- System の対象は Entity の番号の密な配列にし、部品は型ごとの配列から添字で引く（または、いちばん数の少ない部品の配列を回して残りを引く）。
- 1 体ごとの仮想関数をやめ、System が自分の対象をまとめて回す（update() の中のループ）形を基本にする。今の update( ComponentTuple& ) は互換として残すか、使う所を置き換える。
- 指すときは今の Entity*・部品の生ポインタが配列の伸長で無効になるので、EntityId で持つ決まり（4 節）を徹底する。部品のポインタを持っている所（RenderTarget・System の tuple・ActiveObject）を洗い出す。
- 前後の比べ方は 6.4 のベンチマーク（同じテスト・同じ構成で 3 回の中央値）。

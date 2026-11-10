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
| **3 物理の同期と当たりのイベント**（山場） | 位置の正を TransformComponent に。userPointer を EntityId のタグに。当たりを集めるシステム → 互換の層が今の二重ディスパッチを呼ぶ。物体の性質をデータに | 振り分けの単体テスト。風船・梯子・メダル・ゴールのステージ。userPointer を読む所の直し漏れ。**完了（34cb8cbb・3f70cb95・fa636bda・7ae506c0）**。**タグ** : userPointer には「EntityId + 1」を入れる（core/physics/EntityTag.h。0 番の Entity とタグ無し = nullptr を区別する）。userPointer を読み書きするのは EntityTag.h だけになった（grep で確かめた）。直した所 : ActiveObjectPhysics の当たりの振り分け（on_collide）・ClosestNotMe と ClosestNotMeAndHim（レイ。Player の足場・Robot の視線）・Player の WithoutMeContactResultCallback（前の壁）と get_footing_height（足場の性質）・GameObject のデストラクタ（userPointer を消していた）・ECS の PhysicsSystem（RigidBodyComponent* を入れていた）・PhysicsManager::for_each_contact と BalloonSystem（RigidBodyComponent* をキャストし、当たり 1 つごとに対象を 1 つずつ探していた）。本編のタグは ActiveObject が剛体と Entity が揃ったとき（set_rigid_body・attach_entity）に付け、Entity を消すときに外す（外した剛体は以前と同じく地面の扱い）。複製（clone）は剛体を共有するだけなので付け替えない（以前も userPointer は元の物体のままだった）。タグはあるが Entity が消えた剛体は、当たりを起こさず、レイ・壁の判定では通り抜ける（以前なら消えた物を指すポインタだった所）。**当たり** : ecs::CollisionEventSystem が当たった組をタグで集め（ActiveObjectPhysics::collect_contacts_with・collect_all_contacts）、dispatch で渡す直前に 1 つずつ振り分ける（決まりは以前の on_collide と同じ。渡す直前に振り分けるので、先に渡した当たりで死んだ物体は後の当たりを起こさない = 以前と同じ）。互換の層は ActiveObjectManager（ecs::CollisionEventHandler）で、番号から ActiveObject* を ecs::ActiveObjectLinkComponent で引いて on_collide_with を呼ぶ。調べる順（プレイヤー → ゴール → 全部）は変えていない。集めてから渡すので、contactTest の途中で呼ばれた先が物体を動かしても、同じ contactTest の後の接点には効かなくなった（以前はその場で呼んでいた。ログ・画面では違いは見えない）。**性質** : ecs::CollisionTraitsComponent（is_ghost・is_hard・is_block・is_safe_footing・is_dead）。性質は派生の仮想関数がどれも定数を返すので加えるとき（attach_entity）に一度写す。is_dead は ActiveObject の is_dead_ と同じ値を保つ（書くのは kill・restart・加えるときだけ）。**位置の正** : GameObject::get_transform が、ActiveObject に Entity があるあいだ TransformComponent を指す（bind_transform。Entity を消す前に自分の Transform へ写して戻す）。位置は Player などあらゆる所が get_transform・get_location で読み書きするので、呼ぶ側を書き換えずに正を移すため。部品の場所は外すまで動かない（StablePool）のでポインタで指せる。Entity を外から消されると消えた部品を指すので、GameMain::setup_scene は EntityManager を片付ける前に物体を壊すようにした（CanvasTestScene などはスクリプトで作った物体を残したまま終わり、以前は Entity を失った物体が次のシーンへ残っていた）。段階 1 の一方向のコピー（sync_render_component）は位置を写さなくなった（描くかどうかは派生が仮想関数で決めるので、まだ写す）。ecs::PhysicsSyncSystem（TransformComponent と ecs::PhysicsBodyComponent が対象）が物理を進めた後に剛体の位置を TransformComponent へ写す（以前は GamePlayScene が全物体の update_transform を呼んでいた）。位置 → 剛体（commit_transform）は、位置を変えたその場で送る（set_location の直後にレイを飛ばす所などがあるので、まとめて送る形にはしなかった）。計算は PhysicsSyncSystem::write_body・read_body に移した。剛体の速度は、まだ ActiveObject を通して触る（MoveToTargetSystem の TargetMover）。本編の世界と ECS の世界はまだ別（決定 1 の統一は後の段階）。**確かめ** : テスト 392 → 413 件（EntityTagTest 5・CollisionEventSystemTest 9・PhysicsSyncSystemTest 4・RigidBodyTest 3）。スイッチと目標地点のログは 1-1（2・3・2）・2-1（40 秒で 13・39・38）・1-2（40 秒で 1・1・1）・0-1（2・1・0）・2-0（1・1・1）で移す前と同じ。画面は移す前と比べて 0-1・2-0・1-2・2-1・1-1 で画素の差の平均 0.12 以下（0-0・2-3 はゆらぎが物体のアドレスを乱数の種にするので起動ごとに変わる）。開発用の BLUE_SKY_DEBUG_VISIT_OBJECTS（プレイヤーをメダル・風船・梯子・ゴールの所へ順に置いて結果をログに出す）で、0-1・2-0 でメダルを取る・風船につかまる・梯子に触れる・ゴールに着くが起きた（0-0・1-0・1-3 も）。**残り** : MoveToTargetSystem::update_targets は目標を持たない物体も含め全対象を毎フレーム回す（1 万個で Debug 1.1 ms。目標を持つ物体だけの一覧にする）。 |
| **H2 剛体の持ち主を 1 つに**（3 と同時か直後） | ムーブだけできる RAII の剛体をコンポーネントが持つ。BulletPhysics・PhysicsManager の生の new を unique_ptr に | **完了（480d2ad8・fa636bda）**。core::physics::RigidBody をムーブだけできる剛体の持ち主にした（壊れるとき・ムーブで上書きされたときに世界から外し、剛体 → 動きの状態 →（持っていれば）形 の順に消す）。ECS 側 : RigidBodyHandle は RigidBody の別名で、RigidBodyComponent が持つ（部品が外れる・Entity が消えると剛体も消える。destroy_rigid_body は reset と同じ）。本編 : BulletPhysics::add_box_rigid_body などは持ち主を返し、ActiveObject::set_rigid_body が受け取って ecs::PhysicsBodyComponent に入れる（Entity が無い間 = 加える前・Entity を消した後は ActiveObject が預かる）。**今の持ち主（ActiveObjectPhysics）とのつなぎ方** : 物体の剛体の持ち主を物理の世界から物体の部品へ移し、ActiveObjectPhysics は「大きさごとに共有する形」と「地面・ステージの当たりの形の剛体」の持ち主として残した。理由 : 世界が持ったままだと、物体を壊しても剛体が世界に残り（タグを外すと地面として当たる）、剛体を差し替えると前の剛体が残る。部品が持てば Entity と剛体の寿命が一致し、段階 4 で ActiveObject を ECS だけの物体に置き換えるときもそのまま使える。形は大きさごとに多くの剛体が共有するので、剛体ごとには持たせない（PhysicsManager の剛体は剛体ごとに形を作るので、持ち主が形も持つ）。ActiveObjectPhysics::clear は物体の剛体を消さず世界から外すだけにした（以前は世界に残った剛体を全部消していた。今それをすると持ち主が後で二重に消す。本編のシーンは物体を先に壊すので残らない）。PhysicsManager の Impl は世界に残った剛体をもう消さない（持ち主が世界より先に壊れる決まり。GameMain・テストとも既にその順）。複製（clone）は以前と同じく剛体を共有し、持ち主を持たない（元の物体が先に壊れると複製は消えた剛体を指すが、物体を壊すのはシーンの終わりだけで、複製は剛体を触らずに壊れる）。BulletPhysics の世界の部品と PhysicsManager の Impl・世界の部品の生の new / delete は unique_ptr にした（宣言の逆の順に壊れ、以前の delete と同じ順）。 |
| **4 単純な種類を ECS だけに** | StaticObject・TranslationObject → Medal・Goal・Ladder・Rocket・Umbrella・Stone → Switch・AreaSwitch（dynamic_cast を SwitchComponent に）。ObjectFactory（種類名 → EntityId）。Lua の "GameObject" は EntityId を包む | 工場のテスト・スイッチのログ・各ステージの画面。**完了**（先に直したもの : ゆらぎの種を EntityId に 2acea460・MoveToTargetSystem は目標を持つ物体だけを回す e721902e。移した種類 : 動かない物と動く床 a9c8f7fe・メダル 421d2f60・傘 d6662315・ロケット e5e70543・ゴール 378f4e00。使っていない Missile を消した 329e5ad5）。**残り** : Ladder・Stone・Switch・AreaSwitch（下の「残りの理由」）。**作り** : ObjectFactory（blue_sky/ObjectFactory）が種類の名前から物体を作り EntityId を返す。ActiveObjectManager の「名前 → 生成関数」から移した種類（static・medal・umbrella・rocket・goal）と、GameMain::create_object・create_static_object・translation-object・名前の表（EntityId で持つ）をここへ集めた。ECS だけの物体の寿命も工場が持ち、clear は ActiveObjectManager::clear も呼ぶ（片付けの入口を 1 つに）。部品 : Transform・SimpleObject（やり直しの状態・向き・重力・やり直しの質量）・PhysicsBody・TargetMover（mover は nullptr = 部品を直接動かす）・CollisionTraits・ObjectRender に、種類ごとの StaticObject（印）・Translation・Pickup（拾う物の種類）・Spin（回る速さ）。ecs::EntityObject が以前の ActiveObject・GameObject と同じ決まりで部品を動かす（位置・速度・向き・やり直し・死亡・表示）。剛体の操作は ecs::object_body に出して GameObject もそれを呼ぶ。System は TranslationSystem・SpinSystem（ActiveObjectManager::update の物体の更新の後）。**Lua・ステージのコマンド** : GameObjectRef（EntityId を包む）を Lua の "GameObject" にし、ステージの game_object.* も名前で引いた GameObjectRef を操作する。ActiveObject を持つ物体はそのメンバ関数、持たない物体は EntityObject を呼ぶ。ECS だけの物体にできない操作（ゆらぎ・イベントの受け取り・複製）はログを残して何もしない。**互換の層をどう残したか** : 両方とも ActiveObject の組は今の二重ディスパッチのまま。片方だけ ActiveObject（プレイヤーなど）の組は、ECS だけの物体の部品の組み合わせで決める（ActiveObjectManager::on_collide_with_entity_object）: StaticObject の印なら相手の GameObject::on_collide_with_static_object（以前の on_collide_with( StaticObject* )）、Pickup なら相手の GameObject::on_pick_up( 種類 ) を呼び、true（拾った）なら拾われた物を死なせる（ロケットは当たりの応答も消す）。理由 : 相手側（Player の持ち物・メダル・ロケットを止める処理）はまだ ActiveObject の中にあり、段階 6 で Player を分けるまで動かせない。ECS だけの物体側の振る舞いはデータ（部品）にし、相手側だけ仮想関数を残せば、Player を ECS にするときに on_pick_up・on_collide_with_static_object を System に置き換えるだけで済む。呼ぶ回数は以前と同じ（1 組につき 1 回）。**確かめ** : テスト 413 → 435 件（MoveToTargetSystemTest 3・ObjectFactoryTest 19）。スイッチと目標地点のログは 1-1（2・3・2）・2-1（13・39・38）・1-2（1・1・1）・0-1（2・1・0）・2-0（1・1・1）で移す前と同じ。画面は 0-1・2-0・1-2・2-1・1-1 で移す前と差の平均 0.35 以下（ゆらぎを直したので 2-3 も起動ごとに同じになった）。BLUE_SKY_DEBUG_VISIT_OBJECTS は 0-1・2-0・0-0・1-0・1-3 で移す前と同じ結果、ロケットの所へ行く手順を足して 2-0・1-0・0-2 で 1 個になる。**残りの理由** : Ladder はプレイヤーが Ladder* を持ち（使ってはいない）、触れた物ごとに 0.5 秒の音の間引き（TimedCache< GameObject* >）を持つ。Stone はプレイヤーが Stone* の一覧を持って投げ（restart・set_location・set_velocity）、Robot・Balloon・BaseSwitch が on_collide_with( Stone* ) で石の位置・速度・is_moving_to を使う。Switch・AreaSwitch はイベントの受け取り手（ステージの event 行）と状態（入・切・壊れた）・触れた物の間引きを持ち、石との当たりもある。どれも相手側（Player・Robot・Balloon）が GameObject* として触るので、GameObjectRef か EntityId で受け取る形に相手側を書き換える必要があり、前半には入れなかった。**後半（残りの 4 種類）完了** : 先に開発用の BLUE_SKY_DEBUG_THROW_STONES（石を全部拾い、最初のスイッチ・全部の敵・最初の風船へ投げて結果をログに出す。ede57ac9 に入った。下の「確かめ」）を足し、相手側の受け取り方を書き換え（6812b8ae）、梯子（87e3bfe3）・石（7bb44c3d）・スイッチと空間スイッチ（0a4f8a3a）を ECS だけの物体にした。Ladder・Stone・BaseSwitch・Switch・AreaSwitch クラスを消した。**相手側の受け取り方** : 石は GameObject::on_collide_with_stone( const GameObjectRef& )（拾ったら true。Player だけ）、梯子は on_collide_with_ladder()（反応したら true。Player だけ。梯子そのものは渡さない。Player の ladder_ は読む所が無かったので消した）、スイッチは can_press_switch()（Player だけ true）。Player は持っている石を GameObjectRef の一覧で持ち、窓口で restart・set_location・set_velocity して投げる。Robot は石の窓口の is_moving_to（GameObjectRef に足した。計算は GameObject::is_moving_to の静的な形）で「こちらへ飛んでくるか」を見る。触れた物の間引きは Entity の番号をキーにした。**作り** : 梯子 = LadderComponent（印）＋ ContactCacheComponent（触れた物を EntityId で 0.5 秒覚える。ContactCacheSystem が ActiveObjectManager::update の中で減らし、やり直すと忘れる）。石 = ThrowableComponent（やり直すと Bullet の連続衝突判定を設定）＋ SpinComponent（degree_per_frame を足した。以前の Stone::update は経った時間によらず 1 フレームに 1 度）。スイッチ = SwitchComponent（状態・石で変わるか・アニメーションと音を鳴らすか・イベントの名前 → 受け取り手の一覧）＋ ContactCacheComponent、振る舞いは SwitchSystem（on_touch・do_switch・do_break・turn_on・turn_off・add_event_handler・get_state。音を鳴らすので ObjectFactory が SoundManager を受け取る）。空間スイッチは ObjectFactory::create_area_switch（すり抜ける・モデル無し・石では変わらない・音とアニメーション無し。ObjectSpec に is_ghost を足した）。やり直しの摩擦・回転のしやすさ（以前の Switch::restart）は SimpleObjectComponent の restart_friction・restart_angular_factor。ECS だけの物体の位置で音を鳴らす EntityObject::play_sound（以前の GameObject::play_sound と同じ値）。**互換の層** : ActiveObject × ECS だけの物体は on_collide_with_entity_object（相手の Entity の番号と SoundManager も受け取るようにした）で、石なら相手の on_collide_with_stone に窓口を渡して拾われたら石を死なせ、梯子なら相手が反応したら音と覚え、スイッチなら相手が押せる物体なら SwitchSystem::on_touch。ECS だけの物体どうし（以前は何もしなかった）に on_collide_entity_objects を足し、石とスイッチの組だけ石の速度で on_touch（以前の BaseSwitch::on_collide_with( Stone* )。1 組につき 1 回）。ステージの event 行は GameObjectRef::add_event_handler → SwitchSystem、game_object.action は "turn-on"・"turn-off"・"break" だけ受け付ける（イベントは呼ばない。以前と同じ。ほかのアクションは以前は黙って無視、今はログに残す）。BLUE_SKY_DEBUG_PRESS_SWITCHES は dynamic_cast< BaseSwitch* > ではなく、工場が作った物体を作った順に回して SwitchSystem で押す。**決めたこと** : ① 受け取る側の仮想関数は「型の多重定義」ではなく「部品の種類ごとの名前の付いた関数」にし、戻り値で相手側の結果（拾った・反応した）を返す（拾われた物を死なせる・梯子が音を鳴らすのは ECS 側の決まりにして、Player を ECS にするときに System へ置き換えやすくするため。前半の on_pick_up と同じ形）。② 触れた物の間引きは梯子とスイッチで同じ部品（ContactCacheComponent）にした（中身が同じ TimedCache で、減らす時点も同じ）。③ スイッチの状態を変える処理は部品ではなく System に置いた（音・アニメーション・イベントの呼び出しに SoundManager と他の部品が要り、互換の層・ステージのコマンド・開発用の確かめの 3 か所から呼ぶため）。④ 石の窓口は互換の層がその場で作る（描画の GraphicsManager は渡さない。プレイヤーは位置・速度・やり直ししか触らない）。**確かめ** : テスト 435 → 448 件（LadderStoneSwitchTest 13 件 : 梯子・石・スイッチ・空間スイッチの部品と性質・やり直し・当たり・覚え・イベントの順・壊れたら変わらない・ステージのアクション）。スイッチと目標地点のログは 1-1（2・3・2）・2-1（13・39・38）・1-2（1・1・1）・0-1（2・1・0）・2-0（1・1・1）で移す前と同じ（2-1 は動かし始めの座標の小数 2 桁目が 1 か所揺れた。以前から起動ごとに揺れる所）。BLUE_SKY_DEBUG_VISIT_OBJECTS は 0-1・2-0・0-0・1-0・1-3・0-2 で移す前と同じ。BLUE_SKY_DEBUG_THROW_STONES は 1-0・1-1・1-2・2-0・2-1 で移す前と同じ（石を全部拾う・1-2 の敵の 3・4・8 番目が止まる・2-0 のスイッチが入り風船が割れる・2-1 のスイッチが入りイベントで壊れる。1-2 のスイッチに当たるかは移す前も起動ごとに入・切が変わる）。空間スイッチのある 0-3・1-0・1-3 はスイッチを全部押して落ちない（移す前の基準は取っていない）。画面は押す確かめの 60・120 フレーム目で移す前と差の平均 0.002 以下（2-1 の 120 フレーム目だけ 1.4。影の色を変えるイベントの進み具合の違いで、絵は同じ）。**残り** : 石を投げる・石がスイッチや敵に当たる確かめは開発用の手段で、手で遊んでは確かめていない。物理は実時間の刻みで進むので、石が当たるかは起動ごとに揺れることがある |
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

**部品の持ち方の作り替えを、段階 3（物理の同期と当たりのイベント）の前に行う。** → **完了**（6.6）。 段階 3 以降は位置の正を TransformComponent に移し、物理・当たり・行動の System が毎フレーム全 Entity を回すので、先に土台を速くしておく。

- 部品を型ごとの密な配列で持つ（Entity の番号 → 添字 の表と、添字 → Entity の番号の逆引き。消すときは末尾と入れ替える）。部品は値で持ち、1 つずつ new しない。
- System の対象は Entity の番号の密な配列にし、部品は型ごとの配列から添字で引く（または、いちばん数の少ない部品の配列を回して残りを引く）。
- 1 体ごとの仮想関数をやめ、System が自分の対象をまとめて回す（update() の中のループ）形を基本にする。今の update( ComponentTuple& ) は互換として残すか、使う所を置き換える。
- 指すときは今の Entity*・部品の生ポインタが配列の伸長で無効になるので、EntityId で持つ決まり（4 節）を徹底する。部品のポインタを持っている所（RenderTarget・System の tuple・ActiveObject）を洗い出す。
- 前後の比べ方は 6.4 のベンチマーク（同じテスト・同じ構成で 3 回の中央値）。

### 6.6 部品の持ち方の作り替え（完了、2026-11-10）

| 段階 | 中身 | コミット | 足したテスト |
|---|---|---|---|
| ① | 置き場のクラス StablePool< T >（値をページに詰めて置く。場所は消すまで動かない）と、EntityId → 添字の表 SparseIndex（番号を 1024 個ごとのページに分けて配列で直に引く。空いたページは放す）。引き方を決める計測 | 72bb2ae5 | StablePoolTest・SparseIndexTest 8 件、EcsBenchmark.DISABLED_SparseIndexVsUnorderedMap |
| ② | EntityManager が部品を型ごとの置き場 ComponentStorage< T >（中身は StablePool）に値で持ち、Entity も StablePool に置く（API はそのまま） | 3bc263d2 | ComponentPointersStayValidWhenOtherEntitiesComeAndGo |
| ③ | System の対象の一覧を unordered_map から vector< pair< EntityId, tuple< T*... > > >＋SparseIndex に。外すときは末尾と入れ替える | d2542cb2 | SystemListStaysCorrectAfterSwapRemove |
| ④ | 1 体ごとの仮想関数 update( ComponentTuple& ) をやめ、update() を System ごとに一度だけ呼ぶ。中は for_each（型の分かったループ）で、1 体ぶんは非仮想の update_entity | 1c3ab399 | （既存のテストを新しい形に替えた） |

テストは 375 件 → 385 件（ほかに無効 3 件 = ベンチマーク）。

#### 決めたこと

- **sparse set の形（型ごとの置き場＋EntityId から引く表）にした。archetype にはしなかった。** 今の API（add_component・remove_component・get_component、System< T... > が部品の組を集めて付け外しの通知を受ける）をそのまま残せ、作り替えが小さいため。archetype は部品を付け外しするたびに Entity の全部品を別の表へ移すので、目標地点の部品（着いたら外す）のように付け外しの多い使い方と、下のポインタの安定性に合わない。
- **置き場の部品の場所は、外すまで動かさない（StablePool）。** 指示の推奨は「消すときに末尾と入れ替える密な配列」だったが、洗い出すと部品のポインタを持ち続ける所が多かった:
  - PhysicsSystem が剛体の userPointer に RigidBodyComponent* を入れ、BalloonSystem・PhysicsManager::for_each_contact が当たりからそれを読む（Bullet の中にあるので、場所が動いても知らせられない）
  - GraphicsManager の main_camera_transform_component_・main_camera_component_（シーンが set_main_camera_info で渡す）
  - シーンのメンバ（EnemyAiTestScene・MusicGamePrototypeScene・PhysicsTestScene・TransformTestScene・GeometryShaderCanvasTestScene の player_transform_・camera_component_ など）
  - System の対象の組（tuple< T*... >）・ObjectRenderDataSystem の RenderTarget（render・transform・animation）
  - ActiveObject は部品のポインタを持たない（EntityId で持ち、使うたびに引く。段階 1・2 の決まりどおり）ことを確かめた

  場所が動く置き方では、これらのどれかを直し漏らすと「ほかの Entity を消したときにだけ」壊れ、見つけにくい。そこで部品は約 16 KB のページに詰めて置き、伸びるときもページを動かさず、消した場所は空きとして次に置くときに使い回す（以前の 1 つずつ new と同じ決まりのまま、確保をやめて連続に置く）。空きができても、System が回すのは置き場ではなく自分の対象の一覧（密な配列）なので、走査の速さにはほとんど効かない。ポインタを毎回引く・添字で持つ・動いたら知らせる、のどれにも作り替えずに済んだ。
- **EntityId → 添字は SparseIndex（ページに分けた配列）。** 1 万・10 万個で unordered_map< EntityId, uint32_t > と比べた（3 回の中央値・1 個あたり、10 万個）: Release で 足す 1.7 / 41 ns、作った順に引く 0.4 / 2.8 ns、ばらばらに引く 0.4 / 6.1 ns、消す 0.9 / 23 ns（SparseIndex / unordered_map）。Debug は 引く 7.9 / 99 ns。EntityId は二度と使わない通し番号なので、生きている番号は最近の範囲に固まり、空いたページを放せば使う量も増え続けない（ページへのポインタの一覧だけが番号 1024 個につき 8 バイト残る）。なお EntityId は uint_t（32 bit）で、約 43 億個で一周する（今は考えない）。
- **System の走査** : 対象の一覧（番号と部品のポインタの組の密な配列）をそのまま回す。部品のポインタは動かないので、回すときに引き直さない（「一番小さい置き場を回してほかを引く」形は、付け外しの通知・on_remove_entity・debug_entity_count の決まりを変えずに済むこちらにした）。対象の一覧を回している間に、その System の対象を増減してはいけない（以前の unordered_map でも同じだった）。
- **仮想関数** : EntityManager が System ごとに update() を一度だけ呼ぶ（仮想関数）。中は for_each( f ) で回し、f は型が分かるのでインライン化できる。
- **順番** : System の対象の一覧の並びは決まっていない（末尾と入れ替えるため）。作った順が要る ObjectRenderDataSystem・MoveToTargetSystem は、以前から「対象が増減したときだけ番号の順に並べ直す」ので変えていない（その後、並べ直しもやめた。6.6 の「確かめたこと・残り」）。
- **部品の基底クラス** : Component の仮想デストラクタは残した（部品は値で置き、壊すときは型の分かった std::destroy_at なので、仮想の呼び出しにはならない。部品 1 つにつき vptr の 8 バイトが残る）。add_component に「Component を継承すること」の static_assert を足した。unique_ptr を持つ部品（ObjectRenderComponent・AnimationComponent）・粒子の一覧が 2.4 MB ある ParticleSystemComponent（1 ページに 1 つ）も、作った場所から動かさないのでムーブすら要らない。

#### 計測の前後

6.4 と同じベンチマーク（blue-sky-test.exe --gtest_also_run_disabled_tests --gtest_filter=EcsBenchmark.DISABLED_Entities*）を 3 回走らせた中央値（ミリ秒）。x64・開発機。作り替える前は 96b12785（6.4 の「全部の後」と同じ中身。同じ日に取り直した）。

| 構成 | 個数 | 項目 | 前（96b12785） | ②（3bc263d2） | ③（d2542cb2） | ④（1c3ab399） | 前 → ④ |
|---|---|---|---|---|---|---|---|
| Debug | 1 万 | 作る＋部品 3 つ | 48.96 | 15.87 | 9.332 | 8.713 | 5.6 倍速い |
| Debug | 1 万 | 部品を引く | 4.204 | 0.562 | 0.583 | 0.573 | 7.3 倍 |
| Debug | 1 万 | update（1 フレーム） | 0.257 | 0.221 | 0.077 | 0.065 | 4.0 倍 |
| Debug | 1 万 | 消す | 17.43 | 9.166 | 3.606 | 3.848 | 4.5 倍 |
| Debug | 10 万 | 作る＋部品 3 つ | 506.1 | 147.5 | 91.07 | 86.80 | 5.8 倍 |
| Debug | 10 万 | 部品を引く | 54.86 | 5.669 | 5.825 | 5.644 | 9.7 倍 |
| Debug | 10 万 | update（1 フレーム） | 4.311 | 2.533 | 0.786 | 0.650 | 6.6 倍 |
| Debug | 10 万 | 消す | 208.0 | 108.6 | 36.01 | 40.19 | 5.2 倍 |
| Release | 1 万 | 作る＋部品 3 つ | 4.737 | 2.130 | 1.296 | 1.300 | 3.6 倍 |
| Release | 1 万 | 部品を引く | 0.263 | 0.070 | 0.070 | 0.070 | 3.8 倍 |
| Release | 1 万 | update（1 フレーム） | 0.179 | 0.125 | 0.024 | 0.014 | 12.8 倍 |
| Release | 1 万 | 消す | 2.100 | 0.949 | 0.306 | 0.268 | 7.8 倍 |
| Release | 10 万 | 作る＋部品 3 つ | 48.52 | 21.22 | 11.86 | 11.59 | 4.2 倍 |
| Release | 10 万 | 部品を引く | 3.413 | 0.723 | 0.747 | 0.763 | 4.5 倍 |
| Release | 10 万 | update（1 フレーム） | 2.220 | 1.594 | 0.253 | 0.165 | 13.5 倍 |
| Release | 10 万 | 消す | 32.35 | 10.12 | 2.981 | 2.987 | 10.8 倍 |

読み方:

- **Release の 10 万個の update は 2.22 ms → 0.165 ms**（1 個あたり 22 ns → 1.7 ns。1 フレーム 16.7 ms の 13% → 1%）。個数が 10 倍で時間も約 10〜12 倍と、ほぼ個数に比例するようになった。
- 部品を引くのは ② から変わらない（③④ は引く所を触っていない。1 個 3 種類で 7.6 ns = SparseIndex を 3 回引く）。Debug の 10 万個の消すが ③ → ④ で 36 → 40 になったのは揺れと見ている（④ は消す所を触っていない）。
- 残る手間 : 作るのは 1 個あたり約 116 ns（部品を付けるたびに全 System に知らせ、System が部品の組を集め直す）。消すのは約 30 ns（全 System に知らせ、全部の型の置き場から消す）。1 万を超えても 1 フレームで作る・消す数が数百なら問題にならない。

#### 確かめたこと・残り

- 起動して確かめた（Debug）: ステージ 1-1（スイッチ 2・動かします 3・着きました 2）、2-1（40 秒で 13・39・38）は基準と同じ。1-2（40 秒で 1・1・1。比べる基準の数は無い）・enemy_ai_test・physics_test・particle_system_test・music_game_prototype も落ちず、crash.log も出ない。1-2 と enemy_ai_test のフレーム 60 は作り替える前に撮った画面と同じ。
- 残りだった「ObjectRenderDataSystem・MoveToTargetSystem の一覧を、対象が 1 つ増減するたびに全部作り直して並べ替える」は直した（9c9989ee）。core::ecs::EntityIdOrderedList（足すときはたいてい末尾に足すだけ、外すときは印を付けて次に読むときにまとめて詰める、引くのは二分探索）。そのために System::on_remove_entity に外れる Entity の番号を、BaseSystem::on_watched_component_changed に付いたか外れるかを渡すようにした。ベンチマーク EcsBenchmark.DISABLED_OrderedListAddRemove（Debug x64、200 フレームの平均）: 1 万個で描く対象の一覧の「1 つ足す」0.838 → 0.000 ms、「1 つ消す」1.385 → 0.050 ms、10 万個で 9.17 → 0.0001 ms・14.9 → 0.49 ms。

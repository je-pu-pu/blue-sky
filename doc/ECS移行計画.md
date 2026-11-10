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
- 足りない : アニメーションとボーン行列、本編の描画（影・手描きの線・速度パス）、本編の物理の世界への対応（→ 3 章の「7」で PhysicsManager に統一した）、当たりのイベント、目標地点への移動、ステージからの生成と名前で引く仕組み、Lua から Entity を指す方法、プレイヤーの入力、物体の性質（すり抜け・硬さ・足場）のデータ。
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
| **5 Balloon・Girl・Robot** | Robot は Patrol・Perception を本編の物理で動くように広げ、状態ごとのテクスチャは Handle<Texture> | **完了**（風船 a1b5327c・少女 3b626c62・敵 da5e50cd）。Balloon・Girl・Robot クラスを消し、ActiveObject の種類は Player だけになった（ActiveObjectManager の「名前 → 作る関数」の表は空）。**風船** : BalloonHoldComponent（つかんでいる物体の EntityId と窓口）・BalloonHoldSystem（つかまれていなければ始めの位置でゆらぎ 1 フレームに 1 度回る、つかまれたら手元に置き、相手が別の風船を持つか動作が終わると割れる、石で割れる）。ECS のテストシーンの BalloonComponent・BalloonSystem は使わなかった（ECS の物理の世界で「触れた物に上向きの力を与える」別の振る舞い。MusicGamePrototypeScene のものとして残す）。点滅は描画の部品の is_hidden（描くかどうかの値を変えずに今だけ隠す。should_draw_mesh など）。死ぬと (0, -100, 0) へ退くのは EntityObject::kill（スクリプトの kill も同じ）。**少女** : GirlComponent（立つ／浮く・浮く中心）・GirlSystem（浮いている間は浮く中心へ上下の速度）。着いたら浮く中心になるのは EntityObject::notify_arrive_at_target_location。StageBuilder の結果の Girl* と GamePlayScene::girl_ は使う所が無く消した。**敵** : 決定 2 のとおり行動を RobotBehaviorComponent（状態・前の状態・タイマー・見た目・見た目ごとの TextureHandle・追う相手）にまとめ、RobotBehaviorSystem が以前の Robot と同じ値・順で動かす。巡回の地点だけ敵 AI のテストシーンの PatrolComponent に寄せた（速さ 0.1。以前から次の地点へ進まない）。Perception・Alert・PatrolSystem は寄せなかった。理由 : 本編の敵は距離 3・10・20 m の 3 段で視野の広さが変わり（近くは真後ろ寄りも見える）目の高さ 1.5 m で、PerceptionComponent の視界錐（距離 1 つ・水平の半画角・上の制限）では同じ判定にならない。遮るもののレイは本編の物理の世界（ActiveObjectPhysics の ClosestNotMeAndHim）で飛ばす。警戒は「見えたら 3 秒注目 → 追う → 10 秒で諦める」の時間で決まる状態で、AlertComponent の発見度とは振る舞いが違う。PatrolSystem は Transform を直接動かすが、本編の敵は剛体の速度で動く（重力・壁）。物理の世界を 1 つにする（決定 1）ときに寄せ直す。モデルは敵どうしで共有しているので、見た目のテクスチャは最後に動かした敵のものになる（以前と同じ。直すなら make_model_unique）。**決めたこと** : ① プレイヤー（まだ ActiveObject）の状態を ECS 側が読む所は、段階 2 の TargetMover と同じく窓口（interface）をプレイヤーが実装する : ecs::BalloonHolder（手元の位置・風船の動作中か・終わりかけか）と ecs::ChaseTarget（位置・速度・前・剛体）。部品には窓口と相手の EntityId を持ち、相手が生きている間だけ使う。段階 6 で部品から読む形に替える。テストは偽物で剛体なしに確かめられる。② 当たりの互換の層は段階 4 と同じ「部品の種類ごとの名前の付いた仮想関数」: on_collide_with_balloon（つかんだら窓口を返す。つかまれた風船には呼ばない）・on_collide_with_robot( 追っているか )（プレイヤーは傷つき、true なら敵は立つ・回る間なら止まる。2 つは別の状態でだけ起きるので呼ぶ順によらない）。石 × 風船・敵は ECS だけの物体どうし（on_collide_entity_objects。1 組につき 1 回）。③ ゆらぎ・点滅の計算を ecs::flicker に出し、ゆらぎの大きさは FlickerComponent（少女・敵。やり直すと 1）。種は Entity の番号のまま。④ 風船・敵の System は作った順（Entity の番号の順）に回す（以前の物体の一覧の順と同じ。音の 3D の位置が順で変わり得るため）。⑤ 少女の「いつも描く対象（is_visible が常に true）」は移さなかった（set_visible で隠した後の定数の書き込みだけの違いで、画面は変わらない）。開発用 BLUE_SKY_DEBUG_LOG_ACTORS（30 フレームごとに風船・少女・敵の位置・向き・状態をログに出す）を足した。**確かめ** : テスト 461 → 481 件（BalloonGirlRobotTest 20 件）。移す前に撮った基準と比べ、0-0・0-1・0-2・2-0・2-1・2-3・1-2・ext-0・ext-1 の作った物体の一覧・様子のログ（始めの位置・向きの進み・状態の移り変わり・動く幅）が同じ、スイッチと目標地点のログ（1-1 は 2・3・2、2-1 は 13・39・38、1-2 は 1・1・1、0-1 は 2・1・0、2-0 は 1・1・1。2-1 の動かし始めの座標の小数 2 桁目は以前から揺れる）・物体巡り（0-1・2-0・0-0・1-0・1-3・0-2）・石投げ（1-0・1-1・1-2・2-0・2-1。1-2 の敵の 3・4・8 番目が止まるのを 2 回）が同じ。画面は風船・少女・敵のいるステージで差の平均 0.1 以下（0-0 は手描きの揺れで移す前どうしでも同じだけ変わる。2-1 のスイッチを押す確かめはカメラの高さが少し違う、以前から揺れる所）。1-2 で石を投げる確かめと一緒に様子を出し、止まった敵が倒れるアニメーションの 60 フレーム目から当たりの応答が無くなって沈むのを見た（以前と同じ振る舞い）。2-3 で敵と少女が浮いて上下し、1-2 で敵が 1 フレームに 0.25 度回るのを見た。**確かめられなかったこと** : 敵が相手を見つけて注目 → 追う・プレイヤーが傷つくは、遊ばずに起こせず単体テストだけで確かめた。巡回（PATROL）はステージで使っていない。 |
| **6 Player** | 入力（GamePlayScene.cpp:371-511）を PlayerInputSystem へ。移動・梯子・持ち物・風船・視点を分ける。Ladder*・Balloon* は EntityId で持つ | 手で遊んで確かめる時間を一番多く取る。**完了**（開発用の確かめ e09cadd9・プレイヤーを ECS へ e2452eb1・ActiveObject などを消す 36758535・持ち物を切り出す 3b1c5a4d）。**確かめの手段** : 先に入力を値（ecs::PlayerInputComponent）にし、開発用 BLUE_SKY_DEBUG_PLAYER_SCRIPT（Input を読まずに決まった入力の並びを流す : 歩く → 跳ぶ・よじ登る → 向きを変えて歩く → 横・後ろ歩き → 見下ろす・見上げる → 走る → 跳ぶ → 望遠鏡 → 梯子を上って跳び降りる → 風船 → ロケット → 落ちて死ぬ → A でやり直す。5 フレームごとにプレイヤーの様子をログに出す）を足して、移す前の基準を撮った。BLUE_SKY_DEBUG_LOG_ACTORS にもプレイヤーを足した。確かめはすべて BLUE_SKY_BACKGROUND=1（25a4df1b。窓を前面に出さず入力を受け取らない）で、移す前の exe は 25a4df1b から作った。**作り** : プレイヤーは ECS だけの物体（ObjectFactory::create_player。剛体はステージを読むときに add_player_rigid_body）。部品はほかの物体と同じもの（Transform・SimpleObject・TargetMover・CollisionTraits・ObjectRender）と、ecs::PlayerBehaviorComponent（行動をまとめて持つ部品。以前の Player のメンバ変数と定数）・PlayerInventoryComponent（持ち物）・PlayerInputComponent（入力）・FlickerComponent。振る舞いは PlayerBehaviorSystem（以前の Player のメンバ関数を同じ値・同じ順で）と PlayerInventorySystem（拾う・選ぶ・使う）。入力は PlayerInputSystem（以前の update_main のプレイヤーの所。シーンはカメラの様子 View を渡し、結果 Result（望遠鏡・視野・やり直し）でカメラを動かす。カメラの動きはプレイヤーの操作の結果を変えないので、操作の後にまとめた）。カメラは Player* ではなく値で動く（Camera::update_with_player）。やり直し・死（スクリプトの restart・kill も）は EntityObject がプレイヤーの System を呼ぶ。**窓口を部品から読む形に** : 風船は BalloonHoldComponent::holder_id のプレイヤーの部品（持っている風船・動作・終わりかけ・手元 = 位置 + 前 0.5 - 右 0.25 + 上 1.75）を、敵は RobotBehaviorComponent::target_id の相手の部品（位置・速度・前・剛体）を読む。BalloonHolder・ChaseTarget を消した。当たりは ecs::contact_rules（部品の組み合わせ）がプレイヤーの System の on_touch_* と持ち物の System を呼ぶ（以前の GameObject の仮想関数 on_pick_up・on_collide_with_balloon などと同じ順・同じ回数）。**ActiveObject・ActiveObjectManager・GameObject を消した**（下の「消せる条件」）。ActiveObjectManager の残す役目は 2 つに分けた : ObjectUpdater（物体の System を以前と同じ順に動かす・剛体の位置の同期・当たりを集めて渡す）と ecs::contact_rules（当たりの決まり）。ObjectFactory はやり直しで目標地点・向きを捨てる（以前の ActiveObjectManager::restart）。GameObject::GravityDefault・is_moving_to は ecs::object_body へ。Camera は GameObject を継承しない（使っていなかった）。Lua の clone_object は複製できる種類が無くなったのでログを残して nil。**切り出したもの・残したもの** : 持ち物（数・選ぶ・石の一覧・メダル）を切り出した（ほかの状態とほとんど関わらず、ロケット・石を使う時点と走れるかだけでつながる）。残したもの（PlayerBehaviorComponent に入ったまま）と理由 : ① 移動・ジャンプ・よじ登り : 足場・壁・よじ登り・のぞき込み・投げられるかを調べるレイと箱が毎フレームの更新の頭にまとまっていて、その結果（is_on_footing など）を梯子・風船・ロケット・視点が同じフレームで読む。分けると更新の順を System の間で守る決まりが要る。② 梯子 : is_on_ladder は当たりで立ち、跳べるか・重力・速さの減り方・入力（梯子では上り下りだけ）・目の奥行きが読み、更新の最後に消す。読む所が 5 か所に散っていて、分けても部品の読み書きが増えるだけ。③ 風船・ロケット : 動作のモード（action_mode）を望遠鏡と共有する 1 つの状態機械で、動作の始め・終わりで跳ぶ・重力・落ちる音・足場の高さが変わる。分けるなら動作のモードを先に「動作の状態」の部品にする必要がある。④ 視点（上下の向き・目の高さ・奥行き）: のぞき込めるかの箱（行動の更新の中）が目の高さと奥行きを使い、死ぬと目の高さが下がる。小さいが行動の更新の順に入り込んでいる。いずれも振る舞いを変えずに分けられるが、分ける効果（ほかから独立に使える・テストしやすくなる）が小さく、更新の順の取り違えの危険の方が大きいと判断した。次に分けるなら ③ の動作の状態から。**決めたこと** : ① 入力の部品は生の入力（ボタン・マウス・ホイール）を持ち、カメラの角度などは View で渡す（開発用の並びもテストも同じ入力を書ける）。② ホイールは以前と同じくプレイヤーが生きている間だけ取り出す。③ Player::super_jump・on_collide_with( DynamicObject* )（使う所・定義が無い）は移さなかった。④ 作った物体の一覧（BLUE_SKY_DEBUG_DUMP_STAGE）のプレイヤーの種類の名前は "class blue_sky::Player" → "player" に変わる（ほかは同じ）。⑤ ActiveObjectPhysics は残した（下の「消せる条件」）。**確かめ** : テスト 481 → 494 件（PlayerTest 13 件。BalloonGirlRobotTest・ObjectFactoryTest・LadderStoneSwitchTest の偽の物体・窓口は ECS のプレイヤー・敵に替えた）。移す前（25a4df1b）と、プレイヤーの入力の並び（2-0・1-1・0-1・1-2・2-1）・スイッチと目標地点（1-1 は 2・3・2、0-1 は 2・1・0、2-0 は 1・1・1、2-1 は 13・40・39、1-2 は 1・1・1）・物体巡り（0-1・2-0・0-0・1-0・1-3・0-2）・石投げ（1-0・1-1・1-2・2-0・2-1）・作った物体の一覧・風船・少女・敵の様子が、同じ版を 2 回動かしたときの揺れの範囲で同じ（揺れる所 : 2-1 の動く物の数・1-2 で石がスイッチに当たるか・0-1 で落ちて死ぬ時点が 1 フレーム・テレポートした直後のカメラ）。画面は手描きの線の揺れの範囲。**確かめられなかったこと** : 手で遊んではいない（決まった入力の並びと開発用の手段だけ）。敵に追われて傷つく・押し返されるは単体テストだけ。HMD（Oculus）の向きの入力は確かめていない。 |
| **7 物理の世界の統一**（決定 1） | 本編の世界（ActiveObjectPhysics・BulletPhysics）の機能を core::physics::PhysicsManager に移し、本編と ECS のテストシーンが同じ窓口を使う。Bullet の型を core/physics の外に出さない | **完了（2026-11-11。fb58ec0e・42903a1f・de0e9039）**。**2 つの世界の違いと合わせ方** : ① 形 : 本編は同じ種類・大きさの形を共有、ECS は剛体ごと → RigidBodyCreateInfo::share_shape（本編だけ true。形の持ち主は PhysicsManager、clear で消える）。② 摩擦 : 本編は Bullet の既定 0.5、ECS は 0.1 → friction（本編は BulletDefaultFriction）。③ 振り分け : 本編は Bullet の既定（動く物は全部と、動かない物は動く物とだけ当たる）、ECS はグループとマスク → use_default_collision_filter（本編だけ true）。④ 剛体の中心のずれ : 本編は足元から中心まで右から掛ける → offset。⑤ OBJ の当たりの形・地面 : 本編の世界だけ → PhysicsManager::load_obj・add_ground（同じ読み方・同じ箱。世界が持ち clear で消える）。⑥ レイ : 本編は ClosestNotMe・ClosestNotMeAndHim（自分・相手・死んだ物・すり抜ける物・消えた物・足場に含めない柔らかい物を通り抜ける）、ECS は ray_test_excluding（グループで除く）→ ray_test_closest（自分と相手の 2 つを除き、残りはタグを受け取る関数で選ぶ。決まりは blue_sky の CollisionEventSystem::is_ray_target）。⑦ 接触 : 本編は contactTest・contactPairTest・めり込んだ接点の走査・世界に入れない箱、ECS は接点の組をグループで走査（for_each_contact）→ contact_test・contact_pair_test・for_each_penetrating_contact・box_contact_test を足した（結果は Bullet の剛体ではなく Entity の番号のタグ。壁の決まりは CollisionEventSystem::is_block）。⑧ 重力（-9.8）・刻み（stepSimulation( 経過時間, 5, 1/120 ) とデバッグ描画）・clear の順は同じだった。⑨ 剛体の操作 : 本編は btRigidBody を直接触っていた → core::physics::RigidBody のメンバ関数（速度・位置・質量・重力・摩擦・回転・当たりの応答・連続衝突判定・タグ）。**決めたこと** : ① 世界は 1 つ（GameMain が持ち、シーンを替えるときに物体と Entity を片付けた後で clear）。理由 : ObjectFactory・ObjectUpdater・GameObjectContext がアプリの間ずっと同じ世界を指す作りで、以前の本編の世界も ECS の世界も 1 つがアプリの間生きていた。シーンごとに作ると組み立て直しが要る。clear を setup_scene でも呼ぶので、DebugScene・ParticleSystemTestScene が足した地面が次のシーンに残らなくなった（以前の本編の世界は GamePlayScene だけが片付けていた）。② 値は変えない（本編の剛体は以前の BulletPhysics と同じ値・同じ掛け算。ECS の既定も以前のまま）。③ PhysicsBodyComponent は持ち主だけを持つ（複製のための生の btRigidBody* は、複製できる種類が無くなったので消した）。④ RigidBody::get() は private（friend の PhysicsManager だけ）。⑤ 敵の Perception・Alert・PatrolSystem への寄せはしなかった（世界は 1 つになったが、見え方・警戒・動かし方の違いは「5」の行の理由のまま）。**Bullet の型が core/physics の外に残る所** : core/math の Vector・Quaternion・Transform の変換（Bullet と同じメモリの形を使う）と core/graphics の BulletDebugDraw（btIDebugDraw を実装する）、そのテスト。blue_sky・Scene・GameObject・core/ecs には無い（grep）。**確かめ** : テスト 494 → 508 件（PhysicsManagerTest 13・RigidBodyTest 2・CollisionEventSystemTest 1 を足し、BulletPhysics の 2 件を PhysicsManagerTest の同じ確かめに置き換えて消した）。移す前（463d61d2）の exe を 2 回・後を 1 回動かし、プレイヤーの入力の並び（2-0・1-1・0-1・1-2・2-1）・スイッチと目標地点・物体巡り（6 ステージ）・石投げ（5 ステージ）・風船/少女/敵の様子・作った物体の一覧・画面を比べた。一覧は全部同じ、物体巡りの結果も同じ、様子の数の差は 0.04 以下、スイッチは 1-1 が 2・3・2、0-1 が 2・1・0、2-0 が 1・1・1、1-2 が 1・1・1（2-1 は移す前の 2 回が 13・40・39 と 13・39・38、後が 13・40・39）。違いは移す前どうしでも出る所だけ（2-1 の動く物の数・1-2 で石がスイッチに当たるか・0-1 で死んだ後に弾む様子と前の壁の旗・押す確かめの 60 フレーム目のカメラ）。physics_test・enemy_ai_test・music_game_prototype・particle_system_test・debug も起動して落ちず、物理の漏れのログも出ない。**確かめられなかったこと** : 手で遊んではいない。ECS のテストシーンは起動して落ちないことと画面を撮っただけ（操作はしていない）。シーンを続けて替えての確かめ（clear の後に次のシーンが正しく動くか）は、起動ごとに 1 シーンなので実機ではしていない |

**ActiveObject を消せる条件** : 全ステージで ActiveObject が 0・ActiveObject.h の include が 0・userPointer がすべて EntityId のタグ・GraphicsManager・Lua・GameObjectContext が ActiveObject を知らない・全ステージの画面とスイッチ・ゴールのログが移す前と一致。→ **満たして消した**（段階 6、36758535）: ActiveObject の種類は段階 6 でプレイヤーも ECS だけの物体にして 0。ActiveObject.h・GameObject.h・ActiveObjectManager.h の include は 0（grep）。userPointer を読み書きするのは core/physics/EntityTag.h だけ（と PhysicsManager の解放時の消去）。GraphicsManager（段階 1 から）・Lua の "GameObject"（GameObjectRef。互換の分岐も消した）・GameObjectContext（物理の世界の型を前方宣言するだけ）は ActiveObject を知らない。確かめのログ・画面は 3 章の「6」の行。消したもの : ActiveObject・GameObject・ActiveObjectManager・ActiveObjectLinkComponent・Player・BalloonHolder・ChaseTarget。**残したもの : ActiveObjectPhysics**（本編の物理の世界。名前は以前のまま。→ 3 章の「7」で PhysicsManager に統一して消した）。理由 : 決定 1 の「core::physics::PhysicsManager に統一」は、OBJ の当たりの形の読み込み・地面・自分を除くレイ判定を PhysicsManager へ移すのが先で、別の作業（todo の「物理の世界を統一する」）。ActiveObject を受け取る関数（add_active_object_as_*・is_collision・collect_contacts_with）は消し、ActiveObject は知らない。

## 4. ハンドルにするもの・unique_ptr にするもの

- **ハンドル** : 共有され、差し替え・解放があり、多くの所から指されるもの（Model・Mesh・Texture・Shader・Sound）。
- **unique_ptr／RAII** : 持ち主が 1 つで寿命が同じもの（剛体・定数バッファ・AnimationPlayer・物理の世界）。
- **Entity への参照** : EntityId。番号を二度と使わない（EntityManager.h:45、clear でも戻さない）ので、is_alive で確かめれば世代は要らない。Entity* はシーンの中の一時的な使用に限る。
- 順番 : H1 を先か 0 と並行に（段階 1 で生まれる ObjectRenderComponent が最初から Handle<Model> を持つように）。テクスチャとシェーダーのハンドル化（シェーダーが持つ Texture* など）は範囲が広いので別の流れで後から。

### 4.1 結果 : 残っていた生ポインタのメンバの整理（2026-11-11）

2026-11-11 に `^\s+(const\s+)?(…::)?(Texture|Model|Mesh|Shader|Sound)\s*\*\s*\w+_\s*(=|;|\[)` で数えた 52 件と、探す途中で見つかったもの（容器・名前に _ の無いメンバ・ファイルの外の変数）を分けた。

- **ハンドルにした（44 件）**
  - シェーダーが持つテクスチャ 10 件（Flat・Flat2D・Lit・LitInstanced・Matcap・TessellationMatcap・SkyBox・Ground・DebugShadowMapTexture・post_effect の Default）と、Shader< … > の `std::vector< Texture* >` : `graphics::TextureSlot`（GraphicsManager が管理するテクスチャはハンドル、管理していない描画先などはポインタ）。描くたびに引き、設定したテクスチャが解放されていれば描かない（シェーダーごとに 1 回ログ）。a0e274c8
  - RenderSystem のシェーダー 3 件・NeuralNprEffect のシェーダー 2 件 : `core::graphics::ShaderHandle`（GraphicsManager の get_shader_handle・resolve_shader を足した）。81283ce6
  - 見つかったもの : core の `ModelComponent::model`（ModelHandle に。RenderSystem はハンドルの番号ごとに集めて描くときに引く）・PhysicsTestScene のファイルの外の `Model* models[]`（シーンのメンバのハンドルに）。81283ce6
  - タイトル 7 件（使っていない title_texture_ は消した）・ステージ選択 4 件・ストーリー 3 件・エンディング 5 件（current_drawing_model_ は番号から引く関数にした）・背景レイヤー BgSpriteLayer 1 件。見つかったもの : ステージ選択の `Stage::texture`・エンディングの `std::vector< Model* >`。Scene の基底に resolve（画像・モデル・音。引けなければ名前ごとに 1 回ログ）・load_named_texture_handle・load_model_handle・play を足した。288a7686
  - 本編（GamePlayScene）6 件・CanvasTestScene 1 件・GeometryShaderCanvasTestScene の画像 1 件・線（Line）の画像 1 件。4c99b299
- **unique_ptr／RAII にした（0 件）** : 該当なし。持ち主が 1 つのもの（下の CityGenerator・GeometryShaderCanvasTestScene の作ったもの）は、作り手が ResourceManager に名前で入れ、自分のデストラクタで外している（RAII と同じ寿命）。描く側が名前・ハンドルで引けるよう ResourceManager に置いたままにした
- **そのまま（8 件）**
  - `CityGenerator::model_`・`debug_model_`、`GeometryShaderCanvasTestScene::mesh_` : 作り手＝持ち主。自分のデストラクタで外す（unload_named_*）まで、ほかに解放する所が無い
  - `blue_sky::graphics::Model::mesh_` : モデルの中のつながり（読み込みでモデルと一緒に作る）。メッシュだけを外すのは上の作り手が自分のモデルに対してだけ。ハンドルにするには Model が引く窓口を持つ必要があり、範囲が大きい
  - `Mesh::Buffer::mesh_` : 持ち主（Mesh）への逆向きの参照。Buffer は Mesh が持つので寿命が同じ
  - `BaseFileLoader::model_` : 読み込みの間だけ生きる一時的な読み手
  - `Sprite::pending_texture_` : 1 フレームの中だけ溜める（begin〜end の間）。flush の前にテクスチャは解放されない
  - `GraphicsManager::null_texture_` : 持ち主（GraphicsManager）自身が作り、自分のデストラクタまで解放しない
- **残り（後続）** : `blue_sky::graphics::Model::shader_list_`（`std::vector< Shader* >`。ポストエフェクトのシェーダーもここ）・`GraphicsManager::shadow_map_shader_`（自分で選び直す）。シェーダーを個別に解放するのは CityGenerator が自分のシェーダーに対してだけなので、今は壊れないが、モデルが引く窓口を持つときに一緒に移す
- テスト : TextureSlotTest（5 件）・RenderSystemHandleTest（3 件。core の描画の窓口の偽物）。535 → 543 件。シーンを替えて戻る確かめのため、開発用の BLUE_SKY_DEBUG_SCENE_SEQUENCE を足した。19628109
- **ハンドルの中身の作り替え（2026-11-11、doc/ハンドルの設計.md）** : 上のハンドル（番号＋ハッシュの表）を、Handle（ResourceManager の置き場にある、リソースへのポインタだけの持ち主の印）と WeakHandle（Handle へのポインタだけ、8 バイト）にした。`ResourceHandle` は WeakHandle の別名なので、上で書き換えた使う側（ModelHandle・TextureHandle・ShaderHandle・SoundHandle・TextureSlot・Scene::resolve）はそのまま `get()` で引く。引く手間は Release で 5〜12 ns → 0.4〜1.3 ns、Debug で 106〜120 ns → 2.5〜4.0 ns。解放と差し替えはフレームの終わり（Present の後）とシーンの切り替えにまとめ、1 フレームの中では引いたポインタを使い回せるようにした（描く流れは描く対象のモデルを 1 フレームに 1 回だけ引く）。上の「残り」（Model のシェーダーの一覧・shadow_map_shader_）も、フレームの中では解放されなくなった。955a8cb8・788a9551・88b378de・79c3bc70

## 5. 決めること

1. 最後に残す物理の世界（推奨 : core::physics::PhysicsManager に統一）
2. 行動の書き方（推奨 : 単純な種類はデータとシステムに分け切り、Player・Robot は当面 BehaviorComponent を許して段階的に分ける）
3. ハンドルの中身（推奨 : 今の ResourceHandle（通し番号と map）を型付きのまま広め、replace を足す。添字＋世代への変更は遅さが測れてから）

決めた結果は、この節に日付付きで書き足す。

### 決定（2026-11-10、利用者）
1. 物理 : core::physics::PhysicsManager に統一する（Bullet を隠す）。足りない機能（OBJ の当たりの形の読み込み・地面・自分を除くレイ判定）は BulletPhysics から移す。移し終えるまで本編は今の世界のまま橋渡しで進める → **完了（2026-11-11、fb58ec0e・42903a1f・de0e9039。3 章の「7」）**。世界は GameMain の 1 つで、シーンを替えるときに clear する。ActiveObjectPhysics・BulletPhysics は消した
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

### 6.7 1 万個の物体でのフレームの時間（2026-11-11）

6.4・6.6 のベンチマークは ECS の仕組みだけを測ったもので、本編のステージの物体は数十個しかない。そこで本編（game_play）に物体を大量に出して、実際のフレームの時間を測った。

#### 測り方

- **物体を出す** : 環境変数 `BLUE_SKY_DEBUG_SPAWN_OBJECTS="数[,種類]"`（12440399）。ステージを読んだ後・やり直しの前に、ObjectFactory で物体を作る。置く場所はプレイヤーの始めの位置を中心にした 3 m 間隔の正方形の格子（乱数なし。プレイヤーに触れないよう 4 m 上）。種類は負荷の違う 3 つを使った:
  - `static` : 動かない壁（wall-1。質量 0 の剛体）
  - `medal` : 回るメダル（重力 0・当たりの応答なしの動く剛体。SpinSystem が毎フレーム向きを変える）
  - `balloon` : ゆらぐ風船（同じく動く剛体。BalloonHoldSystem が毎フレーム位置と向きを変える。線があり、線も影を落とす）
- **段階ごとの時間** : 環境変数 `BLUE_SKY_DEBUG_FRAME_STAGES="最初,最後"`（ef783f6b）。update と render の段階ごと・System ごとの CPU の時間と描いた回数（ドローコール）の中央値を `[FrameStage]` としてログに出す。`BLUE_SKY_DEBUG_FRAME_TIME` は present にかかった時間（GPU が追いつかないとここで待つ）も出すようにした。
- 起動 : `BLUE_SKY_BACKGROUND=1`・`BLUE_SKY_START_SCENE=game_play`・`BLUE_SKY_START_STAGE=1-2`・`BLUE_SKY_START_OVERLAY=none`。Release はフレーム 200〜600、Debug は 60〜120 を測り、**3 回起動した中央値**。x64・開発機（RTX 5070 Ti）・解像度は設定のまま。
- 「仕事の時間」= update の始まり〜present の前（CPU）。「間隔」はフレームの始まりどうしの間隔。**本編は 60 fps で頭打ち**（game::MainLoop が 1/60 秒より早くは回さない。垂直同期は既定で切）なので、仕事の時間が 16.7 ms 未満なら間隔は 16.7 ms になる。
- 物体の数はステージの物体（21 個）を含まない数。

#### 結果（直した後。3 回の中央値）

作る時間の O(n²) を直した（027cdf2f）後に測った。風船だけは、次に書く風船の直し（1fdd800e）の前後の両方を載せる。

| 構成 | 種類 | 数 | 仕事の時間 | フレームの間隔 | present | 描いた回数 | 作る時間 |
|---|---|---:|---:|---:|---:|---:|---:|
| Release | （ステージだけ） | 0 | 0.17 ms | 16.75 ms | 0.05 ms | 144 | - |
| Release | static | 1,000 | 1.15 ms | 16.68 ms | 0.05 ms | 4,144 | 13 ms |
| Release | static | 10,000 | 12.11 ms | 16.67 ms | 4.50 ms | 40,144 | 58 ms |
| Release | static | 30,000 | 31.83 ms | 31.91 ms | 0.08 ms | 120,144 | 175 ms |
| Release | medal | 1,000 | 1.35 ms | 16.65 ms | 0.05 ms | 4,144 | 4 ms |
| Release | medal | 10,000 | 10.90 ms | 16.65 ms | 0.05 ms | 40,144 | 49 ms |
| Release | medal | 30,000 | 48.37 ms | 48.38 ms | 0.10 ms | 120,144 | 166 ms |
| Release | balloon（直す前） | 1,000 | 2.62 ms | 16.70 ms | 0.05 ms | 8,144 | 5 ms |
| Release | balloon（直す前） | 10,000 | 25.27 ms | 25.32 ms | 0.05 ms | 80,144 | 49 ms |
| Release | balloon（直す前） | 30,000 | 93.36 ms | 93.59 ms | 0.07 ms | 240,144 | 161 ms |
| Release | balloon（直した後） | 1,000 | 1.61 ms | 16.72 ms | 0.04 ms | 8,144 | 5 ms |
| Release | balloon（直した後） | 10,000 | 14.25 ms | 16.65 ms | 0.04 ms | 80,144 | 49 ms |
| Release | balloon（直した後） | 30,000 | 61.34 ms | 61.40 ms | 0.06 ms | 240,144 | 162 ms |
| Debug | （ステージだけ） | 0 | 1.85 ms | 16.73 ms | 0.04 ms | 144 | - |
| Debug | static | 1,000 | 14.71 ms | 16.70 ms | 0.04 ms | 4,144 | 26 ms |
| Debug | static | 10,000 | 137.62 ms | 137.69 ms | 0.05 ms | 40,144 | 265 ms |
| Debug | static | 30,000 | 438.29 ms | 438.59 ms | 0.07 ms | 120,144 | 1,244 ms |
| Debug | medal | 1,000 | 15.56 ms | 16.39 ms | 0.04 ms | 4,144 | 16 ms |
| Debug | medal | 10,000 | 167.74 ms | 167.83 ms | 0.07 ms | 40,144 | 243 ms |
| Debug | medal | 30,000 | 521.92 ms | 522.04 ms | 0.07 ms | 120,144 | 1,229 ms |
| Debug | balloon（直す前） | 1,000 | 25.75 ms | 25.80 ms | 0.04 ms | 8,144 | 19 ms |
| Debug | balloon（直す前） | 10,000 | 267.58 ms | 267.55 ms | 0.06 ms | 80,144 | 244 ms |
| Debug | balloon（直す前） | 30,000 | 839.75 ms | 839.12 ms | 0.06 ms | 240,144 | 1,193 ms |
| Debug | balloon（直した後） | 1,000 | 22.16 ms | 22.20 ms | 0.04 ms | 8,144 | 20 ms |
| Debug | balloon（直した後） | 10,000 | 232.76 ms | 232.78 ms | 0.06 ms | 80,144 | 256 ms |

読み方:

- **Release の 1 万個は、static・medal・balloon（直した後）とも 60 fps に収まる**（仕事の時間 11〜14 ms）。3 万個はどれも収まらない（32〜61 ms）。Debug は 1 万個で 140〜230 ms（4〜7 fps）。
- 描いた回数は **1 物体あたり 4 回（影 3 段 + 本体）、線のある風船は 8 回**（線も影 3 段 + 本体）。ステージだけで 144 回。物体の数にそのまま比例する（見えない物も全部描く。カリングが無い）。
- present の時間はほぼ 0.05〜0.1 ms で、GPU はほぼ追いついている。static 1 万個だけ 4.5 ms（壁は 4 × 1.75 m と面が大きく、画面を埋める量が多いためと見ている。推測）。ただし 60 fps の頭打ちで待つ間に GPU が進むので、GPU の時間そのものは測れていない。

#### 段階ごとの内訳（Release・1 万個。ms）

| 段階 | static | medal | balloon（直す前） | balloon（直した後） |
|---|---:|---:|---:|---:|
| 物体のメッシュ（本体を描く命令） | 7.54 | 4.25 | 4.27 | 4.29 |
| 影（シャドウマップ 3 段） | 2.74 | 2.72 | 4.64 | 4.47 |
| 物理（Bullet の step_simulation） | 0.69 | 1.86 | 3.11 | 2.13 |
| 描く準備・物体の定数の更新（setup_rendering） | 0.68 | 0.69 | 0.72 | 0.70 |
| 物体の線 | - | - | 0.57 | 0.48 |
| System : 回る物 / 風船 | - | 0.41 | 11.28 | 0.77 |
| 剛体の位置を写す（update_transforms） | 0.15 | 0.15 | 0.15 | 0.15 |
| そのほか（入力・当たり・ほかの System・背景・文字） | 0.1 未満 | 0.1 未満 | 0.1 未満 | 0.1 未満 |

3 万個（Release）: medal は 物理 15.54・メッシュ 12.90・影 8.91・定数 5.85、balloon（直した後）は 物理 19.88・影 14.02・メッシュ 12.86・定数 6.87。
Debug 1 万個: medal は メッシュ 71.9・影 52.6・物理 29.4・定数 4.6、balloon（直した後）は 影 91.1・メッシュ 72.4・物理 35.1・線 18.1・風船 9.1。

ECS の仕組み（System の走査・部品を引く・ハンドルを引く）はどこにも目立って出てこない（回る物 1 万個で 0.41 ms、剛体の位置を写す 0.15 ms。6.6 のベンチマークと合う）。重いのは描く流れと物理。

#### 重い所（上位 3 つ）

1. **描く流れ（物体のメッシュ + 影）: Release 1 万個で 7〜10 ms、Debug で 120〜160 ms。** 物体 1 つごとに、定数バッファを 3 か所（VS・DS・PS）に結び、シェーダーのパスを当て（render_technique）、1 回描く。影は 3 段それぞれで全部の物体を描く。1 回あたり本体 約 0.42 μs・影 約 0.09 μs（Release）。**見えない物も全部描く**（視錐台・影のカスケードごとのカリングが無い）。
2. **物理（Bullet）: medal・balloon で Release 1 万個 1.9〜2.1 ms、3 万個 15〜20 ms（個数が 3 倍で 8〜9 倍）。** メダル・風船は重力 0 で動かない「動く剛体」だが、System が毎フレーム剛体の位置・向きを書く（setWorldTransform）ので眠らず、毎フレーム全部が広い判定と積分を通る。動かない壁（質量 0）は 1 万個 0.69 ms。3 万個で個数以上に増えるのは、フレームが遅いほど 1/120 秒の刻みを多く回す（step_simulation の最大 5 回）ためと見ている（推測。刻みの回数は数えていない）。
3. **描く準備・物体の定数の更新: Release 1 万個 0.7 ms、3 万個 5.9〜6.9 ms（個数以上に増える）。** 物体ごとに DYNAMIC の定数バッファを Map（WRITE_DISCARD）・Unmap する。3 万個で増え方が大きいのは、ドライバの定数バッファの名前の付け替えが追いつかないためと見ている（推測）。

#### 直したもの（小さく確実なもの）

| 中身 | コミット | 前 → 後 |
|---|---|---|
| 物体を始めの位置に置いてから剛体を作る（以前は原点に作り、やり直しで動かしていた。作るたびに広い判定が原点に重なった全部の剛体と組を作り O(n²)）。ObjectFactory::create_object に始めの位置を渡せるようにした | 027cdf2f | medal を作る時間（Release）: 1,000 個 27 → 4 ms・3,000 個 392 → 14 ms・10,000 個 6,981 → 49 ms |
| ゆらぎの位相を物体ごとに一度だけ求めて覚える（以前は風船ごとに毎フレーム std::mt19937 を種から作り直していた） | 1fdd800e | 風船 1 万個（Release）: 風船の System 11.28 → 0.77 ms・仕事の時間 25.27 → 14.25 ms（60 fps に戻った）。Debug: 44.92 → 9.06 ms |

ステージを読むときの物体（数十個）も、作るときから始めの位置に置かれるようになった（やり直した後の位置は以前と同じ）。

#### 027cdf2f の後の本編のステージの見比べ（2026-11-11）

027cdf2f で本編のステージの物体も作るときから始めの位置に置かれるようになったので、027cdf2f より前の exe（ハンドルの作り替えの後。`%TEMP%\h1\s4_debug`）と今の exe（32d4ee30 の Debug）を、同じ確かめで比べた。物理が実時間で揺れるので、古い exe・今の exe をそれぞれ 2 回ずつ交互に流し、古いどうしの違いを揺れの幅とした（どれも作業ツリーの source を作業ディレクトリにして起動。解像度は 1280x720）。

| 確かめ | ステージ | 結果 |
|---|---|---|
| 作った物体の一覧（`BLUE_SKY_DEBUG_DUMP_STAGE`） | 16（0-0〜2-3・ext-0〜2・staff） | 4 回すべて同じ（種類・名前・位置・向き・行の数） |
| スイッチ（`BLUE_SKY_DEBUG_PRESS_SWITCHES`） | 1-1・2-1・1-2・0-1・2-0 | 入った回数・目標地点のログは同じ。2-1 だけ「目標地点へ動かします」の始めの位置の y（99.71 / 99.75）と、着いた回数（33 / 37）が回によって違うが、古いどうし（1 回目と 2 回目）でも同じ違いが出る |
| 物体巡り（`BLUE_SKY_DEBUG_VISIT_OBJECTS`） | 0-1・2-0・0-0・1-0・1-3・0-2 | 結果の行が 4 回すべて同じ |
| 石投げ（`BLUE_SKY_DEBUG_THROW_STONES`） | 1-0・1-1・1-2・2-0・2-1 | 結果の行が 4 回すべて同じ |
| 物体の様子（`BLUE_SKY_DEBUG_LOG_ACTORS`） | 0-0・0-1・2-0・2-1・2-3・1-2 | 0 フレーム目（やり直した直後）の行は 4 回すべて同じ。その後の数の差の最大は、古いどうしで 0.01〜3.51（1-2 のプレイヤーの 30 フレーム目）、古いと今で 0.01〜3.75 と同じ程度。1-2 のプレイヤーの y が -0.00 / 0.00 と書かれる回があるが、古い 1 回目と今の 2 回目は同じで、回による揺れ |
| 画面（60・120 フレーム目） | 上のすべて | 違う画素の割合は、古いどうしで 0〜88%（ext-1 の 60 フレーム目、1-2・1-3・1-1・1-0 などが大きい）、古いと今でも同じステージに同じ程度の差が出る。古いどうしで差の出ないステージ（0-1・0-2・2-0・2-1 など）は、古いと今でも差がほぼ 0 |

**揺れの幅を超える違いは無かった**（直したものは無い）。始めの位置に置いてから剛体を作っても、やり直した後の位置・その後の動き・スイッチ・物体巡り・石投げは以前と同じ。確かめのスクリプトと結果は `%TEMP%\p027`（run.ps1・all.ps1・compare.py・actors.py・runs）。

#### 直し方の案（今回は書くだけ）

| # | 案 | 効く所 | 効果の見積もり | 手間 |
|---|---|---|---|---|
| A | **視錐台カリング**（本体）と **影のカスケードごとのカリング**。物体の AABB を部品に持ち、描く前に段ごとに選ぶ | 1（描く流れ）・3 | 格子の 1 万個のうち画面に入るのは一部なので、描く回数が数分の 1 になる見込み（Release 7〜10 ms → 2〜4 ms。画面に入る割合は測っていない） | 中（AABB の部品・選ぶ System・影の段ごとの範囲） |
| B | **インスタンシング**。同じモデル・同じシェーダーの物体をまとめて 1 回で描く（LitInstancedShader がすでにある）。world 行列は 1 つの大きなバッファ（構造化バッファかインスタンスバッファ）に書く | 1・3 | 描く回数が「モデルの種類 × 4」に減る。メッシュ + 影 + 定数で Release 1 万個 約 8 ms → 1 ms 前後の見込み（推測）。Debug ではさらに効く | 大（本体・影・線・速度のシェーダーをインスタンス用に。物体ごとの色・点滅の扱い） |
| C | **動かない動く剛体をキネマティックにする**（メダル・風船・梯子など、速度では動かさず位置を直接書く物）。または当たりだけのゴースト（btGhostObject）にする | 2（物理） | 積分と眠らないことの手間が無くなり、広い判定だけになる。medal 3 万個で 15.5 ms → 数 ms の見込み（推測） | 小〜中（当たりの振る舞い（拾う・つかむ）が変わらないことをテストで確かめる） |
| D | **描く前に、モデル・シェーダーで並べ替える**（パスを当てる回数を減らす）。A・B の前でもできる | 1 | 本体 0.42 μs/回 の一部（パスの当て直し）が減る。効果は測っていない | 小〜中 |
| E | 物理の刻みの上限（最大 5 回）を、重いときに下げる。または刻みを 1/60 秒にする | 2 | 遅いフレームで物理がさらに遅くなる悪循環を止める | 小（動きの細かさが変わるので、遊んで確かめる） |

勧める順: **C（小さく、物理の半分以上が消える見込み）→ A（描く回数を減らす。B より小さい）→ B**。Debug の 1 万個を遊べる速さにするには A か B が要る。

#### 確かめられなかったこと

- **GPU の時間** : present の待ちしか見ていない（60 fps の頭打ちの間に GPU が進むので隠れる）。D3D11 のタイムスタンプのクエリは足していない。
- **プロファイラ** : xperf（Windows Performance Toolkit）はカーネルロガーを始められなかった（`NT Kernel Logger: フラグが無効です (0x3ec)`）。VSDiagnostics.exe はあるが、集めた結果をコマンドで集計する手段が無いので使っていない。段階ごとの時間は `BLUE_SKY_DEBUG_FRAME_STAGES` で測った。
- **画面に何個映っているか**（カリングの効果の見積もりに要る）。
- 物体はプレイヤーの 4 m 上に置いたので、物体どうし・プレイヤーとの当たり（check_collision）はほとんど起きていない（0.1 ms 未満）。当たりが多いときの重さは測っていない。
- 速度のパス（render_objects_velocity）は game_play では使っていない（DebugScene だけ）ので測っていない。

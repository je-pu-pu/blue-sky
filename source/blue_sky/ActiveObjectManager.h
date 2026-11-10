#pragma once

#include <GameObject/ActiveObject.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace blue_sky
{

class ActiveObjectPhysics;
struct GameObjectContext;
namespace graphics { class GraphicsManager; }
}

namespace core::ecs { class EntityManager; class Entity; }
namespace core::sound { class SoundManager; }

namespace blue_sky
{

/**
 * 複数の ActiveObject を管理する Manager
 *
 * 当たりのイベントの互換の層（ecs::CollisionEventHandler）も受け持つ。ecs::CollisionEventSystem が Entity の番号で渡す当たりを、
 * 番号から ActiveObject* を引いて、今の二重ディスパッチ（GameObject::on_collide_with）に渡す
 */
class ActiveObjectManager : public ecs::CollisionEventHandler
{
public:
	/// 加えた順（作った順）に並ぶ一覧。更新・スイッチの処理の順が起動ごとに変わらないように
	/// （以前は unordered_set< ActiveObject* > で、アドレスの順になり起動ごとに変わっていた）
	using ActiveObjectList			= std::vector< ActiveObject* >;

	using CreateObjectFunction		= std::function< ActiveObject* () >;

private:
	ActiveObjectList active_object_list_;

	// 物体の名前の表は ObjectFactory が持つ（EntityId で引く。以前はここに名前 → ActiveObject* の表があった）

	// 目標地点・向きは Entity の部品（ecs::TargetLocationComponent・TargetDirectionComponent）が持ち、ecs::MoveToTargetSystem が動かす
	// （以前はここに ActiveObject* を鍵にした 3 つの unordered_map があった）

	std::unordered_map< string_t, CreateObjectFunction > object_creator_map_;

	ActiveObjectPhysics* physics_;					///< 物理（GameMain が渡す。以前は GameMain のシングルトンから引いていた）
	graphics::GraphicsManager* graphics_manager_;	///< 描画（同上）
	const GameObjectContext* object_context_;		///< オブジェクトに渡すもの（物理の世界・音・経過時間。同上）
	core::ecs::EntityManager* entity_manager_;		///< オブジェクトの Entity（描画の部品）を作る先（同上。描画のデータは ecs::ObjectRenderComponent が持つ）

public:
	ActiveObjectManager( ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, const GameObjectContext* object_context, core::ecs::EntityManager* entity_manager );
	~ActiveObjectManager();

	void clear();

	void restart();

	void add_active_object( ActiveObject* active_object );

	// 目標地点・向きへ動かすのは GameObjectRef::set_target_location など（ecs::MoveToTargetSystem に頼む。以前はここに窓口があった）

	/// まだ ActiveObject の種類を名前から作る（ObjectFactory::create_active_object が呼ぶ。知らない種類なら nullptr）
	ActiveObject* create_object( const string_t& );

	ActiveObject* clone_object( const ActiveObject* );

	void update();

	/**
	 * 1 つの風船（ECS だけの物体）を動かす（ecs::BalloonHoldSystem に頼む）
	 *
	 * GamePlayScene が物理を進めた後に、プレイヤーの持っている風船を手元に合わせ直すのに使う（以前は Balloon::update を呼んでいた）
	 */
	void update_balloon( core::ecs::EntityId id );

	/**
	 * 物理を進めた後に、全部の剛体の位置と回転を位置の正（TransformComponent）へ写す（ecs::PhysicsSyncSystem に頼む）
	 *
	 * 以前は GamePlayScene が全物体の GameObject::update_transform を呼んでいた
	 */
	void update_transforms();

	/**
	 * a と接している物体との当たりを起こす（以前の ActiveObjectPhysics::check_collision_with）
	 *
	 * 接した組を ecs::CollisionEventSystem に集め、振り分けて、互換の層（下の on_collide_*）が on_collide_with を呼ぶ
	 */
	void check_collision_with( ActiveObject* a );

	/// 物体（Entity の番号）と接している物体との当たりを起こす（プレイヤー。剛体が無ければ何もしない）
	void check_collision_with( core::ecs::EntityId id );

	/// 世界の中で接している物体どうしの当たりを全部起こす（以前の ActiveObjectPhysics::check_collision_all）
	void check_collision_all();

	/// Entity の番号から物体を引く（無い・消えた・ActiveObject でないなら nullptr）
	ActiveObject* find_active_object( core::ecs::EntityId id ) const;

	// ecs::CollisionEventHandler（互換の層）
	void on_collide_with_ground( core::ecs::EntityId id ) override;
	void on_collide( core::ecs::EntityId a, core::ecs::EntityId b ) override;

	/**
	 * ActiveObject と ECS だけの物体（ActiveObject を持たない Entity）が当たった
	 *
	 * ECS だけの物体の振る舞いは部品の組み合わせで決める（StaticObjectComponent なら object.on_collide_with_static_object など）。
	 * 相手（プレイヤーなど）はまだ ActiveObject なので、相手の振る舞いは GameObject の仮想関数で呼ぶ
	 *
	 * @param object 当たった ActiveObject
	 * @param object_id object の Entity の番号（梯子が触れた物体を覚えるのに使う）
	 * @param entity 当たった ECS だけの物体
	 * @param sound_manager ECS だけの物体の位置で音を鳴らす先（nullptr なら鳴らさない。テスト）
	 */
	static void on_collide_with_entity_object( GameObject& object, core::ecs::EntityId object_id, core::ecs::Entity& entity, core::sound::SoundManager* sound_manager = nullptr );

	/**
	 * ECS だけの物体どうしが当たった
	 *
	 * 部品の組み合わせで決める。今は 石（ThrowableComponent）と次の組だけ（それ以外の組は何もしない。以前と同じ）:
	 * - 風船（BalloonHoldComponent）: 割れる（以前の Balloon::on_collide_with_stone）
	 * - 敵（RobotBehaviorComponent）: 立つ・回る間に、こちらへ飛んでくる石なら止まる（以前の Robot::on_collide_with_stone）
	 * - スイッチ（SwitchComponent）: 石の速さで入・切が変わる（以前の BaseSwitch::on_collide_with( Stone* )。空間スイッチは石では変わらない）
	 */
	static void on_collide_entity_objects( core::ecs::Entity& a, core::ecs::Entity& b, core::sound::SoundManager* sound_manager = nullptr );

	/**
	 * プレイヤー（PlayerBehaviorComponent を持つ ECS だけの物体）と物体が当たった
	 *
	 * 物体の部品の組み合わせで、プレイヤーの System（PlayerBehaviorSystem の on_touch_*）に知らせる。順と回数は、以前プレイヤーが ActiveObject だったときの
	 * on_collide_with_entity_object と同じ（動かない物 → 拾う物 → 石 → スイッチ → 風船 → 敵 → 梯子。1 組につき 1 回）。
	 * 以前は GameObject の仮想関数（on_pick_up・on_collide_with_balloon など）をプレイヤーが実装していた
	 *
	 * @param sound_manager 梯子が触れた音を鳴らす先（nullptr なら鳴らさない。テスト）
	 */
	static void on_collide_with_player( core::ecs::Entity& player, core::ecs::Entity& entity, core::sound::SoundManager* sound_manager = nullptr );

	// ECS だけの物体（ObjectFactory が作る）のうち、毎フレーム動く物（動く床・回る物）は update の中で System に頼む

	/// 全オブジェクトの位置と描くかどうかを描画の部品へ写す（描く前に毎フレーム。GameMain がシーンを描く前に呼ぶ）
	void sync_render_components() const;

	ActiveObjectList& active_object_list() { return active_object_list_; }
	const ActiveObjectList& active_object_list() const { return active_object_list_; }
};

} // namespace blue_sky

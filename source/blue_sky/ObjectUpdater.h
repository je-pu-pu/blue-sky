#pragma once

#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <core/ecs/type.h>

namespace core::ecs { class EntityManager; }
namespace core::physics { class PhysicsManager; }

namespace blue_sky
{

struct GameObjectContext;

/**
 * 本編のゲームの物体（ECS だけの物体）の System を、決まった順に動かす
 *
 * 以前の ActiveObjectManager のうち、物体の一覧（ActiveObject*）を持つ所を除いた残り（段階 6 で ActiveObject の種類が無くなったので消した）:
 * - update : プレイヤー → 風船 → 少女 → 敵 → アニメーション → 動く床 → 触れた物の覚え → 回る物 → 目標地点・向き の順に System を動かす
 *   （以前の ActiveObjectManager::update と同じ順。以前は先頭のプレイヤーが ActiveObject の一覧の update で動いていた）
 * - update_transforms : 物理を進めた後に、剛体の位置を位置の正（TransformComponent）へ写す
 * - check_collision_with・check_collision_all : 当たった組を CollisionEventSystem に集めて渡し、部品の組み合わせの決まり（ecs::contact_rules）を当てる
 *
 * 物体を作る・名前で引く・やり直す・片付けるのは ObjectFactory。EntityManager::update() では System を動かさない（本編のシーンが呼ぶ所を決める）
 */
class ObjectUpdater : public ecs::CollisionEventHandler
{
private:
	const core::physics::PhysicsManager* physics_;	///< 当たりを集める世界（GameMain が渡す）
	const GameObjectContext* object_context_;		///< 経過時間・音（同上）
	core::ecs::EntityManager* entity_manager_;		///< 物体の Entity（同上）

	float_t get_delta_time() const;
	float_t get_scene_elapsed_time() const;

public:
	ObjectUpdater( const core::physics::PhysicsManager* physics, const GameObjectContext* object_context, core::ecs::EntityManager* entity_manager );

	/// 物体の System を決まった順に動かす（物理を進める前。上の説明の順）
	void update();

	/**
	 * 1 つの風船を動かす（ecs::BalloonHoldSystem に頼む）
	 *
	 * GamePlayScene が物理を進めた後に、プレイヤーの持っている風船を手元に合わせ直すのに使う
	 */
	void update_balloon( core::ecs::EntityId id );

	/// 物理を進めた後に、全部の剛体の位置と回転を位置の正（TransformComponent）へ写す（ecs::PhysicsSyncSystem に頼む）
	void update_transforms();

	/// 物体（Entity の番号）と接している物体との当たりを起こす（プレイヤー。剛体が無ければ何もしない）
	void check_collision_with( core::ecs::EntityId id );

	/// 世界の中で接している物体どうしの当たりを全部起こす
	void check_collision_all();

	// ecs::CollisionEventHandler
	void on_collide_with_ground( core::ecs::EntityId id ) override;
	void on_collide( core::ecs::EntityId a, core::ecs::EntityId b ) override;
};

} // namespace blue_sky

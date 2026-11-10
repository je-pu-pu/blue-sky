#pragma once

#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/System.h>

namespace core::physics
{
	class PhysicsManager;
}

namespace core::ecs
{

/**
 * 物理演算システム
 *
 * RigidBodyComponent と TransformComponent を持つ Entity に対して物理演算を適用する。
 * PhysicsManager を通じて物理シミュレーションを実行する。
 */
class PhysicsSystem : public System<RigidBodyComponent, TransformComponent>
{
public:
	using Vector = core::math::Vector;
	using Transform = core::math::Transform;

public:
	/// physics_manager : 物理の世界（作り手が渡す。nullptr なら何もしない。以前は core::Service から取っていた）
	explicit PhysicsSystem( core::physics::PhysicsManager* physics_manager ) : physics_manager_( physics_manager ) { }

	/// 対象に残っている剛体をすべて解放する（remove_system・EntityManager::clear のとき。PhysicsManager はこれより長く生きること）
	~PhysicsSystem() override;

	void update() override;

protected:
	/// 対象から外れる（RigidBody か Transform を外した・Entity を消した）とき、その剛体を解放する
	void on_remove_entity( EntityId, ComponentTuple& ) override;

private:
	core::physics::PhysicsManager* physics_manager_;

	/**
	 * RigidBodyComponent の剛体を物理の世界から外して解放する（無ければ何もしない）
	 *
	 * 以前は destroy_rigid_body を呼ぶ所が無く、Entity を消しても・シーンを替えても剛体が世界に残っていた
	 */
	void release_rigid_body( RigidBodyComponent* rigid_body );

	/**
	 * RigidBodyComponent を初期化する（剛体を作り、userPointer に id のタグを付ける）
	 */
	void initialize_rigid_body( EntityId id, RigidBodyComponent* rigid_body, TransformComponent* transform );

}; // class PhysicsSystem

} // namespace core::ecs

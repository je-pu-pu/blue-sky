#pragma once

#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/System.h>

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
	PhysicsSystem() = default;
	~PhysicsSystem() override = default;

	void update() override;
	void update( ComponentTuple& ) const override;

private:
	/**
	 * RigidBodyComponent を初期化する
	 */
	void initialize_rigid_body( RigidBodyComponent* rigid_body, TransformComponent* transform );

}; // class PhysicsSystem

} // namespace core::ecs

#include "PhysicsSystem.h"

/// @todo core から blue_sky を include しない
#include <blue_sky/GameMain.h>
#include <blue_sky/ActiveObjectPhysics.h>

namespace core::ecs
{

PhysicsSystem::PhysicsSystem()
{
	//
}

void PhysicsSystem::update( ComponentTuple& component_tuple ) const
{
	// 物理演算のステップを進める
	blue_sky::GameMain::get_instance()->get_physics_manager()->update();

	auto rigid_body = std::get< RigidBodyComponent* >( component_tuple );
	auto transform = std::get< TransformComponent* >( component_tuple );
	
	// 物理演算の結果を TransformComponent に適用する
	transform->transform.set_position( Vector( rigid_body->rigid_body.getWorldTransform().getOrigin() ) );

	auto q = rigid_body->rigid_body.getWorldTransform().getRotation();
	transform->transform.set_rotation( reinterpret_cast< const Quaternion& >( q ) );
}

} // namespace core::ecs

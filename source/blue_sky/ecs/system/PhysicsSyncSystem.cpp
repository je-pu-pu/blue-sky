#include "PhysicsSyncSystem.h"

namespace blue_sky::ecs
{

void PhysicsSyncSystem::update_transforms()
{
	for_each( [] ( ComponentTuple& t )
	{
		const auto* body = std::get< PhysicsBodyComponent* >( t );

		if ( ! body->get() )
		{
			return;
		}

		std::get< core::ecs::TransformComponent* >( t )->transform = read_body( * body->get(), body->height_offset );
	} );
}

core::Transform PhysicsSyncSystem::read_body( const core::physics::RigidBody& body, float_t height_offset )
{
	// 剛体の中心から足元へ戻す（計算は以前と同じ Bullet の掛け算。core::physics::RigidBody の中へ移した）
	return body.get_world_transform( core::Vector( 0, -height_offset, 0 ) );
}

void PhysicsSyncSystem::write_body( core::physics::RigidBody& body, float_t height_offset, const core::Transform& transform )
{
	body.set_world_transform( transform, core::Vector( 0, height_offset, 0 ) );
}

} // namespace blue_sky::ecs
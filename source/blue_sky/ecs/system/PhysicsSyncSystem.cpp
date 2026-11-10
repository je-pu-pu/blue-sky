#include "PhysicsSyncSystem.h"

#include <core/math/Quaternion.h>

#include <btBulletDynamicsCommon.h>

namespace blue_sky::ecs
{

void PhysicsSyncSystem::update_transforms()
{
	for_each( [] ( ComponentTuple& t )
	{
		const auto* body = std::get< PhysicsBodyComponent* >( t );

		if ( ! body->body )
		{
			return;
		}

		std::get< core::ecs::TransformComponent* >( t )->transform = read_body( * body->body, body->height_offset );
	} );
}

core::Transform PhysicsSyncSystem::read_body( const btRigidBody& body, float_t height_offset )
{
	auto t = body.getWorldTransform();

	core::Transform offset;
	offset.set_identity();
	offset.set_position( core::Vector( 0, -height_offset, 0 ) );

	t *= offset;

	// btVector3 と Vector はメモリレイアウトが同一のため reinterpret_cast で変換
	core::Transform transform;
	transform.set_position( reinterpret_cast< const core::Vector& >( t.getOrigin() ) );

	auto q = t.getRotation();
	transform.set_rotation( reinterpret_cast< const core::Quaternion& >( q ) );

	return transform;
}

void PhysicsSyncSystem::write_body( btRigidBody& body, float_t height_offset, const core::Transform& transform )
{
	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0, height_offset, 0 ) );

	btTransform t = btTransform( transform ) * offset;
	body.setWorldTransform( t );
	body.setInterpolationWorldTransform( t );
}

} // namespace blue_sky::ecs

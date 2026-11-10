#include "ObjectBody.h"

#include <btBulletDynamicsCommon.h>

namespace blue_sky::ecs::object_body
{

const core::Vector& get_velocity( const btRigidBody& body )
{
	// btVector3 と Vector はメモリレイアウトが同一
	return reinterpret_cast< const core::Vector& >( body.getLinearVelocity() );
}

void set_velocity( btRigidBody& body, const core::Vector& v )
{
	if ( v == get_velocity( body ) )
	{
		return;
	}

	body.setLinearVelocity( v );
	body.setInterpolationLinearVelocity( v );

	body.activate( true );
	body.setActivationState( true );
}

void set_mass( btRigidBody& body, float_t mass )
{
	btVector3 local_inertia( 0, 0, 0 );
	body.getCollisionShape()->calculateLocalInertia( mass, local_inertia );
	body.setMassProps( mass, local_inertia );
}

void set_gravity( btRigidBody& body, const core::Vector& gravity )
{
	body.setGravity( gravity );
}

void set_friction( btRigidBody& body, float_t friction )
{
	body.setFriction( friction );
}

void set_angular_factor( btRigidBody& body, float_t factor )
{
	body.setAngularFactor( factor );
}

void set_angular_factor( btRigidBody& body, const core::Vector& factor )
{
	body.setAngularFactor( factor );
}

void set_kinematic( btRigidBody& body, bool is_kinematic )
{
	if ( is_kinematic )
	{
		body.setCollisionFlags( body.getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
	}
	else
	{
		body.setCollisionFlags( body.getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT );
	}
}

void set_no_contact_response( btRigidBody& body, bool is_no_contact_response )
{
	if ( is_no_contact_response )
	{
		body.setCollisionFlags( body.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
	else
	{
		body.setCollisionFlags( body.getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
}

} // namespace blue_sky::ecs::object_body

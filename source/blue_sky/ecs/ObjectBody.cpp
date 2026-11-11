#include "ObjectBody.h"

#include <core/physics/RigidBody.h>

namespace blue_sky::ecs::object_body
{

core::Vector get_velocity( const core::physics::RigidBody& body )
{
	return body.get_linear_velocity();
}

void set_velocity( core::physics::RigidBody& body, const core::Vector& v )
{
	if ( v == get_velocity( body ) )
	{
		return;
	}

	body.set_linear_velocity( v );
	body.set_interpolation_linear_velocity( v );

	body.activate( true );
	body.set_activation_state_active();
}

void set_mass( core::physics::RigidBody& body, float_t mass )
{
	body.set_mass( mass );
}

void set_gravity( core::physics::RigidBody& body, const core::Vector& gravity )
{
	body.set_gravity( gravity );
}

void set_friction( core::physics::RigidBody& body, float_t friction )
{
	body.set_friction( friction );
}

void set_angular_factor( core::physics::RigidBody& body, float_t factor )
{
	body.set_angular_factor( factor );
}

void set_angular_factor( core::physics::RigidBody& body, const core::Vector& factor )
{
	body.set_angular_factor( factor );
}

void set_kinematic( core::physics::RigidBody& body, bool is_kinematic )
{
	body.set_kinematic( is_kinematic );
}

void set_no_contact_response( core::physics::RigidBody& body, bool is_no_contact_response )
{
	body.set_no_contact_response( is_no_contact_response );
}

// const core::Vector GravityDefault( 0.f, -3.711f, 0.f, 0.f ); // 火星
const core::Vector GravityDefault( 0.f, -9.8f, 0.f, 0.f ); // 地球

bool is_moving_to( const core::Vector& location, const core::Vector& velocity, const core::Vector& target_location )
{
	core::Vector v = velocity;
	v.normalize();
	v *= 0.01f;

	const float_t current_length = ( target_location - location ).length();
	const float_t next_length = ( target_location - ( location + v ) ).length();

	return next_length < current_length;
}

} // namespace blue_sky::ecs::object_body

#include "ActiveObject.h"
#include <common/math.h>
#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: rigid_body_( 0 )
	, transform_( 0 )

	, velocity_( 0, 0, 0 )
	, direction_degree_( 0 )
	, front_( 0, 0, 1 )
	, right_( 1, 0, 0 )
{
	transform_ = new Transform();
	transform_->setIdentity();
}

ActiveObject::~ActiveObject()
{
	delete transform_;
}

void ActiveObject::update_rigid_body_velocity()
{
	if ( get_velocity() == Vector3( 0, 0, 0 ) )
	{
		return;
	}

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity( get_velocity() );
}

void ActiveObject::update_transform()
{
	if ( rigid_body_ )
	{
		rigid_body_->getMotionState()->getWorldTransform( * transform_ );
		Transform offset;
		offset.setIdentity();
		offset.setOrigin( Vector3( 0, -get_collision_height() / 2.f, 0 ) );

		*transform_ = *transform_ * offset;
	}
}

void ActiveObject::step( float_t s )
{
	get_velocity() = get_front() * s;
	get_velocity().setY( get_rigid_body()->getLinearVelocity().y() );
}

void ActiveObject::side_step( float_t s )
{
	get_velocity() = get_right() * s;
	get_velocity().setY( get_rigid_body()->getLinearVelocity().y() );
}

void ActiveObject::set_location( float_t x, float_t y, float_t z )
{
	get_transform().getOrigin().setValue( x, y, z );
}

ActiveObject::Transform& ActiveObject::get_transform()
{
	return * transform_;
}

const ActiveObject::Transform& ActiveObject::get_transform() const
{
	return * transform_;
}

void ActiveObject::set_direction_degree( float d )
{
	direction_degree_ = d;

	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	Matrix m;
	m.setEulerZYX( 0, math::degree_to_radian( -direction_degree_ ), 0 );

	front_ = Vector3( 0.f, 0.f, 1.f ) * m;
	right_ = Vector3( 1.f, 0.f, 0.f ) * m;
}


} // namespace blue_sky

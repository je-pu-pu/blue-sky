#include "ActiveObject.h"
#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: rigid_body_( 0 )
	, transform_( 0 )
{
	transform_ = new Transform();
	transform_->setIdentity();
}

ActiveObject::~ActiveObject()
{
	delete transform_;
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

void ActiveObject::step( float_t velocity )
{
	rigid_body_->setActivationState( true );
	rigid_body_->setLinearVelocity( Vector3( 0, 0.f, velocity ) );
}

void ActiveObject::side_step( float_t velocity )
{
	rigid_body_->setActivationState( true );
	rigid_body_->setLinearVelocity( Vector3( velocity, 0.f, 0 ) );
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



} // namespace blue_sky

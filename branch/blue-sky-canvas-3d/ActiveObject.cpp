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
	}

	// transform_->getOrigin().
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

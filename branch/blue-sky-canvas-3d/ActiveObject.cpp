#include "ActiveObject.h"
#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: rigid_body_( 0 )
	, transform_( 0 )
{
	transform_ = new Transform();
}

ActiveObject::~ActiveObject()
{
	delete transform_;
}

void ActiveObject::update_transform()
{
	rigid_body_->getMotionState()->getWorldTransform( * transform_ );

	// transform_->getOrigin().
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

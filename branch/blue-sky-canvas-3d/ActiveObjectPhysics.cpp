#include "ActiveObjectPhysics.h"
#include "ActiveObject.h"

namespace blue_sky
{

ActiveObjectPhysics::ActiveObjectPhysics()
{

}

ActiveObjectPhysics::~ActiveObjectPhysics()
{

}

ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object( const ActiveObject* active_object )
{
	btVector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );

	return add_box_rigid_body( box, active_object->get_transform() );
}

}
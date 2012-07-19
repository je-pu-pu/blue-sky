#include "ActiveObjectPhysics.h"
#include "StaticObject.h"
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
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	return add_box_rigid_body( active_object->get_transform(), offset, box, dynamic_cast< const StaticObject* >( active_object ) != 0 );
}

}
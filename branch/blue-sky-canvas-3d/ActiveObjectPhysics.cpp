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

ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object_as_cylinder( const ActiveObject* active_object )
{
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	return add_cylinder_rigid_body( active_object->get_transform(), offset, box, dynamic_cast< const StaticObject* >( active_object ) != 0 );
}

class ContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	bool is_hit_;

public:
	ContactResultCallback()
		: is_hit_( false )
	{

	}

	btScalar addSingleResult( btManifoldPoint& cp, const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
	{
		is_hit_ = true;

		return 0;
	}

	bool is_hit() const { return is_hit_; }
};

bool ActiveObjectPhysics::is_collision( ActiveObject* a, ActiveObject* b )
{
	ContactResultCallback callback;
	dynamics_world_->contactPairTest( a->get_rigid_body(), b->get_rigid_body(), callback );

	return callback.is_hit();
}

}
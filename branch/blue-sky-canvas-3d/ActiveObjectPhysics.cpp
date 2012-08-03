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

ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object( ActiveObject* active_object )
{
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBody* rigid_body = add_box_rigid_body( active_object->get_transform(), offset, box, dynamic_cast< const StaticObject* >( active_object ) != 0 );
	rigid_body->setUserPointer(	active_object );

	return rigid_body;
}

ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object_as_cylinder( ActiveObject* active_object )
{
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBody* rigid_body = add_cylinder_rigid_body( active_object->get_transform(), offset, box, dynamic_cast< const StaticObject* >( active_object ) != 0 );
	rigid_body->setUserPointer(	active_object );

	return rigid_body;
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

	btScalar addSingleResult( btManifoldPoint& cp, const btCollisionObject* o0,int part_id_0,int index_0, const btCollisionObject* o1, int part_id_1, int index_1 )
	{
		is_hit_ = true;

		return 0;
	}

	bool is_hit() const { return is_hit_; }
};

class ActiveObjectContactResultCallback : public btCollisionWorld::ContactResultCallback
{
public:
	btScalar addSingleResult( btManifoldPoint& cp, const btCollisionObject* o0,int part_id_0,int index_0, const btCollisionObject* o1, int part_id_1, int index_1 )
	{
		ActiveObject* a0 = reinterpret_cast< ActiveObject* >( o0->getUserPointer() );
		ActiveObject* a1 = reinterpret_cast< ActiveObject* >( o1->getUserPointer() );

		if ( ! a0 || ! a1 || a0->is_dead() || a1->is_dead() )
		{
			return 0;
		}

		a1->on_collide_with( a0 );

		return 0;
	}
};

bool ActiveObjectPhysics::is_collision( ActiveObject* a, ActiveObject* b )
{
	ContactResultCallback callback;
	dynamics_world_->contactPairTest( a->get_rigid_body(), b->get_rigid_body(), callback );

	return callback.is_hit();
}

void ActiveObjectPhysics::check_collision_with( ActiveObject* a )
{
	ActiveObjectContactResultCallback callback;
	dynamics_world_->contactTest( a->get_rigid_body(), callback );
}

}
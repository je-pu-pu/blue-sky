#include "ActiveObjectPhysics.h"
#include "ActiveObject.h"

#include <algorithm>

#include "memory.h"

namespace blue_sky
{

ActiveObjectPhysics::ActiveObjectPhysics()
{

}

ActiveObjectPhysics::~ActiveObjectPhysics()
{

}

/**
 * ActiveObject をボックス形状の剛体として物理空間に追加する
 *
 * @param active_object ActiveObject 
 */
ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object( ActiveObject* active_object )
{
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBody* rigid_body = add_box_rigid_body( active_object->get_transform(), offset, box, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body, active_object );

	return rigid_body;
}

/**
 * ActiveObject をカプセル形状の剛体として物理空間に追加する
 *
 * @param active_object ActiveObject 
 */
ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object_as_capsule( ActiveObject* active_object )
{
	float_t radius = std::max( active_object->get_collision_width(), active_object->get_collision_depth() ) / 2.f;
	float_t height = active_object->get_collision_height() - ( radius * 2.f );

	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBody* rigid_body = add_capsule_rigid_body( active_object->get_transform(), offset, radius, height, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body, active_object );

	return rigid_body;
}

/**
 * ActiveObject をシリンダー形状の剛体として物理空間に追加する
 *
 * @param active_object ActiveObject 
 */
ActiveObjectPhysics::RigidBody* ActiveObjectPhysics::add_active_object_as_cylinder( ActiveObject* active_object )
{
	Vector3 box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBody* rigid_body = add_cylinder_rigid_body( active_object->get_transform(), offset, box, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body, active_object );

	return rigid_body;
}

/**
 * ActiveObject の情報を RigidBody に設定する
 *
 * @param rigid_body RigidBody
 * @param active_object ActiveObject
 */
void ActiveObjectPhysics::set_active_object_info_to_rigid_body( RigidBody* rigid_body, ActiveObject* active_object )
{
	rigid_body->setUserPointer(	active_object );

	if ( ! active_object->is_hard() )
	{
		rigid_body->setCollisionFlags( rigid_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
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

	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper*, int, int, const btCollisionObjectWrapper*, int, int )
	{
		is_hit_ = true;

		return 0;
	}

	bool is_hit() const { return is_hit_; }
};

class ActiveObjectContactResultCallback : public btCollisionWorld::ContactResultCallback
{
public:
	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper* o0, int, int, const btCollisionObjectWrapper* o1, int, int )
	{
		return on_collide( o0->getCollisionObject(), o1->getCollisionObject() );
	}

	static btScalar on_collide( const btCollisionObject* o0, const btCollisionObject* o1 )
	{
		ActiveObject* a0 = reinterpret_cast< ActiveObject* >( o0->getUserPointer() );
		ActiveObject* a1 = reinterpret_cast< ActiveObject* >( o1->getUserPointer() );

		if ( ! a0 )
		{
			if ( a1 && ! a1->is_dead() )
			{
				a1->on_collide_with_ground();
			}

			return 0;
		}

		if ( ! a1 )
		{
			if ( a0 && ! a0->is_dead() )
			{
				a0->on_collide_with_ground();
			}

			return 0;
		}

		if ( a0->is_dead() || a1->is_dead() )
		{
			return 0;
		}

		a0->on_collide_with( a1 );
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

void ActiveObjectPhysics::check_collision_all()
{
	int numManifolds = dynamics_world_->getDispatcher()->getNumManifolds();

	for ( int i = 0; i < numManifolds; i++ )
	{
		const btPersistentManifold* contactManifold =  dynamics_world_->getDispatcher()->getManifoldByIndexInternal( i );
		const btCollisionObject* o0 = static_cast< const btCollisionObject* >( contactManifold->getBody0() );
		const btCollisionObject* o1 = static_cast< const btCollisionObject* >( contactManifold->getBody1() );
 
		int numContacts = contactManifold->getNumContacts();

		for ( int j = 0; j < numContacts; j++ )
		{
			const btManifoldPoint& pt = contactManifold->getContactPoint( j );

			if ( pt.getDistance() < 0.f )
			{
				ActiveObjectContactResultCallback::on_collide( o0, o1 );

				/*
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;
				*/
			}
		}
	}
}

}
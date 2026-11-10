#include "ActiveObjectPhysics.h"

#include <GameObject/ActiveObject.h>

#include <algorithm>

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
ActiveObjectPhysics::RigidBodyOwner ActiveObjectPhysics::add_active_object_as_box( ActiveObject* active_object )
{
	Vector box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );
	
	Transform offset;
	offset.set_identity();
	offset.set_position( Vector( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBodyOwner rigid_body = add_box_rigid_body( active_object->get_transform(), offset, box, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body.get(), active_object );

	return rigid_body;
}

/**
 * ActiveObject をカプセル形状の剛体として物理空間に追加する
 *
 * @param active_object ActiveObject 
 */
ActiveObjectPhysics::RigidBodyOwner ActiveObjectPhysics::add_active_object_as_capsule( ActiveObject* active_object )
{
	float_t radius = std::max( active_object->get_collision_width(), active_object->get_collision_depth() ) / 2.f;
	float_t height = active_object->get_collision_height() - ( radius * 2.f );

	Transform offset;
	offset.set_identity();
	offset.set_position( Vector( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBodyOwner rigid_body = add_capsule_rigid_body( active_object->get_transform(), offset, radius, height, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body.get(), active_object );

	return rigid_body;
}

/**
 * ActiveObject をシリンダー形状の剛体として物理空間に追加する
 *
 * @param active_object ActiveObject 
 */
ActiveObjectPhysics::RigidBodyOwner ActiveObjectPhysics::add_active_object_as_cylinder( ActiveObject* active_object )
{
	Vector box( active_object->get_collision_width() / 2.f, active_object->get_collision_height() / 2.f, active_object->get_collision_depth() / 2.f );

	Transform offset;
	offset.set_identity();
	offset.set_position( Vector( 0, active_object->get_collision_height() / 2, 0 ) );

	RigidBodyOwner rigid_body = add_cylinder_rigid_body( active_object->get_transform(), offset, box, active_object->get_default_mass() );
	set_active_object_info_to_rigid_body( rigid_body.get(), active_object );

	return rigid_body;
}

/**
 * ActiveObject の情報を RigidBody に設定する
 *
 * userPointer の Entity の番号のタグは ActiveObject が付ける（剛体と Entity の両方が揃ったとき。ActiveObject::set_rigid_body・attach_entity）。
 * 以前はここで userPointer に ActiveObject* を入れていた
 *
 * @param rigid_body RigidBody
 * @param active_object ActiveObject
 */
void ActiveObjectPhysics::set_active_object_info_to_rigid_body( RigidBody* rigid_body, ActiveObject* active_object )
{
	if ( ! active_object->is_hard() )
	{
		rigid_body->setCollisionFlags( rigid_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
}

namespace
{

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

/// 接した組を集める（以前はここで userPointer を ActiveObject* にキャストし、その場で on_collide_with を呼んでいた）
class CollectContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	ecs::CollisionEventSystem& events_;

public:
	explicit CollectContactResultCallback( ecs::CollisionEventSystem& events ) : events_( events ) { }

	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper* o0, int, int, const btCollisionObjectWrapper* o1, int, int )
	{
		events_.add_contact( o0->getCollisionObject(), o1->getCollisionObject() );

		return 0;
	}
};

} // namespace

bool ActiveObjectPhysics::is_collision( ActiveObject* a, ActiveObject* b )
{
	ContactResultCallback callback;
	dynamics_world_->contactPairTest( a->get_rigid_body(), b->get_rigid_body(), callback );

	return callback.is_hit();
}

bool ActiveObjectPhysics::is_collision( btCollisionObject* a, btCollisionObject* b )
{
	if ( ! a || ! b )
	{
		return false;
	}

	ContactResultCallback callback;
	dynamics_world_->contactPairTest( a, b, callback );

	return callback.is_hit();
}

void ActiveObjectPhysics::collect_contacts_with( ActiveObject* a, ecs::CollisionEventSystem& events )
{
	CollectContactResultCallback callback( events );
	dynamics_world_->contactTest( a->get_rigid_body(), callback );
}

void ActiveObjectPhysics::collect_contacts_with( btCollisionObject* a, ecs::CollisionEventSystem& events )
{
	CollectContactResultCallback callback( events );
	dynamics_world_->contactTest( a, callback );
}

void ActiveObjectPhysics::collect_all_contacts( ecs::CollisionEventSystem& events )
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
				events.add_contact( o0, o1 );
			}
		}
	}
}

}
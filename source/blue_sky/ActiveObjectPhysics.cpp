#include "ActiveObjectPhysics.h"

#include <algorithm>

namespace blue_sky
{

ActiveObjectPhysics::ActiveObjectPhysics()
{

}

ActiveObjectPhysics::~ActiveObjectPhysics()
{

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
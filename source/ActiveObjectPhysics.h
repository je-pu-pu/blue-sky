#ifndef BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H
#define BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H

#include "BulletPhysics.h"
#include "ActiveObject.h"

namespace blue_sky
{

class ActiveObject;

/**
 * ActiveObject ‘Î‰ž BulletPhysics
 *
 */
class ActiveObjectPhysics : public BulletPhysics
{
public:
	typedef btRigidBody RigidBody;

public:
	ActiveObjectPhysics();
	~ActiveObjectPhysics();

	RigidBody* add_active_object( ActiveObject* );
	RigidBody* add_active_object_as_capsule( ActiveObject* );
	RigidBody* add_active_object_as_cylinder( ActiveObject* );

	void set_active_object_info_to_rigid_body( RigidBody*, ActiveObject* );

	bool is_collision( ActiveObject*, ActiveObject* );
	void check_collision_with( ActiveObject* );
	void check_collision_dynamic_object();

}; // class ActiveObjectPhysics

class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
{
private:
	const btRigidBody* rigid_body_;
	bool include_soft_footing_;

public:
	ClosestNotMe( const btVector3& s, const btVector3& d, const btRigidBody* rb, bool include_soft_footing )
		: ClosestRayResultCallback( s, d )
		, rigid_body_( rb )
		, include_soft_footing_( include_soft_footing )
	{ }

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if ( rayResult.m_collisionObject == rigid_body_ )
		{
			return 1.0;
		}

		if ( rayResult.m_collisionObject && rayResult.m_collisionObject->getUserPointer() )
		{
			ActiveObject* a = reinterpret_cast< ActiveObject* >( rayResult.m_collisionObject->getUserPointer() );
			
			if ( a->is_dead() )
			{
				return 1.0;
			}

			if ( ! include_soft_footing_ && ! a->is_hard() )
			{
				return 1.0;
			}
		}

		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

class ClosestNotMeAndHim : public ClosestNotMe
{
private:
	const btRigidBody* him_;

public:
	ClosestNotMeAndHim( const btVector3& s, const btVector3& d, const btRigidBody* rb, const btRigidBody* him, bool include_soft_footing )
		: ClosestNotMe( s, d, rb, include_soft_footing )
		, him_( him )
	{ }

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if ( rayResult.m_collisionObject == him_ )
		{
			return 1.0;
		}

		return ClosestNotMe::addSingleResult( rayResult, normalInWorldSpace );
	}
};

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H
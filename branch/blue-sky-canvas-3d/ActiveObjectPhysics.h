#ifndef BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H
#define BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H

#include "BulletPhysics.h"

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

	RigidBody* add_active_object( const ActiveObject* );
	RigidBody* add_active_object_as_cylinder( const ActiveObject* );

	bool is_collision( ActiveObject*, ActiveObject* );

}; // class ActiveObjectPhysics

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H
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

	RigidBody* add_active_object( ActiveObject* );
	RigidBody* add_active_object_as_cylinder( ActiveObject* );

	bool is_collision( ActiveObject*, ActiveObject* );
	void check_collision_with( ActiveObject* );
	void check_collision_dynamic_object();

}; // class ActiveObjectPhysics

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_PHYSICS_H
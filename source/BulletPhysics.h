#ifndef BULLET_PHYSICS_H
#define BULLET_PHYSICS_H

#include "type.h"
#include <btBulletDynamicsCommon.h>

/**
 * Bullet のラッパークラス
 *
 */
class BulletPhysics
{
public:
	typedef btAlignedObjectArray< btCollisionShape* > CollisionShapeArray;
	typedef btVector3						Vector3;
	typedef btTransform						Transform;

protected:
	btDefaultCollisionConfiguration*		collision_configuration_;
	btCollisionDispatcher*					collision_dispatcher_;
	btBroadphaseInterface*					broadphase_interface_;
	btSequentialImpulseConstraintSolver*	constraint_solver_;
	btDiscreteDynamicsWorld*				dynamics_world_;

	CollisionShapeArray						collision_shape_list_;

public:
	BulletPhysics();
	~BulletPhysics();

	void clear();

	bool load_obj( const char_t* );

	btRigidBody* add_ground_rigid_body( const Vector3& );
	btRigidBody* add_box_rigid_body( const Transform&, const Transform&, const Vector3&, bool = false );
	btRigidBody* add_cylinder_rigid_body( const Transform&, const Transform&, const Vector3&, bool = false );

	void update( float = 1.f / 60.f );
	
	btDynamicsWorld* get_dynamics_world() { return dynamics_world_; }

	// for debug
	void setDebugDrawer( btIDebugDraw* );

	void setConstraint();

}; // class BulletPhysics

#endif // BULLET_PHYSICS_H
#ifndef BULLET_PHYSICS_H
#define BULLET_PHYSICS_H

#include <btBulletDynamicsCommon.h>

/**
 * Bullet のラッパークラス
 *
 */
class BulletPhysics
{
public:
	typedef btAlignedObjectArray< btCollisionShape* > CollisionShapeArray;

private:
	btDefaultCollisionConfiguration*		collision_configuration_;
	btCollisionDispatcher*					collision_dispatcher_;
	btBroadphaseInterface*					broadphase_interface_;
	btSequentialImpulseConstraintSolver*	constraint_solver_;
	btDiscreteDynamicsWorld*				dynamics_world_;

	CollisionShapeArray						collision_shape_list_;

public:
	BulletPhysics();
	~BulletPhysics();


	void update( float = 1.f / 60.f );

	// ???
	const btTransform& BulletPhysics::getTransform() const;
	
	// for debug
	void setDebugDrawer( btIDebugDraw* );

}; // class BulletPhysics

#endif // BULLET_PHYSICS_H
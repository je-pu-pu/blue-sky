#pragma once

#include <core/math/Transform.h>
#include <core/math/Vector.h>
#include <type/type.h>
#include <btBulletDynamicsCommon.h>
#include <list>

/**
 * Bullet のラッパークラス
 *
 * @todo 複数の RigidBody で同じ CollisionShape を共有できるようにする
 */
class BulletPhysics
{
public:
	using Vector				= core::Vector;
	using Transform				= core::Transform;

	using CollisionShapeArray	= btAlignedObjectArray< btCollisionShape* >;
	using MeshList				= std::list< btStridingMeshInterface* >;

protected:
	btDefaultCollisionConfiguration*		collision_configuration_;
	btCollisionDispatcher*					collision_dispatcher_;
	btBroadphaseInterface*					broadphase_interface_;
	btSequentialImpulseConstraintSolver*	constraint_solver_;
	btDiscreteDynamicsWorld*				dynamics_world_;

	CollisionShapeArray						collision_shape_list_;
	MeshList								mesh_list_;

	btRigidBody* create_rigid_body( btCollisionShape*, const Transform&, const Transform&, float_t );

	BulletPhysics( const BulletPhysics& ) = delete;
	BulletPhysics& operator = ( const BulletPhysics& ) = delete;

public:
	BulletPhysics();
	~BulletPhysics();

	void clear();

	bool load_obj( const char_t* );

	btRigidBody* add_ground_rigid_body( const Vector& );
	btRigidBody* add_box_rigid_body( const Transform&, const Transform&, const Vector&, float_t mass );
	btRigidBody* add_capsule_rigid_body( const Transform&, const Transform&, float_t, float_t, float_t mass );
	btRigidBody* add_cylinder_rigid_body( const Transform&, const Transform&, const Vector&, float_t mass );

	void update( float = 1.f / 60.f );
	
	/// @todo 削除する
	btDynamicsWorld* get_dynamics_world() { return dynamics_world_; }

	// for debug
	void setDebugDrawer( btIDebugDraw* );

	void setConstraint();

}; // class BulletPhysics

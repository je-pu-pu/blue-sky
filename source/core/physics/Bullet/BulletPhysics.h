#pragma once

#include <core/math/Transform.h>
#include <core/math/Vector.h>
#include <type/type.h>
#include <btBulletDynamicsCommon.h>
#include <functional>
#include <list>
#include <map>
#include <tuple>

/**
 * Bullet のラッパークラス
 *
 * 同じ種類・大きさの CollisionShape は複数の RigidBody で共有する（shape_cache_）
 */
class BulletPhysics
{
public:
	using Vector				= core::math::Vector;
	using Transform				= core::math::Transform;

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

	/// 形の種類と大きさ → 共有する形（持ち主は collision_shape_list_）
	using ShapeKey = std::tuple< int, btScalar, btScalar, btScalar >;
	std::map< ShapeKey, btCollisionShape* >	shape_cache_;

	/// 同じ種類・大きさの形があれば返し、無ければ make で作って登録する
	btCollisionShape* find_or_create_shape( const ShapeKey& key, const std::function< btCollisionShape*() >& make );

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
	
	/// 線分で当たりを調べる（以前は get_dynamics_world で世界そのものを出していた）
	void ray_test( const btVector3& from, const btVector3& to, btCollisionWorld::RayResultCallback& callback ) const { dynamics_world_->rayTest( from, to, callback ); }

	/// 物体と接しているものを調べる
	void contact_test( btCollisionObject* object, btCollisionWorld::ContactResultCallback& callback ) const { dynamics_world_->contactTest( object, callback ); }

	// for debug
	void setDebugDrawer( btIDebugDraw* );

	void setConstraint();

}; // class BulletPhysics

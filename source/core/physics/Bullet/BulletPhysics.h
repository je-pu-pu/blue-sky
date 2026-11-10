#pragma once

#include <core/math/Transform.h>
#include <core/math/Vector.h>
#include <core/physics/RigidBody.h>
#include <type/type.h>
#include <btBulletDynamicsCommon.h>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

/**
 * Bullet のラッパークラス
 *
 * 同じ種類・大きさの CollisionShape は複数の RigidBody で共有する（shape_cache_）
 *
 * 剛体の持ち主 :
 * - 物体の剛体（add_box_rigid_body など）は、持ち主（core::physics::RigidBody）を返す。受け取った側（物体の部品）が持ち、
 *   壊れるときに世界から外れる。以前は世界が持ち、物体を壊しても世界に残って clear でまとめて消していた
 * - 地面・ステージの当たりの形（add_ground_rigid_body・load_obj）は、これが持つ（clear で消す）
 * 持ち主はこれ（世界）より先に壊れること
 */
class BulletPhysics
{
public:
	using Vector				= core::math::Vector;
	using Transform				= core::math::Transform;
	using RigidBodyOwner		= core::physics::RigidBody;

	using CollisionShapeArray	= btAlignedObjectArray< btCollisionShape* >;
	using MeshList				= std::list< btStridingMeshInterface* >;

protected:
	// 以前は生の new / delete。宣言の逆の順に壊れる（世界 → 解く物 → 広い判定 → 振り分け → 設定。以前の delete と同じ順）
	std::unique_ptr< btDefaultCollisionConfiguration >		collision_configuration_;
	std::unique_ptr< btCollisionDispatcher >				collision_dispatcher_;
	std::unique_ptr< btBroadphaseInterface >				broadphase_interface_;
	std::unique_ptr< btSequentialImpulseConstraintSolver >	constraint_solver_;
	std::unique_ptr< btDiscreteDynamicsWorld >				dynamics_world_;

	CollisionShapeArray						collision_shape_list_;
	MeshList								mesh_list_;

	/// これが持つ剛体（地面・ステージの当たりの形）
	std::vector< RigidBodyOwner >			owned_rigid_body_list_;

	/// 形の種類と大きさ → 共有する形（持ち主は collision_shape_list_）
	using ShapeKey = std::tuple< int, btScalar, btScalar, btScalar >;
	std::map< ShapeKey, btCollisionShape* >	shape_cache_;

	/// 同じ種類・大きさの形があれば返し、無ければ make で作って登録する
	btCollisionShape* find_or_create_shape( const ShapeKey& key, const std::function< btCollisionShape*() >& make );

	/// 剛体を作って世界に入れ、持ち主を返す（形は共有するので持ち主は持たない）
	RigidBodyOwner create_rigid_body( btCollisionShape*, const Transform&, const Transform&, float_t );

	BulletPhysics( const BulletPhysics& ) = delete;
	BulletPhysics& operator = ( const BulletPhysics& ) = delete;

public:
	BulletPhysics();
	~BulletPhysics();

	/**
	 * これが持つ剛体・形・制約を消す（シーンの終わり）
	 *
	 * 物体の剛体は持ち主（物体の部品）が消すので、先に物体を壊しておくこと。残っていたら世界から外すだけで消さない
	 * （以前は世界に残った剛体を全部消していた。今それをすると、持ち主が後で二重に消す）
	 */
	void clear();

	bool load_obj( const char_t* );

	/// 地面の剛体を足す（これが持つ。clear で消える）
	btRigidBody* add_ground_rigid_body( const Vector& );

	RigidBodyOwner add_box_rigid_body( const Transform&, const Transform&, const Vector&, float_t mass );
	RigidBodyOwner add_capsule_rigid_body( const Transform&, const Transform&, float_t, float_t, float_t mass );
	RigidBodyOwner add_cylinder_rigid_body( const Transform&, const Transform&, const Vector&, float_t mass );

	/// 世界にある剛体（当たりの物）の数（テスト用）
	int get_rigid_body_count() const { return dynamics_world_->getNumCollisionObjects(); }

	void update( float = 1.f / 60.f );

	/// 線分で当たりを調べる（以前は get_dynamics_world で世界そのものを出していた）
	void ray_test( const btVector3& from, const btVector3& to, btCollisionWorld::RayResultCallback& callback ) const { dynamics_world_->rayTest( from, to, callback ); }

	/// 物体と接しているものを調べる
	void contact_test( btCollisionObject* object, btCollisionWorld::ContactResultCallback& callback ) const { dynamics_world_->contactTest( object, callback ); }

	// for debug
	void setDebugDrawer( btIDebugDraw* );

	void setConstraint();

}; // class BulletPhysics

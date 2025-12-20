#pragma once

#include <core/math/Vector.h>
#include <core/math/Transform.h>
#include <core/math/Quaternion.h>
#include <common/Singleton.h>

class btDynamicsWorld;
class btIDebugDraw;
class btRigidBody;
class btCollisionShape;
class btMotionState;

namespace core::physics
{

/**
 * 剛体の形状タイプ
 */
enum class RigidBodyShapeType
{
	Box,
	Capsule,
	Cylinder,
	Sphere,
};

/**
 * 剛体の生成パラメータ
 */
struct RigidBodyCreateInfo
{
	using Vector = math::Vector;
	using Transform = math::Transform;

	RigidBodyShapeType shape_type = RigidBodyShapeType::Box;
	Vector shape_size = Vector( 0.5f, 0.5f, 0.5f );
	float mass = 1.f;
	Transform transform;
	Transform offset;
};

/**
 * 剛体ハンドル
 *
 * 物理マネージャーが管理する剛体への参照
 */
struct RigidBodyHandle
{
	btRigidBody* rigid_body = nullptr;
	btCollisionShape* collision_shape = nullptr;
	btMotionState* motion_state = nullptr;

	bool is_valid() const { return rigid_body != nullptr; }
};

/**
 * 物理マネージャー
 *
 * 物理シミュレーションを管理する。
 * Bullet Physics の詳細を隠蔽し、抽象的なインターフェースを提供する。
 */
class PhysicsManager : public common::Singleton<PhysicsManager>
{
public:
	using Vector = math::Vector;
	using Transform = math::Transform;

private:
	class Impl;
	Impl* impl_ = nullptr;

public:
	PhysicsManager();
	~PhysicsManager();

	// コピー禁止
	PhysicsManager( const PhysicsManager& ) = delete;
	PhysicsManager& operator=( const PhysicsManager& ) = delete;

	/**
	 * 剛体を生成してワールドに追加する
	 */
	RigidBodyHandle create_rigid_body( const RigidBodyCreateInfo& info );

	/**
	 * 剛体をワールドから削除して破棄する
	 */
	void destroy_rigid_body( RigidBodyHandle& handle );

	/**
	 * 剛体の位置・回転を取得する
	 */
	void get_rigid_body_transform( const RigidBodyHandle& handle, Vector& position, math::Quaternion& rotation ) const;

	/**
	 * シミュレーションを1ステップ進める
	 */
	void step_simulation();
	void step_simulation( float time_step );

	/**
	 * 重力を設定する
	 */
	void set_gravity( const Vector& gravity );

	/**
	 * 重力を取得する
	 */
	Vector get_gravity() const;

	/**
	 * タイムステップを設定する
	 */
	void set_time_step( float time_step );

	/**
	 * タイムステップを取得する
	 */
	float get_time_step() const;

	/**
	 * デバッグ描画を設定する
	 */
	void set_debug_drawer( btIDebugDraw* drawer );

	/**
	 * dynamics_world を取得する (内部用)
	 */
	btDynamicsWorld* get_dynamics_world();

}; // class PhysicsManager

} // namespace core::physics

#pragma once

#include <core/math/Vector.h>
#include <core/math/Transform.h>
#include <core/math/Quaternion.h>
#include <common/Singleton.h>
#include <vector>
#include <functional>

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
 * 衝突グループ (ビットフラグ)
 */
namespace CollisionGroup
{
	constexpr short Default  = 1 << 0;  // 地面、建物など
	constexpr short Player   = 1 << 1;  // プレイヤー
	constexpr short Balloon  = 1 << 2;  // 風船
	constexpr short All      = 0x7FFF;  // 全て
}

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

	short collision_group = CollisionGroup::Default;  // 自身の衝突グループ
	short collision_mask = CollisionGroup::All;       // 衝突対象のマスク
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
	void* user_pointer = nullptr;  // Entity へのポインタなど

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

	/**
	 * 剛体の線形速度を設定する
	 */
	void set_rigid_body_linear_velocity( const RigidBodyHandle& handle, const Vector& velocity );

	/**
	 * 剛体の線形速度を取得する
	 */
	Vector get_rigid_body_linear_velocity( const RigidBodyHandle& handle ) const;

	/**
	 * 剛体の角速度係数を設定する (回転の制限)
	 * @param factor 各軸の係数 (0 = 回転なし, 1 = 通常)
	 */
	void set_rigid_body_angular_factor( const RigidBodyHandle& handle, const Vector& factor );

	/**
	 * 剛体にインパルスを適用する (ジャンプ用)
	 */
	void apply_impulse( const RigidBodyHandle& handle, const Vector& impulse );

	/**
	 * 剛体をアクティブ化する (スリープ解除)
	 */
	void activate_rigid_body( const RigidBodyHandle& handle );

	/**
	 * レイキャストを行う (接地判定用)
	 * @param from 開始点
	 * @param to 終点
	 * @return ヒットした場合 true
	 */
	bool ray_test( const Vector& from, const Vector& to ) const;

	/**
	 * レイキャストを行う (自分自身を除外)
	 * @param from 開始点
	 * @param to 終点
	 * @param exclude 除外する剛体
	 * @return ヒットした場合 true
	 */
	bool ray_test_excluding( const Vector& from, const Vector& to, const RigidBodyHandle& exclude ) const;

	/**
	 * 2つの剛体が接触しているかを調べる
	 * @param a 剛体A
	 * @param b 剛体B
	 * @return 接触している場合 true
	 */
	bool check_contact( const RigidBodyHandle& a, const RigidBodyHandle& b ) const;

	/**
	 * 指定した衝突グループ間の全ての衝突を処理する
	 * @param group_a 衝突グループA
	 * @param group_b 衝突グループB
	 * @param callback 衝突時に呼ばれるコールバック (user_pointer_a, user_pointer_b)
	 */
	void for_each_contact( short group_a, short group_b,
		const std::function< void( void*, void* ) >& callback ) const;

}; // class PhysicsManager

} // namespace core::physics

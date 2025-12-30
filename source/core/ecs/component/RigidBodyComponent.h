#pragma once

#include <core/ecs/Component.h>
#include <core/physics/PhysicsManager.h>

namespace core::ecs
{

/**
 * 剛体コンポーネント
 *
 * 物理演算用の剛体情報を保持する。
 * PhysicsSystem が Entity に追加された RigidBodyComponent を検知し、
 * PhysicsManager を通じて剛体を生成・管理する。
 */
struct RigidBodyComponent : public Component
{
	using Vector = core::math::Vector;
	using RigidBodyShapeType = core::physics::RigidBodyShapeType;
	using RigidBodyHandle = core::physics::RigidBodyHandle;

	/// 形状タイプ
	RigidBodyShapeType shape_type = RigidBodyShapeType::Box;

	/// 形状サイズ (Box/Cylinder: x, y, z の半分のサイズ、Capsule: x=radius, y=height、Sphere: x=radius)
	Vector shape_size = Vector( 0.5f, 0.5f, 0.5f );

	/// 剛体の質量 (0 の場合は静的オブジェクト)
	float mass = 1.f;

	/// 剛体の中心点のオフセット
	Vector offset = Vector( 0.f, 0.f, 0.f );

	/// 角速度係数 (各軸の回転を制限。0 = 回転なし、1 = 通常)
	Vector angular_factor = Vector( 1.f, 1.f, 1.f );

	/// 摩擦係数
	float friction = 0.5f;

	/// 衝突グループ (自身が属するグループ)
	short collision_group = core::physics::CollisionGroup::Default;

	/// 衝突マスク (衝突対象のグループ)
	short collision_mask = core::physics::CollisionGroup::All;

	/// 剛体ハンドル (PhysicsSystem によって設定される)
	RigidBodyHandle handle;

}; // struct RigidBodyComponent

} // namespace core::ecs

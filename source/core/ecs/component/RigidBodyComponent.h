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

	/// 剛体ハンドル (PhysicsSystem によって設定される)
	RigidBodyHandle handle;

}; // struct RigidBodyComponent

} // namespace core::ecs

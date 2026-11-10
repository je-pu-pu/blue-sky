#pragma once

#include <core/ecs/Component.h>
#include <core/physics/RigidBody.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 本編の物体の剛体（物理の世界は core::physics::PhysicsManager）
 *
 * 剛体の持ち主（core::physics::RigidBody。ムーブだけできる RAII）を持ち、Entity が消える・部品が外れると剛体も世界から外れて消える。
 * 以前は世界が持ち、物体を壊しても剛体が世界に残って、シーンの終わり（ActiveObjectPhysics::clear）にまとめて消していた。
 *
 * PhysicsSyncSystem が、物理を進めた後に剛体の位置と回転を TransformComponent へ写す（位置の正は TransformComponent）。
 * 以前は複製（ActiveObject::clone）が剛体を元の物体と共有するために、持ち主とは別に Bullet の剛体のポインタを持っていた
 * （複製できる種類は段階 6 で無くなったので、持ち主だけにした）
 */
struct PhysicsBodyComponent : public core::ecs::Component
{
	core::physics::RigidBody	owner;					///< 剛体の持ち主（無ければ空。ステージを読む前のプレイヤー）
	float_t						height_offset = 0.f;	///< 物体の位置（足元）から剛体の中心までの高さ（GameObject::get_height_offset。種類ごとの定数）

	/// 剛体（無ければ nullptr）
	core::physics::RigidBody* get() { return owner.is_valid() ? & owner : nullptr; }
	const core::physics::RigidBody* get() const { return owner.is_valid() ? & owner : nullptr; }
};

} // namespace blue_sky::ecs

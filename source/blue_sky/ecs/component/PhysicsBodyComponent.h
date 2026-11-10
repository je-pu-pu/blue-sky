#pragma once

#include <core/ecs/Component.h>
#include <core/physics/RigidBody.h>
#include <core/type.h>

class btRigidBody;

namespace blue_sky::ecs
{

/**
 * 本編の物理の世界（ActiveObjectPhysics）の剛体
 *
 * 剛体の持ち主（core::physics::RigidBody。ムーブだけできる RAII）を持ち、Entity が消える・部品が外れると剛体も世界から外れて消える。
 * 以前は世界が持ち、物体を壊しても剛体が世界に残って、シーンの終わり（ActiveObjectPhysics::clear）にまとめて消していた。
 *
 * PhysicsSyncSystem が、物理を進めた後に剛体の位置と回転を TransformComponent へ写す（位置の正は TransformComponent）。
 * 複製（ActiveObject::clone）は剛体を元の物体と共有する（以前と同じ）ので、持ち主を持たず body だけを持つ
 */
struct PhysicsBodyComponent : public core::ecs::Component
{
	core::physics::RigidBody	owner;					///< 剛体の持ち主（共有するだけの複製なら空）
	btRigidBody*				body = nullptr;			///< 位置を写す剛体（持ち主の剛体か、元の物体と共有する剛体）
	float_t						height_offset = 0.f;	///< 物体の位置（足元）から剛体の中心までの高さ（GameObject::get_height_offset。種類ごとの定数）
};

} // namespace blue_sky::ecs

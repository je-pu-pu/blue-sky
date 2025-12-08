#pragma once

#include <core/ecs/Component.h>
#include <core/physics/RigidBody.h>

namespace core::ecs
{

/**
 * 剛体コンポーネント
 *
 */
class RigidBodyComponent : public Component
{
public:
	core::physics::RigidBody rigid_body;

}; // class TransformComponent

} // namespace core::ecs

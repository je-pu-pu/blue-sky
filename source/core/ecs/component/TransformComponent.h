#pragma once

#include <core/ecs/Component.h>
#include <core/math/Transform.h>

namespace core::ecs
{

/**
 * トランスフォームコンポーネント
 *
 * 位置・回転・
 */
class TransformComponent : public Component
{
public:
	core::Transform transform;

}; // class TransformComponent

} // namespace core::ecs

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
	// const size_t ID = 2;

public:
	core::Transform transform;

public:
	TransformComponent() = default;
	~TransformComponent() = default;

}; // class TransformComponent

} // namespace core::ecs

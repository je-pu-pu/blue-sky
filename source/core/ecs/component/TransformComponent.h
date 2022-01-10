#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>
#include <array>

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
	core::Vector3 position;

public:
	TransformComponent() = default;
	~TransformComponent() = default;

}; // class TransformComponent

} // namespace core::ecs

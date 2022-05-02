#pragma once

#include <core/ecs/Component.h>
#include <core/math/Transform.h>

namespace core::ecs
{

/**
 * �g�����X�t�H�[���R���|�[�l���g
 *
 * �ʒu�E��]�E
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

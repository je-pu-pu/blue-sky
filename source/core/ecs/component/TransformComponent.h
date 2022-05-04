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
	core::Transform transform;

}; // class TransformComponent

} // namespace core::ecs

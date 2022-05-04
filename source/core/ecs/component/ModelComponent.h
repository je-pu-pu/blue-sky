#pragma once

#include <core/ecs/Component.h>
#include <core/graphics/Model.h>

namespace core::ecs
{

/**
 * �`��p�̃��f����ێ�����R���|�[�l���g
 *
 */
class ModelComponent : public Component
{
public:
	graphics::Model* model = nullptr;


}; // class ModelComponent

} // namespace core::ecs

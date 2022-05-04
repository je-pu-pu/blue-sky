#pragma once

#include <core/ecs/Component.h>
#include <core/graphics/Model.h>

namespace core::ecs
{

/**
 * 描画用のモデルを保持するコンポーネント
 *
 */
class ModelComponent : public Component
{
public:
	graphics::Model* model = nullptr;


}; // class ModelComponent

} // namespace core::ecs

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
	/// 描くモデル（ハンドルで持ち、描く側が resolve_model で引く。以前は生ポインタで、モデルが解放されると壊れ得た）
	graphics::ModelHandle model;


}; // class ModelComponent

} // namespace core::ecs

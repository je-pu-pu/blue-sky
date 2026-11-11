#pragma once

#include <core/ecs/Component.h>
#include <core/math/Transform.h>
#include <core/math/Vector.h>

namespace core::ecs
{

/**
 * トランスフォームコンポーネント
 *
 * 位置・回転・スケール。描画（RenderSystem）は scale も反映する。
 */
class TransformComponent : public Component
{
public:
	core::math::Transform transform;

	/// 描画スケール（既定は等倍）。box などの基本メッシュを任意サイズに拡大するのに使う。
	core::math::Vector scale = core::math::Vector( 1.f, 1.f, 1.f );

}; // class TransformComponent

} // namespace core::ecs

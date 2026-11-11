#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * 視線を遮る煙のボリューム（球）コンポーネント。
 *
 * SmokeRegistrySystem が位置＋半径を集め、PerceptionSystem の視覚 LOS 判定で
 * 「視線がこの球を通ると見えない」ために使う（煙玉＝視界を奪う道具）。
 * 聴覚(hears_noise)は遮らない。
 */
struct SmokeComponent : public core::ecs::Component
{
	float radius = 3.f;   ///< 煙球の半径 (m)

}; // struct SmokeComponent

} // namespace blue_sky::ecs

#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * 風船コンポーネント
 *
 * プレイヤーが触れると浮力を与える
 */
struct BalloonComponent : public core::ecs::Component
{
	float lift_force = 500.f;    // 浮力の強さ
	float radius = 1.0f;         // 風船の半径 (衝突判定用)
};

} // namespace blue_sky::ecs

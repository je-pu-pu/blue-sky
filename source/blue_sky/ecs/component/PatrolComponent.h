#pragma once

#include <core/ecs/Component.h>
#include <core/math/Vector.h>

#include <vector>
#include <cstddef>

namespace blue_sky::ecs
{

/**
 * 敵のパトロール コンポーネント。
 *
 * ウェイポイント列を巡回する（navmesh は将来対応。現状は平面の直線移動）。
 * 警戒 System が捜索/警報にした場合は、パトロール System が最終視認地点へ向かう
 * 挙動に切り替える（移動ロジックは PatrolSystem 側）。
 */
struct PatrolComponent : public core::ecs::Component
{
	/// 巡回するウェイポイント（ワールド座標。XZ 平面で扱う）
	std::vector< core::math::Vector > waypoints;

	/// 現在の目標ウェイポイントの添字
	std::size_t current = 0;

	/// 移動速度 (m/s)
	float move_speed = 2.f;

	/// 向きの補間速度 (rad/s)。進行方向へなめらかに向き直る。
	float turn_speed = 3.f;

	/// ウェイポイント到達とみなす半径 (m)
	float arrive_radius = 0.5f;

	/// 各ウェイポイントでの待機時間 (s)
	float wait_time = 1.f;

	/// 待機の残り時間 (s)
	float wait_timer = 0.f;

}; // struct PatrolComponent

} // namespace blue_sky::ecs

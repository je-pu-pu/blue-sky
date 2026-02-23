#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * FPS プレイヤー用コンポーネント
 *
 * FPS スタイルの移動・視点制御に必要なデータを保持する。
 */
struct FpsPlayerComponent : public core::ecs::Component
{
	/// 視点の Y 軸回転角度 (radian)
	float yaw = 0.f;

	/// 視点の X 軸回転角度 (radian) [-PI/2, PI/2]
	float pitch = 0.f;

	/// 移動速度 (m/s)
	float move_speed = 5.f;

	/// ダッシュ時の移動速度倍率
	float sprint_multiplier = 2.f;

	/// 目の高さ (カプセルの中心からのオフセット)
	float eye_height = 0.5f;

	/// ジャンプ力 (インパルス)
	float jump_impulse = 5.f;

	/// 接地フラグ (ジャンプ判定用)
	bool is_grounded = false;

}; // struct FpsPlayerComponent

} // namespace blue_sky::ecs

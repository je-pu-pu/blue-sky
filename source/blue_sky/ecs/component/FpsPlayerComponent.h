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

	// --- ジェット（滞空・飛ぶ動詞の第一歩）。fly_enabled のときだけ有効 ---
	bool  fly_enabled   = false;  ///< 空中で JUMP 長押しの上昇を許可するか
	float fly_speed     = 4.f;    ///< 上昇速度 (m/s)
	float fly_fuel      = 1.f;    ///< 燃料 [0,1]
	float fly_fuel_max  = 1.f;    ///< 燃料上限
	float fly_burn_rate = 0.5f;   ///< 上昇中の消費 (/s)。0.5 で満タン 2 秒
	float fly_regen_rate = 0.4f;  ///< 接地時の回復 (/s)

}; // struct FpsPlayerComponent

} // namespace blue_sky::ecs

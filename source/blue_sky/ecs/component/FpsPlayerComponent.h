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

	// --- 落下（高さのコスト）。物理的な事実だけを持ち、ダメージ化はゲーム側の判断 ---
	float fall_speed     = 0.f;   ///< 現在の落下速度 (m/s, 下向きが正)。上昇中・接地中は 0
	float landing_impact = 0.f;   ///< 着地したフレームだけ直前の落下速度 (m/s)。それ以外は 0

	// --- 傘（降りる動詞）。開いている間、落ちる速さだけを終端速度に抑える ---
	bool  umbrella_open       = false; ///< 傘を開いているか（入力・所持数の管理はゲーム側）
	float umbrella_fall_speed = 3.f;   ///< 傘を開いている間の最大落下速度 (m/s)

}; // struct FpsPlayerComponent

} // namespace blue_sky::ecs

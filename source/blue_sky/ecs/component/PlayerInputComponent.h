#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * プレイヤーへの 1 フレームぶんの入力（ボタン・マウス・ホイール）
 *
 * シーン（GamePlayScene）が毎フレーム入力（Input）から書き、プレイヤーの操作（PlayerInputSystem）が読む。
 * 以前は GamePlayScene::update_main が Input を直接読みながらプレイヤーの関数を呼んでいた。
 * 入力を値にしたので、開発用の確かめ（BLUE_SKY_DEBUG_PLAYER_SCRIPT）は決まった入力の並びを書き、テストは偽の入力を書ける
 */
struct PlayerInputComponent : public core::ecs::Component
{
	// 押している間（Input::press）
	bool		left = false;
	bool		right = false;
	bool		up = false;
	bool		down = false;
	bool		a = false;
	bool		jump = false;

	// 押した瞬間（Input::push）
	bool		a_pushed = false;
	bool		jump_pushed = false;

	float_t		mouse_dx = 0.f;				///< マウスの横の移動量（Input::get_mouse_dx）
	float_t		mouse_dy = 0.f;				///< マウスの縦の移動量（Input::get_mouse_dy）
	int			wheel = 0;					///< マウスホイール（+ : 前 / - : 後ろ。Input::pop_mouse_wheel_queue。プレイヤーが死んでいる間は取り出さない = 以前と同じ）
	float_t		hmd_delta_yaw_degree = 0.f;	///< ヘッドマウントディスプレイの向きの変化（度。無ければ 0）

	/// 全部を押していない・動かしていない状態にする
	void clear() { * this = PlayerInputComponent(); }
};

} // namespace blue_sky::ecs

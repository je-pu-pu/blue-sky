#pragma once

namespace blue_sky
{

/// そのフレームの入力を受け取る先
enum class InputOwner
{
	CONSOLE,		///< 開発用のコンソール（ImGui）。UI にもゲームにも渡さない
	OVERLAY,		///< 一番上のオーバーレイ（UI 画面）
	BASE_SCENE		///< ベースシーン（ゲーム）
};

/**
 * 入力を受け取る先を決める（優先順位 : コンソール → オーバーレイ → ベースシーン）
 *
 * 以前は GameMain が更新の前後で Input の「ブロック」を付けたり外したりして決めていて、
 * 優先順位がどこにも書かれていなかった（設計上の問題 B27）。
 *
 * @param console_captures コンソール（ImGui）がキーボードかマウスを受け取っている
 * @param has_overlay オーバーレイ（UI 画面）が開いている
 */
inline InputOwner decide_input_owner( bool console_captures, bool has_overlay )
{
	if ( console_captures )
	{
		return InputOwner::CONSOLE;
	}

	return has_overlay ? InputOwner::OVERLAY : InputOwner::BASE_SCENE;
}

} // namespace blue_sky

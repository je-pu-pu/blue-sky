//**********************************************************
/**
 * Input のインターフェイス
 *
 * copyright (c) je-pu-pu all rights reserved
 *
 * @file		Input.h
 * @date		2006/09/12
 * @author		JE
 */
//**********************************************************


#ifndef BLUE_SKY_INPUT_H
#define BLUE_SKY_INPUT_H

#include <deque>
#include <algorithm>
#include <windows.h>

namespace blue_sky
{

/**
 * 入力
 */
class Input
{
public:
	/**
	 * ボタンの定義
	 *
	 * 上下左右と A, B, X, Y のボタンを持つ
	 */
	enum Button { LEFT, RIGHT, UP, DOWN, A, B, X, Y, L, R, MAX_BUTTONS, NONE };

	typedef std::deque< Button > ButtonStack;

private:
	unsigned int state_[ MAX_BUTTONS ];						///< 全てのボタンの状態
	ButtonStack allow_stack_;								///< 最優先の方向ボタン

	JOYINFOEX joy_info_;									///< ジョイスティック
	bool joystick_enabled_;									///< ジョイスティック有効フラグ

public:
	Input();

	void update();
	void update_common();

	void allow_push( Button button )
	{
		allow_stack_.push_back( button );
	}

	void allow_pop( Button button )
	{
		ButtonStack::iterator i = std::find( allow_stack_.begin(), allow_stack_.end(), button );
		allow_stack_.erase( i );
	}

	/**
	 * ボタンが押されていない状態に更新する
	 *
	 * @param button 押されていない状態にするボタン
	 */
	void clear( Button button )
	{
		state_[ button ] = 0;
	}

	/**
	 * 現在ボタンが押されているかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return 現在ボタンが押されている場合は true を、そうでない場合は false を返す
	 */
	bool press( Button button ) const
	{
		return ( state_[ button ] & 1 ) > 0;
	}

	/**
	 * たった今ボタンが押されたかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return たった今ボタンが押された場合は true を、そうでない場合は false を返す
	 */
	bool push( Button button ) const
	{
		return ( state_[ button ] & 1 ) > 0 && ( state_[ button ] & 2 ) == 0;
	}

	/**
	 * たった今ボタンが離されたかどうかを取得する
	 *
	 * button 調べるボタン
	 * @return たった今ボタンが離された場合は true を、そうでない場合は false を返す
	 */
	bool release( Button button ) const
	{
		return ( state_[ button ] & 1 ) == 0 && ( state_[ button ] & 2 ) > 0;
	}

	/**
	 * 現在最優先で押されている方向ボタンを取得する
	 *
	 * @return 現在最優先で押されている方向ボタン ( 方向ボタンが押されていない場合は NONE を返す )
	 */
	Button getPrimaryAllowButton() const
	{
		if ( allow_stack_.empty() ) return NONE;

		return allow_stack_.back();
	}

	const ButtonStack& getAllowButtonStack() const
	{
		return allow_stack_;
	}

	static const Input& GetNullInput()
	{
		static Input null_input;

		return null_input;
	}
}; // class Input

} // namespace blue_sky

#endif // BLUE_SKY_INPUT_H

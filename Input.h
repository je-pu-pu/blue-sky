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

#include <boost/array.hpp>
#include <deque>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace game
{

class Config;

} // namespace game

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

	typedef game::Config Config;

	typedef std::deque< Button > ButtonStack;
	typedef boost::array< unsigned int, MAX_BUTTONS > ButtonCodeList;

private:
	unsigned int state_[ MAX_BUTTONS ];						///< 全てのボタンの状態
	ButtonStack allow_stack_;								///< 最優先の方向ボタン

	JOYINFOEX joy_info_;									///< ジョイスティック
	bool joystick_enabled_;									///< ジョイスティック有効フラグ

	float mouse_x_;											///< マウス X 座標 ( -1.f .. 1.f )
	float mouse_y_;											///< マウス Y 座標 ( -1.f .. 1.f )
	
	float mouse_dx_;										///< マウス X 移動量 ( )
	float mouse_dy_;

	POINT last_mouse_point_;								///< 前のフレームのマウス 座標

	ButtonCodeList key_code_;
	ButtonCodeList joystick_code_;

public:
	Input();

	void load_config( Config& );

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
	Button get_primary_allow_button() const
	{
		if ( allow_stack_.empty() ) return NONE;

		return allow_stack_.back();
	}

	const ButtonStack& get_allow_button_stack() const
	{
		return allow_stack_;
	}

	static const Input& get_null_input()
	{
		static Input null_input;

		return null_input;
	}

	float get_mouse_x() const { return mouse_x_; }
	float get_mouse_y() const { return mouse_y_; }

	float get_mouse_dx() const { return mouse_dx_; }
	float get_mouse_dy() const { return mouse_dy_; }

	void set_mouse_x( float );
	void set_mouse_y( float );

}; // class Input

} // namespace blue_sky

#endif // BLUE_SKY_INPUT_H

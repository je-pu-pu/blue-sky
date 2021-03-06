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

#pragma once

#include <type/type.h>

#include <windows.h>

#include <unordered_map>
#include <deque>
#include <array>
#include <vector>
#include <algorithm>

class DirectInput;

namespace game
{

class Config;

} // namespace game

namespace blue_sky
{

/**
 * 入力
 *
 * @todo Windows と切り離す
 */
class Input
{
public:
	/**
	 * ボタンの定義
	 *
	 * 上下左右と A, B, X, Y のボタンを持つ
	 */
	enum class Button { LEFT, RIGHT, UP, DOWN, A, B, JUMP, L, R, L2, R2, ESCAPE, MAX, NONE };

	using Config				= game::Config;

	using ButtonStack			= std::deque< Button >;
	using ButtonCodeList		= std::array< unsigned int, static_cast< int >( Button::MAX ) >;
	using ButtonCodeMultiList	= std::array< std::vector< int >, static_cast< int >( Button::MAX ) >;
	using ConfigKeyCodeMap		= std::unordered_map< std::string, int >;

private:
	const DirectInput* direct_input_;						///< DirectInput

	unsigned int state_[ static_cast< int >( Button::MAX ) ];						///< 全てのボタンの状態
	ButtonStack arrow_stack_;								///< 最優先の方向ボタン

	bool joystick_enabled_;									///< ジョイスティック有効フラグ

	JOYINFOEX joy_info_;									///< ジョイスティック
	float joystick_axis_threshold_;							///< ジョイスティックの入力の閾値 ( 0.f .. 1.f )

	DWORD* joystick_x_axis_pos_;							///< ジョイスティック X 座標 
	DWORD* joystick_y_axis_pos_;							///< ジョイスティック Y 座標 

	float joystick_x_sensitivity_;							///< ジョイスティック X 座標 感度 ( default : 1.f )
	float joystick_y_sensitivity_;							///< ジョイスティック Y 座標 感度 ( default : 1.f )

	float mouse_x_sensitivity_;								///< マウス X 座標 感度 ( default : 1.f )
	float mouse_y_sensitivity_;								///< マウス X 座標 感度 ( default : 1.f )

	float mouse_x_rate_;									///< マウス X 座標 ( -1.f .. 1.f )
	float mouse_y_rate_;									///< マウス Y 座標 ( -1.f .. 1.f )
	
	float mouse_dx_;										///< マウス X 移動量
	float mouse_dy_;

	int mouse_wheel_;										///< マウスホイール ( + : front / - : back )

	POINT mouse_point_;										///< マウス 座標

	ButtonCodeMultiList key_code_;
	ButtonCodeList joystick_code_;

	ConfigKeyCodeMap config_key_code_map_;					///< コンフィグファイルのキー名 から GetAsyncKeyState() の引数に渡すキーへのマップ

	void load_key_code_config( Config&, Button, const char_t*, const char_t* );
	
	void update_state_by_key_for( uint_t );

	DWORD* get_joystick_axis_pos_pointer_by_index( int );
	float get_rate_by_joystick_axis_pos( DWORD );

public:
	Input();
	~Input();

	void set_direct_input( const DirectInput* i ) { direct_input_ = i; }
	void set_mouse_x_sensitivity( float s ) { mouse_x_sensitivity_ = s; }
	void set_mouse_y_sensitivity( float s ) { mouse_y_sensitivity_ = s; }

	void load_config( Config& );

	void update();
	void update_null();
	void update_common();

	void arrow_push( Button button )
	{
		arrow_stack_.push_back( button );
	}

	void arrow_pop( Button button )
	{
		ButtonStack::iterator i = std::find( arrow_stack_.begin(), arrow_stack_.end(), button );
		arrow_stack_.erase( i );
	}

	/**
	 * ボタンが押されていない状態に更新する
	 *
	 * @param button 押されていない状態にするボタン
	 */
	void clear( Button button )
	{
		state_[ static_cast< int >( button ) ] = 0;
	}

	/**
	 * 現在ボタンが押されているかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return 現在ボタンが押されている場合は true を、そうでない場合は false を返す
	 */
	bool press( Button button ) const
	{
		return ( state_[ static_cast< int >( button ) ] & 1 ) > 0;
	}

	/**
	 * たった今ボタンが押されたかどうかを取得する
	 *
	 * @param button 調べるボタン
	 * @return たった今ボタンが押された場合は true を、そうでない場合は false を返す
	 */
	bool push( Button button ) const
	{
		return ( state_[ static_cast< int >( button ) ] & 1 ) > 0 && ( state_[ static_cast< int >( button ) ] & 2 ) == 0;
	}

	/**
	 * たった今ボタンが離されたかどうかを取得する
	 *
	 * button 調べるボタン
	 * @return たった今ボタンが離された場合は true を、そうでない場合は false を返す
	 */
	bool release( Button button ) const
	{
		return ( state_[ static_cast< int >( button ) ] & 1 ) == 0 && ( state_[ static_cast< int >( button ) ] & 2 ) > 0;
	}

	/**
	 * 現在最優先で押されている方向ボタンを取得する
	 *
	 * @return 現在最優先で押されている方向ボタン ( 方向ボタンが押されていない場合は NONE を返す )
	 */
	Button get_primary_arrow_button() const
	{
		if ( arrow_stack_.empty() ) return Button::NONE;

		return arrow_stack_.back();
	}

	const ButtonStack& get_arrow_button_stack() const
	{
		return arrow_stack_;
	}

	static const Input& get_null_input()
	{
		static Input null_input;

		return null_input;
	}

	int get_mouse_x() const { return mouse_point_.x; }
	int get_mouse_y() const { return mouse_point_.y; }

	float get_mouse_x_rate() const { return mouse_x_rate_; }
	float get_mouse_y_rate() const { return mouse_y_rate_; }

	float get_mouse_dx() const { return mouse_dx_; }
	float get_mouse_dy() const { return mouse_dy_; }

//	void set_mouse_x_rate( float );
//	void set_mouse_y_rate( float );

	void push_mouse_wheel_queue( int wheel ) { mouse_wheel_ += wheel; }
	int pop_mouse_wheel_queue() { int w = mouse_wheel_; mouse_wheel_ = 0; return w; }

}; // class Input

} // namespace blue_sky

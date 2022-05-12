//**********************************************************
/**
 * Input の実装
 *
 * copyright (c) je-pu-pu all rights reserved
 *
 * @file		Input.cpp
 * @date		2006/09/12
 * @author		JE
 */
//**********************************************************

#include "Input.h"
#include "App.h"
#include <core/input/DirectInput/DirectInput.h>

#include <game/Config.h>

#include <common/math.h>

#include <windows.h>

namespace blue_sky
{

Input::Input()
	: direct_input_( nullptr )
	, state_()
	, joystick_enabled_( false )
	, joystick_axis_threshold_( 0.f )
	, joystick_x_axis_pos_( nullptr )
	, joystick_y_axis_pos_( nullptr )
	, joystick_x_sensitivity_( 1.f )
	, joystick_y_sensitivity_( 1.f )
	, mouse_x_sensitivity_( 1.f )
	, mouse_y_sensitivity_( 1.f )
	, mouse_x_rate_( 0.f )
	, mouse_y_rate_( 0.f )
	, mouse_dx_( 0.f )
	, mouse_dy_( 0.f )
	, mouse_wheel_( 0 )
	, mouse_point_()
	, joystick_code_()
{
	joy_info_.dwSize = sizeof( JOYINFOEX );
	joy_info_.dwFlags = JOY_RETURNALL;

	joystick_enabled_ = ( joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR );

	if ( joystick_enabled_ )
	{
		JOYCAPS jc;
		joyGetDevCaps( JOYSTICKID1, & jc, sizeof( JOYCAPS ) );
		joystick_enabled_ &= jc.wNumAxes >= 4;
	}

	config_key_code_map_.insert( std::make_pair( "MOUSE_L", VK_LBUTTON  ) );
	config_key_code_map_.insert( std::make_pair( "MOUSE_R", VK_RBUTTON ) );
	config_key_code_map_.insert( std::make_pair( "SPACE", VK_SPACE ) );
}

Input::~Input()
{
	
}

void Input::load_config( Config& config )
{
	key_code_[ static_cast< int >( Button::ESCAPE ) ].push_back( VK_ESCAPE );

	load_key_code_config( config, Button::LEFT,  "input.key.left",  "A" );
	load_key_code_config( config, Button::RIGHT, "input.key.right", "D" );
	load_key_code_config( config, Button::UP,    "input.key.up",    "W" );
	load_key_code_config( config, Button::DOWN,  "input.key.down",  "S" );
	
	load_key_code_config( config, Button::A,     "input.key.a",     "MOUSE_L" );
	load_key_code_config( config, Button::B,     "input.key.b",     "MOUSE_R" );
	load_key_code_config( config, Button::JUMP,   "input.key.jump", "SPACE"   );

	load_key_code_config( config, Button::L,     "input.key.l",     "Q" );
	load_key_code_config( config, Button::R,     "input.key.r",     "E" );
	load_key_code_config( config, Button::L2,    "input.key.l2",    "Z" );
	load_key_code_config( config, Button::R2,    "input.key.r2",    "C" );

	joystick_code_[ static_cast< int >( Button::A ) ] = 1 << ( config.get( "input.joystick.a", 1 ) - 1 );
	joystick_code_[ static_cast< int >( Button::B ) ] = 1 << ( config.get( "input.joystick.b", 3 ) - 1 );
	
	joystick_code_[ static_cast< int >( Button::L ) ] = 1 << ( config.get( "input.joystick.l", 5 ) - 1 );
	joystick_code_[ static_cast< int >( Button::R ) ] = 1 << ( config.get( "input.joystick.r", 6 ) - 1 );

	joystick_axis_threshold_ = config.get( "input.joystick.axis.threshold", 0.25f );

	joystick_x_axis_pos_ = get_joystick_axis_pos_pointer_by_index( config.get( "input.joystick.axis.x", 3 ) );
	joystick_y_axis_pos_ = get_joystick_axis_pos_pointer_by_index( config.get( "input.joystick.axis.y", 2 ) );

	mouse_x_sensitivity_ = config.get( "input.mouse.x_sensitivity", 1.f );
	mouse_y_sensitivity_ = config.get( "input.mouse.y_sensitivity", 1.f );

	joystick_x_sensitivity_ = config.get( "input.joystick.axis.x_sensitivity", 1.f );
	joystick_y_sensitivity_ = config.get( "input.joystick.axis.y_sensitivity", 1.f );
}

void Input::load_key_code_config( Config& config, Button button, const char_t* name, const char_t* default_value )
{
	std::string values = config.get( name, std::string( default_value ) );
	std::stringstream ss;

	ss << values;

	while ( ss.good() )
	{
		std::string key_code;

		ss >> key_code;

		auto i = config_key_code_map_.find( key_code );

		if ( i != config_key_code_map_.end() )
		{
			key_code_[ static_cast< int >( button ) ].push_back( i->second );
		}
		else if ( key_code.size() == 1 )
		{
			key_code_[ static_cast< int >( button ) ].push_back( key_code.at( 0 ) );
		}
		else
		{

		}
	}
}

/**
 * キーボードからの入力による状態の更新を行う
 */
void Input::update()
{
	for ( int n = 0; n < static_cast< int >( Button::MAX ); n++ )
	{
		state_[ n ] <<= 1;

		update_state_by_key_for( n );
	}

	POINT point;
	GetCursorPos( & point );
	ScreenToClient( App::get_instance()->GetWindowHandle(), & point );

	mouse_x_rate_ = math::clamp( point.x / static_cast< float >( App::get_instance()->get_width() - 1 ) * 2.f - 1.f, -1.f, +1.f ) * mouse_x_sensitivity_;
	mouse_y_rate_ = math::clamp( point.y / static_cast< float >( App::get_instance()->get_height() - 1 ) * 2.f - 1.f, -1.f, +1.f ) * mouse_y_sensitivity_;

	mouse_dx_ = static_cast< float >( direct_input_->get_mouse_state().lX ) / static_cast< float >( App::get_instance()->get_width() - 1 ) * 2.f * mouse_x_sensitivity_;
	mouse_dy_ = static_cast< float >( direct_input_->get_mouse_state().lY ) / static_cast< float >( App::get_instance()->get_height() - 1 ) * 2.f * mouse_y_sensitivity_;

	mouse_point_ = point;

	if ( joystick_enabled_ && joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR )
	{
		if ( joy_info_.dwXpos <  0xFFFF / 4 * 1 ) state_[ static_cast< int >( Button::LEFT  ) ] |= 1;
		if ( joy_info_.dwXpos >= 0xFFFF / 4 * 3 ) state_[ static_cast< int >( Button::RIGHT ) ] |= 1;
		if ( joy_info_.dwYpos <  0xFFFF / 4 * 1 ) state_[ static_cast< int >( Button::UP    ) ] |= 1;
		if ( joy_info_.dwYpos >= 0xFFFF / 4 * 3 ) state_[ static_cast< int >( Button::DOWN  ) ] |= 1;
	
		if ( joy_info_.dwButtons & joystick_code_[ static_cast< int >( Button::A ) ] ) state_[ static_cast< int >( Button::A ) ] |= 1;
		if ( joy_info_.dwButtons & joystick_code_[ static_cast< int >( Button::B ) ] ) state_[ static_cast< int >( Button::B ) ] |= 1;
	
		if ( joy_info_.dwButtons & joystick_code_[ static_cast< int >( Button::L ) ] ) state_[ static_cast< int >( Button::L ) ] |= 1;
		if ( joy_info_.dwButtons & joystick_code_[ static_cast< int >( Button::R ) ] ) state_[ static_cast< int >( Button::R ) ] |= 1;

		mouse_dx_ += get_rate_by_joystick_axis_pos( * joystick_x_axis_pos_ ) * 0.01f * joystick_x_sensitivity_;
		mouse_dy_ += get_rate_by_joystick_axis_pos( * joystick_y_axis_pos_ ) * 0.01f * joystick_y_sensitivity_;
	}

	update_common();
}

void Input::update_null()
{
	for ( int n = 0; n < static_cast< int >( Button::MAX ); n++ )
	{
		state_[ n ] <<= 1;
	}
}

/**
 * キーボード・ジョイスティック共通の更新処理を行う
 */
void Input::update_common()
{
	if ( release( Button::DOWN  ) ) arrow_pop( Button::DOWN  );
	if ( release( Button::UP    ) ) arrow_pop( Button::UP    );
	if ( release( Button::RIGHT ) ) arrow_pop( Button::RIGHT );
	if ( release( Button::LEFT  ) ) arrow_pop( Button::LEFT  );

	if ( push( Button::DOWN  ) ) arrow_push( Button::DOWN  );
	if ( push( Button::UP    ) ) arrow_push( Button::UP    );
	if ( push( Button::RIGHT ) ) arrow_push( Button::RIGHT );
	if ( push( Button::LEFT  ) ) arrow_push( Button::LEFT  );
}

/**
 * キー入力をチェックし指定したボタンの状態を更新する
 *
 * @param button ボタン
 */
void Input::update_state_by_key_for( uint_t button )
{
	for ( auto i = key_code_[ button ].begin(); i != key_code_[ button ].end(); ++i )
	{
		if ( GetAsyncKeyState( *i ) & 0x8000 )
		{
			state_[ button ] |= 1;
		}
	}
}

DWORD* Input::get_joystick_axis_pos_pointer_by_index( int index )
{
	std::unordered_map< int, DWORD* > axis_map;

	axis_map[ 0 ] = & joy_info_.dwXpos;
	axis_map[ 1 ] = & joy_info_.dwYpos;
	axis_map[ 2 ] = & joy_info_.dwZpos;
	axis_map[ 3 ] = & joy_info_.dwRpos;
	axis_map[ 4 ] = & joy_info_.dwUpos;
	axis_map[ 5 ] = & joy_info_.dwVpos;

	return axis_map.at( index );
}

float Input::get_rate_by_joystick_axis_pos( DWORD pos )
{
	float rate = ( pos / static_cast< float >( 0xFFFF ) * 2.f ) - 1.f;

	return math::chase( rate, 0.f, joystick_axis_threshold_ ) / ( 1.f - joystick_axis_threshold_ );
}

#if 0

void Input::set_mouse_x_rate( float x )
{
	mouse_x_rate_ = x;
	mouse_dx_ = 0.f;

	POINT point;

	point.x = static_cast< int >( ( mouse_x_rate_ + 1.f ) * 0.5f * static_cast< float >( App::GetInstance()->get_width() - 1 ) );
	point.y = static_cast< int >( ( mouse_y_rate_ + 1.f ) * 0.5f * static_cast< float >( App::GetInstance()->get_height() - 1 ) );

	mouse_point_.x = point.x;

	ClientToScreen( App::GetInstance()->GetWindowHandle(), & point );
	SetCursorPos( point.x, point.y );

	POINT new_point;
	GetCursorPos( & new_point );
}

void Input::set_mouse_y_rate( float y )
{
	return;

	mouse_y_rate_ = y;
	mouse_dy_ = 0.f;

	POINT point;

	point.x = static_cast< int >( ( mouse_x_rate_ + 1.f ) * 0.5f * static_cast< float >( App::GetInstance()->get_width() - 1 ) );
	point.y = static_cast< int >( ( mouse_x_rate_ + 1.f ) * 0.5f * static_cast< float >( App::GetInstance()->get_height() - 1 ) );

	mouse_point_.y = point.y;

	ClientToScreen( App::GetInstance()->GetWindowHandle(), & point );
	SetCursorPos( point.x, point.y );
}

#endif // 0

} // namespace blue_sky
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
#include "KeyNames.h"
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
	, key_now_()
	, key_prev_()
	, was_foreground_( false )
	, pad_buttons_now_( 0 )
	, pad_buttons_prev_( 0 )
	, pad_dir_now_( 0 )
	, pad_dir_prev_( 0 )
	, is_blocked_( false )
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
}

Input::~Input()
{
	
}

const char_t* Input::get_key_config_name( Button button )
{
	switch ( button )
	{
		case Button::LEFT:	return "input.key.left";
		case Button::RIGHT:	return "input.key.right";
		case Button::UP:	return "input.key.up";
		case Button::DOWN:	return "input.key.down";
		case Button::A:		return "input.key.a";
		case Button::B:		return "input.key.b";
		case Button::JUMP:	return "input.key.jump";
		case Button::L:		return "input.key.l";
		case Button::R:		return "input.key.r";
		case Button::L2:	return "input.key.l2";
		case Button::R2:	return "input.key.r2";
		default:			return nullptr;
	}
}

void Input::load_config( Config& config )
{
	// 何度呼ばれても割り当てが重複しないよう、毎回作り直す
	for ( auto& codes : key_code_ ) codes.clear();
	for ( auto& codes : fixed_key_code_ ) codes.clear();

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

	// 固定キー : ESC と、メニュー操作用の矢印キー・Enter（ユーザー割り当てを変えても常に有効）
	fixed_key_code_[ static_cast< int >( Button::ESCAPE ) ].push_back( VK_ESCAPE );
	fixed_key_code_[ static_cast< int >( Button::UP ) ].push_back( VK_UP );
	fixed_key_code_[ static_cast< int >( Button::DOWN ) ].push_back( VK_DOWN );
	fixed_key_code_[ static_cast< int >( Button::LEFT ) ].push_back( VK_LEFT );
	fixed_key_code_[ static_cast< int >( Button::RIGHT ) ].push_back( VK_RIGHT );
	fixed_key_code_[ static_cast< int >( Button::A ) ].push_back( VK_RETURN );

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
	auto codes = parse_key_bindings_text( config.get( name, std::string( default_value ) ) );

	// 1 つも解釈できなければ既定値を使う（設定ファイルが壊れていても操作不能にしない）
	if ( codes.empty() )
	{
		codes = parse_key_bindings_text( default_value );
	}

	key_code_[ static_cast< int >( button ) ] = codes;
}

std::vector< int > Input::parse_key_bindings_text( const string_t& text )
{
	std::vector< int > codes;
	std::stringstream ss( text );
	std::string name;

	while ( ss >> name )
	{
		const int code = key_names::code_from_name( name );

		if ( code != 0 && std::find( codes.begin(), codes.end(), code ) == codes.end() )
		{
			codes.push_back( code );
		}
	}

	return codes;
}

string_t Input::get_key_bindings_text( Button button ) const
{
	string_t text;

	for ( int code : key_code_[ static_cast< int >( button ) ] )
	{
		const string_t name = key_names::name_from_code( code );

		if ( name.empty() )
		{
			continue;
		}

		if ( ! text.empty() )
		{
			text += ' ';
		}

		text += name;
	}

	return text;
}

int Input::get_pushed_bindable_key() const
{
	for ( int code : key_names::get_bindable_codes() )
	{
		if ( is_key_pushed( code ) )
		{
			return code;
		}
	}

	return 0;
}

int Input::pad_dir_bit( Button dir )
{
	switch ( dir )
	{
		case Button::LEFT: return 1;
		case Button::RIGHT: return 2;
		case Button::UP: return 4;
		case Button::DOWN: return 8;
		default: return 0;
	}
}

/**
 * 生のキー状態（全仮想キー）を更新する
 */
void Input::update_raw_keys()
{
	const bool foreground = App::get_instance() && GetForegroundWindow() == App::get_instance()->GetWindowHandle();

	key_prev_ = key_now_;

	for ( int vk = 1; vk < 256; vk++ )
	{
		key_now_[ vk ] = foreground && ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;
	}

	// 再アクティブになったフレーム（Alt+Tab で戻った直後など）は、押しっぱなしのキーを「押した瞬間」にしない
	if ( foreground && ! was_foreground_ )
	{
		key_prev_ = key_now_;
	}

	was_foreground_ = foreground;

	pad_buttons_prev_ = pad_buttons_now_;
	pad_dir_prev_ = pad_dir_now_;
	pad_buttons_now_ = 0;
	pad_dir_now_ = 0;
}

/**
 * キーボードからの入力による状態の更新を行う
 */
void Input::update()
{
	update_raw_keys();

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

		// 生の状態（UI 用）
		if ( was_foreground_ )
		{
			pad_buttons_now_ = joy_info_.dwButtons;

			if ( joy_info_.dwXpos <  0xFFFF / 4 * 1 ) pad_dir_now_ |= pad_dir_bit( Button::LEFT );
			if ( joy_info_.dwXpos >= 0xFFFF / 4 * 3 ) pad_dir_now_ |= pad_dir_bit( Button::RIGHT );
			if ( joy_info_.dwYpos <  0xFFFF / 4 * 1 ) pad_dir_now_ |= pad_dir_bit( Button::UP );
			if ( joy_info_.dwYpos >= 0xFFFF / 4 * 3 ) pad_dir_now_ |= pad_dir_bit( Button::DOWN );
		}

		mouse_dx_ += get_rate_by_joystick_axis_pos( * joystick_x_axis_pos_ ) * 0.01f * joystick_x_sensitivity_;
		mouse_dy_ += get_rate_by_joystick_axis_pos( * joystick_y_axis_pos_ ) * 0.01f * joystick_y_sensitivity_;
	}

	update_common();
}

void Input::update_null()
{
	// 生の状態も「何も押されていない」に進める
	key_prev_ = key_now_;
	key_now_.fill( false );
	was_foreground_ = false;
	pad_buttons_prev_ = pad_buttons_now_;
	pad_dir_prev_ = pad_dir_now_;
	pad_buttons_now_ = 0;
	pad_dir_now_ = 0;

	for ( int n = 0; n < static_cast< int >( Button::MAX ); n++ )
	{
		state_[ n ] <<= 1;
	}
}

/**
 * キーボード・ジョイスティック共通の更新処理を行う
 *
 * is_blocked_ に関わらず arrow_stack_ を正しく更新するため、
 * push/release の代わりに raw state を直接チェックする。
 */
void Input::update_common()
{
	auto raw_push = [this]( Button b ) -> bool {
		auto n = static_cast< int >( b );
		return ( state_[ n ] & 1 ) > 0 && ( state_[ n ] & 2 ) == 0;
	};

	auto raw_release = [this]( Button b ) -> bool {
		auto n = static_cast< int >( b );
		return ( state_[ n ] & 1 ) == 0 && ( state_[ n ] & 2 ) > 0;
	};

	if ( raw_release( Button::DOWN  ) ) arrow_pop( Button::DOWN  );
	if ( raw_release( Button::UP    ) ) arrow_pop( Button::UP    );
	if ( raw_release( Button::RIGHT ) ) arrow_pop( Button::RIGHT );
	if ( raw_release( Button::LEFT  ) ) arrow_pop( Button::LEFT  );

	if ( raw_push( Button::DOWN  ) ) arrow_push( Button::DOWN  );
	if ( raw_push( Button::UP    ) ) arrow_push( Button::UP    );
	if ( raw_push( Button::RIGHT ) ) arrow_push( Button::RIGHT );
	if ( raw_push( Button::LEFT  ) ) arrow_push( Button::LEFT  );
}

/**
 * キー入力をチェックし指定したボタンの状態を更新する
 *
 * @param button ボタン
 */
void Input::update_state_by_key_for( uint_t button )
{
	for ( int code : key_code_[ button ] )
	{
		if ( GetAsyncKeyState( code ) & 0x8000 )
		{
			state_[ button ] |= 1;
		}
	}

	for ( int code : fixed_key_code_[ button ] )
	{
		if ( GetAsyncKeyState( code ) & 0x8000 )
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

float Input::get_rate_by_joystick_axis_pos( DWORD pos ) const
{
	float rate = ( pos / static_cast< float >( 0xFFFF ) * 2.f ) - 1.f;

	return math::chase( rate, 0.f, joystick_axis_threshold_ ) / ( 1.f - joystick_axis_threshold_ );
}


} // namespace blue_sky
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
#include "KeyLatch.h"
#include "App.h"
#include "KeyNames.h"
#include <core/input/DirectInput/DirectInput.h>

#include <game/Config.h>

#include <common/math.h>

#include <cmath>

#include <windows.h>

namespace blue_sky
{

Input::Input( const App* app )
	: app_( app )
	, direct_input_( nullptr )
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

	detect_joystick();
}

void Input::detect_joystick()
{
	joystick_enabled_ = ( joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR );

	if ( joystick_enabled_ )
	{
		JOYCAPS jc;
		joyGetDevCaps( JOYSTICKID1, & jc, sizeof( JOYCAPS ) );
		joystick_enabled_ &= jc.wNumAxes >= 4;
	}
}

void Input::push_char_byte( char_t byte )
{
	const unsigned char b = static_cast< unsigned char >( byte );

	// ウィンドウは ANSI 版なので、日本語は CP932 の 2 バイトに分かれて届く。先頭バイトなら次を待つ
	if ( pending_lead_byte_ == 0 && IsDBCSLeadByte( b ) )
	{
		pending_lead_byte_ = b;
		return;
	}

	char bytes[ 2 ];
	int length = 0;

	if ( pending_lead_byte_ != 0 )
	{
		bytes[ length++ ] = static_cast< char >( pending_lead_byte_ );
		pending_lead_byte_ = 0;
	}

	bytes[ length++ ] = static_cast< char >( b );

	wchar_t w[ 2 ] = { };
	const int n = MultiByteToWideChar( CP_ACP, 0, bytes, length, w, 2 );

	// 溜めすぎない（UI が読まない間に打たれた文字は古いものから捨てる）
	constexpr size_t MAX_PENDING_TEXT = 64;

	for ( int i = 0; i < n; i++ )
	{
		if ( pending_text_.size() >= MAX_PENDING_TEXT )
		{
			pending_text_.erase( pending_text_.begin() );
		}

		pending_text_.push_back( static_cast< char32_t >( w[ i ] ) );
	}
}

std::u32string Input::take_text()
{
	std::u32string text;
	text.swap( pending_text_ );
	return text;
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

/**
 * 開発用・機器の設定（blue-sky.config）を読む
 *
 * キー割り当てとマウス感度はユーザーの設定なので、ここでは読まない（SettingStore が反映する）。
 * 以前はここでも既定値付きで読んでいて、既定値と読み方が 2 か所にあった。
 */
void Input::load_config( const Config& config )
{
	// 何度呼ばれても割り当てが重複しないよう、毎回作り直す
	for ( auto& codes : fixed_key_code_ ) codes.clear();

	// 固定キー : ESC と、メニュー操作用の矢印キー・Enter（ユーザー割り当てを変えても常に有効）
	fixed_key_code_[ static_cast< int >( Button::ESCAPE ) ].push_back( VK_ESCAPE );
	fixed_key_code_[ static_cast< int >( Button::UP ) ].push_back( VK_UP );
	fixed_key_code_[ static_cast< int >( Button::DOWN ) ].push_back( VK_DOWN );
	fixed_key_code_[ static_cast< int >( Button::LEFT ) ].push_back( VK_LEFT );
	fixed_key_code_[ static_cast< int >( Button::RIGHT ) ].push_back( VK_RIGHT );
	fixed_key_code_[ static_cast< int >( Button::A ) ].push_back( VK_RETURN );

	// 設定ファイルの値は手で編集されることがあるので、範囲外なら既定値を使う
	// （以前は範囲外のボタン番号でシフトが未定義動作に、範囲外の軸番号で例外になり、起動できなかった）
	const auto button_mask = [ &config ] ( const char_t* key, int def )
	{
		int n = config.get( key, def );

		if ( n < 1 || n > 32 )
		{
			n = def;
		}

		return 1u << ( n - 1 );
	};

	const auto axis_index = [ &config ] ( const char_t* key, int def )
	{
		const int n = config.get( key, def );
		return ( n >= 0 && n <= 5 ) ? n : def;
	};

	joystick_code_[ static_cast< int >( Button::A ) ] = button_mask( "input.joystick.a", 1 );
	joystick_code_[ static_cast< int >( Button::B ) ] = button_mask( "input.joystick.b", 3 );

	joystick_code_[ static_cast< int >( Button::L ) ] = button_mask( "input.joystick.l", 5 );
	joystick_code_[ static_cast< int >( Button::R ) ] = button_mask( "input.joystick.r", 6 );

	// しきい値は ( 1 - しきい値 ) で割るので 1 未満に収める
	joystick_axis_threshold_ = std::clamp( config.get( "input.joystick.axis.threshold", 0.25f ), 0.f, 0.95f );

	joystick_x_axis_pos_ = get_joystick_axis_pos_pointer_by_index( axis_index( "input.joystick.axis.x", 3 ) );
	joystick_y_axis_pos_ = get_joystick_axis_pos_pointer_by_index( axis_index( "input.joystick.axis.y", 2 ) );


	joystick_x_sensitivity_ = config.get( "input.joystick.axis.x_sensitivity", 1.f );
	joystick_y_sensitivity_ = config.get( "input.joystick.axis.y_sensitivity", 1.f );
}

std::vector< int > Input::parse_key_bindings_text( const string_t& text, const std::function< void( const std::string& ) >& log )
{
	std::vector< int > codes;
	std::stringstream ss( text );
	std::string name;

	while ( ss >> name )
	{
		const int code = key_names::code_from_name( name );

		if ( code == 0 )
		{
			if ( log ) log( "[Input] キー割り当ての \"" + text + "\" にある \"" + name + "\" はキーの名前として解釈できないので飛ばします" );
			continue;
		}

		if ( ! key_names::is_bindable( code ) )
		{
			if ( log ) log( "[Input] キー割り当ての \"" + text + "\" にある \"" + name + "\" は割り当てられないキー（固定キーなど）なので飛ばします" );
			continue;
		}

		if ( std::find( codes.begin(), codes.end(), code ) == codes.end() )
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
		const bool pushed = key_names::is_pad_code( code ) ? is_pad_button_pushed( key_names::pad_button_index( code ) ) : is_key_pushed( code );

		if ( pushed )
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
	const bool foreground = app_ && GetForegroundWindow() == app_->GetWindowHandle();

	key_prev_ = key_now_;

	for ( int vk = 1; vk < 256; vk++ )
	{
		key_now_[ vk ] = foreground && ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;
	}

	// 1 フレーム内に押して離した短いタップを取りこぼさない（前面でないときの押下は捨てる）
	if ( foreground )
	{
		apply_latched_presses( key_now_, key_prev_, key_latched_ );
	}
	else
	{
		key_latched_.fill( false );
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

	// パッドは起動時の 1 回だけでなく、つながっていなければ約 2 秒ごとに探し直す（途中で挿しても使えるように）
	if ( ! joystick_enabled_ && ++joystick_detect_wait_ >= 120 )
	{
		joystick_detect_wait_ = 0;
		detect_joystick();
	}

	// パッドを先に読む（パッドのボタンの割り当てを、同じフレームのボタンの状態に入れるため）
	const bool joy_ok = joystick_enabled_ && joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR;
	stick_x_ = 0.f;
	stick_y_ = 0.f;

	if ( joy_ok && was_foreground_ )
	{
		pad_buttons_now_ = joy_info_.dwButtons;

		if ( joy_info_.dwXpos <  0xFFFF / 4 * 1 ) pad_dir_now_ |= pad_dir_bit( Button::LEFT );
		if ( joy_info_.dwXpos >= 0xFFFF / 4 * 3 ) pad_dir_now_ |= pad_dir_bit( Button::RIGHT );
		if ( joy_info_.dwYpos <  0xFFFF / 4 * 1 ) pad_dir_now_ |= pad_dir_bit( Button::UP );
		if ( joy_info_.dwYpos >= 0xFFFF / 4 * 3 ) pad_dir_now_ |= pad_dir_bit( Button::DOWN );

		// スティックの傾き（-1 〜 +1。中央付近は 0）
		const auto axis = [] ( DWORD pos )
		{
			const float_t v = static_cast< float_t >( pos ) / 32767.5f - 1.f;
			constexpr float_t DEAD_ZONE = 0.2f;
			return std::abs( v ) < DEAD_ZONE ? 0.f : ( v - std::copysign( DEAD_ZONE, v ) ) / ( 1.f - DEAD_ZONE );
		};

		stick_x_ = axis( joy_info_.dwXpos );
		stick_y_ = -axis( joy_info_.dwYpos );		// 上を正にする
	}

	for ( int n = 0; n < static_cast< int >( Button::MAX ); n++ )
	{
		state_[ n ] <<= 1;

		update_state_by_key_for( n );
	}

	POINT point;
	GetCursorPos( & point );
	ScreenToClient( app_->GetWindowHandle(), & point );
	point = app_->client_to_render( point );	// フルスクリーン（ボーダーレス）ではウィンドウが描画より大きい

	mouse_x_rate_ = math::clamp( point.x / static_cast< float >( app_->get_width() - 1 ) * 2.f - 1.f, -1.f, +1.f ) * mouse_x_sensitivity_;
	mouse_y_rate_ = math::clamp( point.y / static_cast< float >( app_->get_height() - 1 ) * 2.f - 1.f, -1.f, +1.f ) * mouse_y_sensitivity_;

	// マウスの移動量 : Raw Input が使えるなら、フレームの間に受け取った相対の移動量の合計を使う（DirectInput のマウスは古い API）
	const bool raw = app_->is_raw_input_enabled();
	const float_t move_x = raw ? static_cast< float_t >( raw_mouse_dx_ ) : static_cast< float_t >( direct_input_->get_mouse_state().lX );
	const float_t move_y = raw ? static_cast< float_t >( raw_mouse_dy_ ) : static_cast< float_t >( direct_input_->get_mouse_state().lY );
	raw_mouse_dx_ = 0;
	raw_mouse_dy_ = 0;

	mouse_dx_ = move_x / static_cast< float >( app_->get_width() - 1 ) * 2.f * mouse_x_sensitivity_;
	mouse_dy_ = move_y / static_cast< float >( app_->get_height() - 1 ) * 2.f * mouse_y_sensitivity_;

	mouse_point_ = point;

	if ( joy_ok )
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
 * キー入力をチェックし指定したボタンの状態を更新する
 *
 * @param button ボタン
 */
void Input::update_state_by_key_for( uint_t button )
{
	// 生のキー状態（update_raw_keys で読んだもの。短いタップの取りこぼし対策を含む）から作る
	// （以前は GetAsyncKeyState をもう一度読んでいて、ゲームの操作だけ短いタップを取りこぼしていた）
	for ( int code : key_code_[ button ] )
	{
		// パッドのボタンも割り当てられる（以前はキーボードのキーだけだった）
		const bool down = key_names::is_pad_code( code ) ? is_pad_button_down( key_names::pad_button_index( code ) ) : is_key_down( code );

		if ( down )
		{
			state_[ button ] |= 1;
		}
	}

	for ( int code : fixed_key_code_[ button ] )
	{
		if ( is_key_down( code ) )
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
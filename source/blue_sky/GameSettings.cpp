#include "GameSettings.h"
#include "GameMain.h"
#include "Input.h"

#include <core/setting/SettingStore.h>
#include <core/sound/SoundManager.h>

#include <sstream>

namespace blue_sky::game_settings
{

using core::setting::ApplyMode;
using core::setting::SettingStore;
using Button = core::input::Button;

const std::vector< string_t >& get_display_keys()
{
	static const std::vector< string_t > keys = { SCREEN_WIDTH, SCREEN_HEIGHT, FULL_SCREEN };
	return keys;
}

const std::vector< BindableButton >& get_bindable_buttons()
{
	static const std::vector< BindableButton > buttons =
	{
		{ Button::UP,		"options.key.up" },
		{ Button::DOWN,		"options.key.down" },
		{ Button::LEFT,		"options.key.left" },
		{ Button::RIGHT,	"options.key.right" },
		{ Button::JUMP,		"options.key.jump" },
		{ Button::A,		"options.key.a" },
		{ Button::B,		"options.key.b" },
		{ Button::L,		"options.key.l" },
		{ Button::R,		"options.key.r" },
		{ Button::L2,		"options.key.l2" },
		{ Button::R2,		"options.key.r2" },
	};

	return buttons;
}

string_t get_primary_key( const string_t& bindings )
{
	std::istringstream ss( bindings );
	string_t first;
	ss >> first;
	return first;
}

string_t replace_primary_key( const string_t&, const string_t& key_name )
{
	return key_name;
}

namespace
{

/// 既定のキー割り当て（Input::load_config の既定値と同じ）
const char_t* default_key( Button b )
{
	switch ( b )
	{
		case Button::LEFT:	return "A";
		case Button::RIGHT:	return "D";
		case Button::UP:	return "W";
		case Button::DOWN:	return "S";
		case Button::A:		return "MOUSE_L";
		case Button::B:		return "MOUSE_R";
		case Button::JUMP:	return "SPACE";
		case Button::L:		return "Q";
		case Button::R:		return "E";
		case Button::L2:	return "Z";
		case Button::R2:	return "C";
		default:			return "";
	}
}

/**
 * キー割り当ての重複を入れ替えるフック
 *
 * 変更したボタンの主キーを、他のボタンが主キーとして使っていたら、そのボタンに変更前の主キーを渡す。
 */
void swap_duplicate_key( SettingStore& store, const string_t& key, const SettingStore::Value& old_value )
{
	const string_t new_primary = get_primary_key( store.get< string_t >( key ) );
	const auto* old_text = std::get_if< string_t >( & old_value );
	const string_t old_primary = old_text ? get_primary_key( *old_text ) : string_t();

	if ( new_primary.empty() )
	{
		return;
	}

	for ( const auto& b : get_bindable_buttons() )
	{
		const string_t other = Input::get_key_config_name( b.button );

		if ( other == key )
		{
			continue;
		}

		if ( get_primary_key( store.get< string_t >( other ) ) == new_primary )
		{
			store.set( other, replace_primary_key( store.get< string_t >( other ), old_primary ) );
		}
	}
}

} // namespace

void register_all( SettingStore& store, GameMain& game_main )
{
	GameMain* gm = & game_main;

	// サウンド
	store.add_float( AUDIO_VOLUME, 1.f, 0.f, 1.f, 0.05f, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_sound_manager()->set_volume( std::get< float_t >( v ) );
	} );

	store.add_bool( AUDIO_MUTE, false, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_sound_manager()->set_mute( std::get< bool >( v ) );
	} );

	// 操作
	store.add_float( MOUSE_X_SENSITIVITY, 1.f, 0.1f, 3.f, 0.1f, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_input()->set_mouse_x_sensitivity( std::get< float_t >( v ) );
	} );

	store.add_float( MOUSE_Y_SENSITIVITY, 1.f, 0.1f, 3.f, 0.1f, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_input()->set_mouse_y_sensitivity( std::get< float_t >( v ) );
	} );

	for ( const auto& b : get_bindable_buttons() )
	{
		const Button button = b.button;
		const string_t key = Input::get_key_config_name( button );

		store.add_string( key, default_key( button ), ApplyMode::IMMEDIATE, [gm, button] ( const SettingStore::Value& v )
		{
			auto codes = Input::parse_key_bindings_text( std::get< string_t >( v ) );

			if ( ! codes.empty() )
			{
				gm->get_input()->set_key_bindings( button, codes );
			}
		} );

		store.set_on_set( key, & swap_duplicate_key );
	}

	// カメラ（反映はシーン側が change_count を見て行う）
	store.add_float( CAMERA_FOV, 90.f, 50.f, 120.f, 1.f, ApplyMode::IMMEDIATE );

	// グラフィックス
	store.add_bool( DISPLAY_FPS, false, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->set_display_fps( std::get< bool >( v ) );
	} );

	store.add_int( SCREEN_WIDTH, 1920, 640, 7680, ApplyMode::DEFERRED );
	store.add_int( SCREEN_HEIGHT, 1080, 480, 4320, ApplyMode::DEFERRED );
	store.add_bool( FULL_SCREEN, false, ApplyMode::DEFERRED );

	// 表示系はまとめて 1 回、フレームの最後に反映する
	store.add_apply_group( get_display_keys(), [gm] { gm->request_display_change(); } );
}

} // namespace blue_sky::game_settings

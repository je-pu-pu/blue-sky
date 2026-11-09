#include "GameSettings.h"
#include "GameMain.h"
#include "Input.h"
#include "KeyNames.h"

#include <core/setting/SettingStore.h>
#include <core/sound/SoundManager.h>

#include <game/Config.h>

#include <filesystem>

#include <windows.h>
#include <shlobj.h>

#pragma comment( lib, "shell32.lib" )
#include <sstream>

#include <algorithm>

namespace blue_sky::game_settings
{

using core::setting::ApplyMode;
using core::setting::SettingStore;
using Button = core::input::Button;

const std::vector< string_t >& get_display_keys()
{
	static const std::vector< string_t > keys = { SCREEN_WIDTH.str(), SCREEN_HEIGHT.str(), FULL_SCREEN.str() };
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

/// 既定のキー割り当て（キー割り当ての既定値の唯一の定義。Input はこの値をストア経由で受け取る）
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

/// キー割り当ての文字列の主キーのコード（解釈できなければ 0）
int primary_code( const string_t& bindings )
{
	return key_names::code_from_name( get_primary_key( bindings ) );
}

/**
 * キー割り当ての重複を入れ替えるフック
 *
 * 変更したボタンの主キーを、他のボタンが主キーとして使っていたら、そのボタンに変更前の主キーを渡す。
 * 比較はキーの名前ではなくコードで行う（"ENTER" と "RETURN"、"a" と "A" のような別名を同じキーと判定する）。
 * 変更前の主キーが解釈できないとき（読み込み時に直しているので通常は起きない）は、相手を既定値に戻す。
 */
void swap_duplicate_key( SettingStore& store, const string_t& key, const SettingStore::Value& old_value )
{
	const int new_code = primary_code( store.get< string_t >( key ) );

	if ( new_code == 0 )
	{
		return;
	}

	const auto* old_text = std::get_if< string_t >( & old_value );
	const string_t old_primary = old_text ? get_primary_key( *old_text ) : string_t();
	const bool old_valid = key_names::code_from_name( old_primary ) != 0;

	for ( const auto& b : get_bindable_buttons() )
	{
		const string_t other = Input::get_key_config_name( b.button );

		if ( other == key || primary_code( store.get< string_t >( other ) ) != new_code )
		{
			continue;
		}

		store.set( other, old_valid ? replace_primary_key( store.get< string_t >( other ), old_primary ) : string_t( default_key( b.button ) ) );
	}
}

/// 表示系の項目を登録する（App の起動時の読み込みと、ゲーム中のストアで共有する。既定値と範囲の唯一の定義）
void register_display( SettingStore& store )
{
	store.add_int( SCREEN_WIDTH, 1920, 640, 7680, ApplyMode::DEFERRED );
	store.add_int( SCREEN_HEIGHT, 1080, 480, 4320, ApplyMode::DEFERRED );
	store.add_bool( FULL_SCREEN, false, ApplyMode::DEFERRED );

	store.set_info( SCREEN_WIDTH, "options.resolution", "options.resolution.description" );
	store.set_info( FULL_SCREEN, "options.full_screen", "options.full_screen.description" );
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
	store.set_info( AUDIO_VOLUME, "options.volume", "options.volume.description" );

	store.add_bool( AUDIO_MUTE, false, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_sound_manager()->set_mute( std::get< bool >( v ) );
	} );
	store.set_info( AUDIO_MUTE, "options.mute", "options.mute.description" );

	// 操作
	store.add_float( MOUSE_X_SENSITIVITY, 1.f, 0.1f, 3.f, 0.1f, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_input()->set_mouse_x_sensitivity( std::get< float_t >( v ) );
	} );
	store.set_info( MOUSE_X_SENSITIVITY, "options.mouse_sensitivity", "options.mouse_sensitivity.description" );

	store.add_float( MOUSE_Y_SENSITIVITY, 1.f, 0.1f, 3.f, 0.1f, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->get_input()->set_mouse_y_sensitivity( std::get< float_t >( v ) );
	} );
	store.set_info( MOUSE_Y_SENSITIVITY, "options.mouse_sensitivity", "options.mouse_sensitivity.description" );

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

		store.set_info( key, b.label_key, "options.key.description" );
		store.set_on_set( key, & swap_duplicate_key );
	}

	// どの経路（設定・読み込み・元に戻す・既定値に戻す）で変わっても、キー割り当てを整える
	store.set_normalizer( [] ( SettingStore& s ) { normalize_key_bindings( s ); } );

	// カメラ（反映はシーン側が change_count を見て行う）
	store.add_float( CAMERA_FOV, 90.f, 50.f, 120.f, 1.f, ApplyMode::IMMEDIATE );
	store.set_info( CAMERA_FOV, "options.fov", "options.fov.description" );

	// グラフィックス
	store.add_bool( DISPLAY_FPS, false, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->set_display_fps( std::get< bool >( v ) );
	} );
	store.set_info( DISPLAY_FPS, "options.display_fps", "options.display_fps.description" );

	// 垂直同期（以前は選べず、常に待たなかった。既定は以前と同じ「待たない」）
	store.add_bool( VSYNC, false, ApplyMode::IMMEDIATE, [gm] ( const SettingStore::Value& v )
	{
		gm->set_vsync( std::get< bool >( v ) );
	} );
	store.set_info( VSYNC, "options.vsync", "options.vsync.description" );

	// 文字の大きさ（アクセシビリティ）。行の高さは部品が文字に合わせて広げる
	// ドラッグの途中で画面全体を配置し直すと重く、行が動いて操作しにくいので、離したときに反映する
	store.add_float( UI_TEXT_SCALE, 1.f, 0.8f, 1.6f, 0.1f, ApplyMode::ON_RELEASE, [gm] ( const SettingStore::Value& v )
	{
		gm->set_ui_text_scale( std::get< float_t >( v ) );
	} );
	store.set_info( UI_TEXT_SCALE, "options.text_scale", "options.text_scale.description" );

	register_display( store );

	// 表示系はまとめて 1 回、フレームの最後に反映する
	store.add_apply_group( get_display_keys(), [gm] { gm->request_display_change(); } );
}

bool normalize_key_bindings( SettingStore& store )
{
	// キー割り当て : 解釈できない・割り当てできない・空のキーは既定値に直す
	// （以前は Input は既定値に戻すのにストアは壊れた文字列のまま保持し、画面は空欄・保存も壊れたままだった）
	// 別の操作と同じキーなら、後の方を既定値に直す（読み込み・元に戻すの経路でも重複を残さない）
	bool fixed = false;
	std::vector< int > used;

	for ( const auto& b : get_bindable_buttons() )
	{
		const string_t key = Input::get_key_config_name( b.button );
		int code = primary_code( store.get< string_t >( key ) );

		const bool invalid = code == 0 || ! key_names::is_bindable( code );
		const bool duplicate = ! invalid && std::find( used.begin(), used.end(), code ) != used.end();

		if ( invalid || duplicate )
		{
			store.set( key, default_key( b.button ) );
			code = primary_code( default_key( b.button ) );
			fixed = true;
		}

		used.push_back( code );
	}

	return fixed;
}

void fix_invalid_values( SettingStore& store )
{
	const bool fixed = normalize_key_bindings( store );

	if ( fixed )
	{
		store.commit_immediate();
	}
}

string_t user_settings_path()
{
	std::filesystem::path dir;

	if ( wchar_t env[ MAX_PATH ] = { }; GetEnvironmentVariableW( L"BLUE_SKY_SETTINGS_DIR", env, MAX_PATH ) > 0 )
	{
		dir = env;
	}
	else
	{
		PWSTR local_app_data = nullptr;

		if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, & local_app_data ) ) )
		{
			dir = std::filesystem::path( local_app_data ) / L"blue-sky";
		}

		CoTaskMemFree( local_app_data );
	}

	if ( dir.empty() )
	{
		return LEGACY_USER_SETTINGS_PATH;		// ユーザーのフォルダが分からないときは、以前と同じ作業ディレクトリ
	}

	// Config のファイル操作は char のパス（ANSI）で行うので、ANSI に変換して返す
	return ( dir / L"settings.config" ).string();
}

void load_user_settings_file( game::Config& user_config, const game::Config& legacy_config )
{
	const string_t path = user_settings_path();

	if ( std::filesystem::exists( path ) )
	{
		user_config.load_file( path.c_str() );
		return;
	}

	// 新しい置き場所にまだ無い : 以前の置き場所（作業ディレクトリ）にあれば引き継ぐ。次の保存から新しい置き場所に書く
	if ( std::filesystem::exists( LEGACY_USER_SETTINGS_PATH ) )
	{
		user_config.load_file( LEGACY_USER_SETTINGS_PATH );
		return;
	}

	// まだユーザーの設定ファイルが無い : 従来の blue-sky.config に書かれていた値を引き継ぐ
	// （ストアの load() は登録された項目だけを読むので、ここでは全部写してよい）
	for ( const auto& [ name, value ] : legacy_config.get_values() )
	{
		user_config.set< string_t >( name.c_str(), value );
	}
}

void read_initial_display( const game::Config& user_config, int& width, int& height, bool& full_screen )
{
	SettingStore store;
	register_display( store );
	store.load( user_config );

	width = store.get( SCREEN_WIDTH );
	height = store.get( SCREEN_HEIGHT );
	full_screen = store.get( FULL_SCREEN );
}

} // namespace blue_sky::game_settings

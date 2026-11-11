#include "KeyNames.h"

#include <windows.h>

#include <algorithm>
#include <cctype>

namespace blue_sky::key_names
{

namespace
{

struct Entry
{
	const char_t* name;
	int code;
};

/**
 * 名前付きキーの表（英字・数字 1 文字は別処理）
 *
 * 同じコードに複数の名前がある場合、先に書いたものが name_from_code() の結果になる。
 */
const Entry NAMED_KEYS[] =
{
	{ "MOUSE_L", VK_LBUTTON },
	{ "MOUSE_R", VK_RBUTTON },
	{ "MOUSE_M", VK_MBUTTON },
	{ "MOUSE_X1", VK_XBUTTON1 },
	{ "MOUSE_X2", VK_XBUTTON2 },

	{ "SPACE", VK_SPACE },
	{ "ENTER", VK_RETURN },
	{ "RETURN", VK_RETURN },
	{ "TAB", VK_TAB },
	{ "BACKSPACE", VK_BACK },
	{ "ESCAPE", VK_ESCAPE },
	{ "ESC", VK_ESCAPE },

	{ "SHIFT", VK_SHIFT },
	{ "LSHIFT", VK_LSHIFT },
	{ "RSHIFT", VK_RSHIFT },
	{ "CTRL", VK_CONTROL },
	{ "CONTROL", VK_CONTROL },
	{ "LCTRL", VK_LCONTROL },
	{ "RCTRL", VK_RCONTROL },
	{ "ALT", VK_MENU },
	{ "LALT", VK_LMENU },
	{ "RALT", VK_RMENU },
	{ "CAPSLOCK", VK_CAPITAL },

	{ "UP", VK_UP },
	{ "DOWN", VK_DOWN },
	{ "LEFT", VK_LEFT },
	{ "RIGHT", VK_RIGHT },

	{ "INSERT", VK_INSERT },
	{ "DELETE", VK_DELETE },
	{ "HOME", VK_HOME },
	{ "END", VK_END },
	{ "PAGE_UP", VK_PRIOR },
	{ "PAGE_DOWN", VK_NEXT },

	{ "F1", VK_F1 }, { "F2", VK_F2 }, { "F3", VK_F3 }, { "F4", VK_F4 },
	{ "F5", VK_F5 }, { "F6", VK_F6 }, { "F7", VK_F7 }, { "F8", VK_F8 },
	{ "F9", VK_F9 }, { "F10", VK_F10 }, { "F11", VK_F11 }, { "F12", VK_F12 },

	{ "NUMPAD0", VK_NUMPAD0 }, { "NUMPAD1", VK_NUMPAD1 }, { "NUMPAD2", VK_NUMPAD2 },
	{ "NUMPAD3", VK_NUMPAD3 }, { "NUMPAD4", VK_NUMPAD4 }, { "NUMPAD5", VK_NUMPAD5 },
	{ "NUMPAD6", VK_NUMPAD6 }, { "NUMPAD7", VK_NUMPAD7 }, { "NUMPAD8", VK_NUMPAD8 },
	{ "NUMPAD9", VK_NUMPAD9 },
	{ "NUMPAD_ADD", VK_ADD },
	{ "NUMPAD_SUBTRACT", VK_SUBTRACT },
	{ "NUMPAD_MULTIPLY", VK_MULTIPLY },
	{ "NUMPAD_DIVIDE", VK_DIVIDE },
	{ "NUMPAD_DECIMAL", VK_DECIMAL },

	// 記号キー（US 配列基準の名前。日本語配列でも VK は同じ）
	{ "COMMA", VK_OEM_COMMA },
	{ "PERIOD", VK_OEM_PERIOD },
	{ "MINUS", VK_OEM_MINUS },
	{ "PLUS", VK_OEM_PLUS },
	{ "SEMICOLON", VK_OEM_1 },
	{ "SLASH", VK_OEM_2 },
	{ "BACKQUOTE", VK_OEM_3 },
	{ "LBRACKET", VK_OEM_4 },
	{ "BACKSLASH", VK_OEM_5 },
	{ "RBRACKET", VK_OEM_6 },
	{ "QUOTE", VK_OEM_7 },
	{ "BACKSLASH_JP", VK_OEM_102 },
};

string_t to_upper( const string_t& s )
{
	string_t r = s;
	std::transform( r.begin(), r.end(), r.begin(), [] ( char_t c ) { return static_cast< char_t >( std::toupper( static_cast< unsigned char >( c ) ) ); } );
	return r;
}

bool is_alnum_code( int code )
{
	return ( code >= 'A' && code <= 'Z' ) || ( code >= '0' && code <= '9' );
}

} // namespace

int code_from_name( const string_t& name )
{
	const string_t upper = to_upper( name );

	// パッドのボタン : PAD1 〜 PAD32
	if ( upper.size() > 3 && upper.compare( 0, 3, "PAD" ) == 0 && upper.find_first_not_of( "0123456789", 3 ) == string_t::npos )
	{
		const int n = std::atoi( upper.c_str() + 3 );
		return n >= 1 && n <= PAD_BUTTON_COUNT ? PAD_BUTTON_BASE + n - 1 : 0;
	}

	if ( upper.size() == 1 && is_alnum_code( upper[ 0 ] ) )
	{
		return upper[ 0 ];
	}

	for ( const auto& e : NAMED_KEYS )
	{
		if ( upper == e.name )
		{
			return e.code;
		}
	}

	return 0;
}

string_t name_from_code( int code )
{
	if ( is_pad_code( code ) )
	{
		return "PAD" + std::to_string( pad_button_index( code ) + 1 );
	}

	if ( is_alnum_code( code ) )
	{
		return string_t( 1, static_cast< char_t >( code ) );
	}

	for ( const auto& e : NAMED_KEYS )
	{
		if ( e.code == code )
		{
			return e.name;
		}
	}

	return string_t();
}

const std::vector< int >& get_bindable_codes()
{
	static const std::vector< int > codes = []
	{
		std::vector< int > v;

		for ( int c = '0'; c <= '9'; c++ ) v.push_back( c );
		for ( int c = 'A'; c <= 'Z'; c++ ) v.push_back( c );

		for ( const auto& e : NAMED_KEYS )
		{
			// ESC はキャンセルに予約。F1 / F2 / F5 はグローバルなホットキー（コマンドモード・ミュート・フルスクリーン）。
			// 矢印キーと Enter は固定キー（Input の fixed_key_code_ で方向と A に割り当て済み）なので、
			// 別のボタンに割り当てると 1 回の押下で 2 つのボタンが入る（例: ジャンプを Enter にすると決定とジャンプが同時に入る）。
			// 左右を区別しない SHIFT / CTRL / ALT は、走査では左右別のコードが拾えるので除外する
			if ( e.code == VK_ESCAPE || e.code == VK_F1 || e.code == VK_F2 || e.code == VK_F5 ||
				e.code == VK_RETURN || e.code == VK_UP || e.code == VK_DOWN || e.code == VK_LEFT || e.code == VK_RIGHT ||
				e.code == VK_SHIFT || e.code == VK_CONTROL || e.code == VK_MENU )
			{
				continue;
			}

			if ( std::find( v.begin(), v.end(), e.code ) == v.end() )
			{
				v.push_back( e.code );
			}
		}

		// パッドのボタン
		for ( int i = 0; i < PAD_BUTTON_COUNT; i++ )
		{
			v.push_back( PAD_BUTTON_BASE + i );
		}

		return v;
	} ();

	return codes;
}

bool is_bindable( int code )
{
	const auto& codes = get_bindable_codes();
	return std::find( codes.begin(), codes.end(), code ) != codes.end();
}

} // namespace blue_sky::key_names

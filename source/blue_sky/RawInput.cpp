#include "RawInput.h"

#include <game/Game.h>

namespace blue_sky::raw_input
{

void Decoder::decode( const RAWINPUT& raw, std::vector< Event >& out )
{
	if ( raw.header.dwType == RIM_TYPEKEYBOARD )
	{
		const RAWKEYBOARD& k = raw.data.keyboard;

		// 0xFF は、いくつかのキーの組み合わせで送られる偽物の出来事
		if ( k.VKey == 0 || k.VKey >= 0xFF )
		{
			return;
		}

		const int vk = k.VKey;
		const bool up = ( k.Flags & RI_KEY_BREAK ) != 0;

		if ( up )
		{
			key_down_[ vk ] = false;
			out.push_back( Event{ Event::Type::KEY_UP, vk } );
		}
		else if ( ! key_down_[ vk ] )
		{
			key_down_[ vk ] = true;
			out.push_back( Event{ Event::Type::KEY_DOWN, vk } );
		}

		return;
	}

	if ( raw.header.dwType == RIM_TYPEMOUSE )
	{
		const RAWMOUSE& m = raw.data.mouse;

		// 相対の移動量だけを使う（絶対座標はリモートデスクトップやペンタブレット。カーソルの位置は別に読んでいる）
		if ( ! ( m.usFlags & MOUSE_MOVE_ABSOLUTE ) && ( m.lLastX != 0 || m.lLastY != 0 ) )
		{
			out.push_back( Event{ Event::Type::MOUSE_MOVE, 0, m.lLastX, m.lLastY } );
		}

		struct ButtonFlag { USHORT down; USHORT up; int vk; };
		static constexpr ButtonFlag buttons[] =
		{
			{ RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP, VK_LBUTTON },
			{ RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP, VK_RBUTTON },
			{ RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP, VK_MBUTTON },
		};

		for ( const auto& b : buttons )
		{
			if ( m.usButtonFlags & b.down )
			{
				out.push_back( Event{ Event::Type::KEY_DOWN, b.vk } );
			}

			if ( m.usButtonFlags & b.up )
			{
				out.push_back( Event{ Event::Type::KEY_UP, b.vk } );
			}
		}

		if ( m.usButtonFlags & RI_MOUSE_WHEEL )
		{
			out.push_back( Event{ Event::Type::MOUSE_WHEEL, 0, 0, static_cast< SHORT >( m.usButtonData ) } );
		}
	}
}

void dispatch( const std::vector< Event >& events, game::Game& game, const std::function< void( const std::string& ) >& log )
{
	static bool logged_key = false;
	static bool logged_mouse = false;

	for ( const auto& e : events )
	{
		const bool is_mouse = e.type == Event::Type::MOUSE_MOVE || e.type == Event::Type::MOUSE_WHEEL || e.code == VK_LBUTTON || e.code == VK_RBUTTON || e.code == VK_MBUTTON;
		bool& logged = is_mouse ? logged_mouse : logged_key;

		if ( log && ! logged )
		{
			logged = true;
			log( std::string( "[Input] Raw Input で最初の出来事を受け取りました : " ) + ( is_mouse ? "マウス" : "キー" ) + "（コード " + std::to_string( e.code ) + "・移動量 " + std::to_string( e.dx ) + ", " + std::to_string( e.dy ) + "）" );
		}

		if ( e.type == Event::Type::KEY_DOWN )
		{
			game.on_raw_key_down( e.code );
		}
		else if ( e.type == Event::Type::MOUSE_MOVE )
		{
			game.on_raw_mouse_move( e.dx, e.dy );
		}
	}
}

bool register_devices( HWND hwnd )
{
	// 0x01 = Generic Desktop、0x02 = マウス、0x06 = キーボード。
	// RIDEV_NOLEGACY は付けない（文字入力・IME・ファンクションキーの処理は WM_KEYDOWN・WM_CHAR を使い続ける）
	RAWINPUTDEVICE devices[ 2 ] = {};
	devices[ 0 ].usUsagePage = 0x01;
	devices[ 0 ].usUsage = 0x02;
	devices[ 0 ].hwndTarget = hwnd;
	devices[ 1 ].usUsagePage = 0x01;
	devices[ 1 ].usUsage = 0x06;
	devices[ 1 ].hwndTarget = hwnd;

	return RegisterRawInputDevices( devices, 2, sizeof( RAWINPUTDEVICE ) ) != FALSE;
}

bool read( LPARAM lp, Decoder& decoder, std::vector< Event >& out )
{
	RAWINPUT raw = {};
	UINT size = sizeof( raw );

	if ( GetRawInputData( reinterpret_cast< HRAWINPUT >( lp ), RID_INPUT, & raw, & size, sizeof( RAWINPUTHEADER ) ) == static_cast< UINT >( -1 ) )
	{
		return false;
	}

	decoder.decode( raw, out );
	return true;
}

} // namespace blue_sky::raw_input

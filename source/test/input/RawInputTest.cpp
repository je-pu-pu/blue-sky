#include "pch.h"

#include <blue_sky/RawInput.h>

using blue_sky::raw_input::Decoder;
using blue_sky::raw_input::Event;

// ---- 入力のイベント化（設計上の問題 B27）: Raw Input の出来事の読み取り ----

namespace
{

RAWINPUT make_key( USHORT vk, bool up )
{
	RAWINPUT raw = {};
	raw.header.dwType = RIM_TYPEKEYBOARD;
	raw.data.keyboard.VKey = vk;
	raw.data.keyboard.Flags = up ? RI_KEY_BREAK : RI_KEY_MAKE;
	return raw;
}

RAWINPUT make_mouse( LONG dx, LONG dy, USHORT button_flags = 0, USHORT button_data = 0 )
{
	RAWINPUT raw = {};
	raw.header.dwType = RIM_TYPEMOUSE;
	raw.data.mouse.usFlags = MOUSE_MOVE_RELATIVE;
	raw.data.mouse.lLastX = dx;
	raw.data.mouse.lLastY = dy;
	raw.data.mouse.usButtonFlags = button_flags;
	raw.data.mouse.usButtonData = button_data;
	return raw;
}

} // namespace

TEST( RawInputTest, キーの押し続けによる繰り返しは1回の押下にまとめる )
{
	Decoder decoder;
	std::vector< Event > events;

	decoder.decode( make_key( 'A', false ), events );
	decoder.decode( make_key( 'A', false ), events );		// 押し続けで送られる
	decoder.decode( make_key( 'A', false ), events );
	decoder.decode( make_key( 'A', true ), events );
	decoder.decode( make_key( 'A', false ), events );		// もう一度押した

	ASSERT_EQ( 3u, events.size() );
	EXPECT_EQ( Event::Type::KEY_DOWN, events[ 0 ].type );
	EXPECT_EQ( 'A', events[ 0 ].code );
	EXPECT_EQ( Event::Type::KEY_UP, events[ 1 ].type );
	EXPECT_EQ( Event::Type::KEY_DOWN, events[ 2 ].type );
}

TEST( RawInputTest, 同じフレームの中で押して離した短いクリックも順に残る )
{
	Decoder decoder;
	std::vector< Event > events;

	decoder.decode( make_mouse( 0, 0, RI_MOUSE_LEFT_BUTTON_DOWN ), events );
	decoder.decode( make_mouse( 0, 0, RI_MOUSE_LEFT_BUTTON_UP ), events );

	ASSERT_EQ( 2u, events.size() );
	EXPECT_EQ( Event::Type::KEY_DOWN, events[ 0 ].type );
	EXPECT_EQ( VK_LBUTTON, events[ 0 ].code );
	EXPECT_EQ( Event::Type::KEY_UP, events[ 1 ].type );
}

TEST( RawInputTest, マウスの相対の移動量とホイールを読み絶対座標は使わない )
{
	Decoder decoder;
	std::vector< Event > events;

	decoder.decode( make_mouse( 5, -3 ), events );
	decoder.decode( make_mouse( 0, 0, RI_MOUSE_WHEEL, static_cast< USHORT >( -120 ) ), events );

	RAWINPUT absolute = make_mouse( 30000, 30000 );
	absolute.data.mouse.usFlags = MOUSE_MOVE_ABSOLUTE;
	decoder.decode( absolute, events );

	ASSERT_EQ( 2u, events.size() );
	EXPECT_EQ( Event::Type::MOUSE_MOVE, events[ 0 ].type );
	EXPECT_EQ( 5, events[ 0 ].dx );
	EXPECT_EQ( -3, events[ 0 ].dy );
	EXPECT_EQ( Event::Type::MOUSE_WHEEL, events[ 1 ].type );
	EXPECT_EQ( -120, events[ 1 ].dy );
}

TEST( RawInputTest, フォーカスを失ったら押している状態を忘れる )
{
	Decoder decoder;
	std::vector< Event > events;

	decoder.decode( make_key( VK_SPACE, false ), events );
	decoder.reset();		// 離したことを受け取れなかった
	decoder.decode( make_key( VK_SPACE, false ), events );

	ASSERT_EQ( 2u, events.size() );
	EXPECT_EQ( Event::Type::KEY_DOWN, events[ 1 ].type );
}

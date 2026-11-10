#include "pch.h"

#include <blue_sky/RawInput.h>
#include <game/Game.h>

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

namespace
{

/// 受け取ったものを記録するゲーム（App がゲームへ渡す経路の確認用）
class RecordingGame : public game::Game
{
public:
	std::vector< int > pressed;
	long move_x = 0;
	long move_y = 0;

	bool update() override { return true; }
	void on_key_down( char_t ) override { }
	void on_special_key_down( int ) override { }
	void on_mouse_wheel( int ) override { }
	void on_resize() override { }
	int get_width() const override { return 0; }
	int get_height() const override { return 0; }
	bool is_show_cursor() const override { return false; }

	void on_raw_key_down( int vk ) override { pressed.push_back( vk ); }
	void on_raw_mouse_move( long dx, long dy ) override { move_x += dx; move_y += dy; }
};

} // namespace

TEST( RawInputTest, RAWINPUTからゲームまで押下と移動量が順に届く )
{
	Decoder decoder;
	std::vector< Event > events;

	// 1 フレームの間に届いた RAWINPUT（押して離したクリック・キー・移動 2 回・ホイール）
	decoder.decode( make_mouse( 0, 0, RI_MOUSE_LEFT_BUTTON_DOWN ), events );
	decoder.decode( make_mouse( 0, 0, RI_MOUSE_LEFT_BUTTON_UP ), events );
	decoder.decode( make_key( 'W', false ), events );
	decoder.decode( make_key( 'W', false ), events );		// 押し続け
	decoder.decode( make_mouse( 7, 1 ), events );
	decoder.decode( make_mouse( 3, -4 ), events );
	decoder.decode( make_mouse( 0, 0, RI_MOUSE_WHEEL, 120 ), events );

	RecordingGame game;
	std::vector< std::string > logs;
	blue_sky::raw_input::dispatch( events, game, [&] ( const std::string& s ) { logs.push_back( s ); } );

	ASSERT_EQ( 2u, game.pressed.size() );
	EXPECT_EQ( VK_LBUTTON, game.pressed[ 0 ] );		// 離す前に押したことが届く（短いクリックを取りこぼさない）
	EXPECT_EQ( 'W', game.pressed[ 1 ] );			// 押し続けは 1 回
	EXPECT_EQ( 10, game.move_x );					// 移動量は合計で届く
	EXPECT_EQ( -3, game.move_y );
}

#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/Button.h>
#include <core/ui/Dialog.h>
#include <core/ui/Label.h>
#include <core/ui/Stack.h>

using namespace core::ui;
using ui_test::ScreenFixture;

namespace
{

/**
 * 縦リスト（ボタン 4 つ）＋ 下に横のボタン行（3 つ）の画面
 */
class NavTest : public ::testing::Test, public ScreenFixture
{
protected:
	std::vector< Button* > rows;
	std::vector< Button* > footer;
	Stack* list = nullptr;
	int clicked = -1;
	int back_count = 0;

	void SetUp() override
	{
		auto root = std::make_unique< Stack >( Axis::VERTICAL );
		list = root->add< Stack >( Axis::VERTICAL );

		for ( int i = 0; i < 4; i++ )
		{
			rows.push_back( list->add< Button >( "row" + std::to_string( i ), [this, i] { clicked = i; } ) );
		}

		auto* bar = root->add< Stack >( Axis::HORIZONTAL );

		for ( int i = 0; i < 3; i++ )
		{
			footer.push_back( bar->add< Button >( "btn" + std::to_string( i ), [this, i] { clicked = 100 + i; } ) );
		}

		screen.set_root( std::move( root ), [this] { back_count++; } );
	}
};

} // namespace

// ---- ナビゲーション ----

TEST_F( NavTest, 最初のフォーカスは先頭 )
{
	EXPECT_EQ( rows[ 0 ], screen.get_focused() );
	EXPECT_TRUE( rows[ 0 ]->is_focused() );
}

TEST_F( NavTest, 上下で移動し上端では止まる )
{
	press_nav( NavDir::DOWN );
	EXPECT_EQ( rows[ 1 ], screen.get_focused() );

	press_nav( NavDir::UP );
	press_nav( NavDir::UP );
	EXPECT_EQ( rows[ 0 ], screen.get_focused() );
}

TEST_F( NavTest, リスト末尾から下でボタン行へ抜け上で元の行に戻る )
{
	for ( int i = 0; i < 3; i++ ) press_nav( NavDir::DOWN );
	EXPECT_EQ( rows[ 3 ], screen.get_focused() );

	press_nav( NavDir::DOWN );
	// 横の行には直前の位置（全幅の行の中央）に近いボタンから入る
	EXPECT_EQ( footer[ 1 ], screen.get_focused() );

	press_nav( NavDir::LEFT );
	EXPECT_EQ( footer[ 0 ], screen.get_focused() );

	// 上に戻ると、リストの中で最後にフォーカスしていた行へ
	press_nav( NavDir::UP );
	EXPECT_EQ( rows[ 3 ], screen.get_focused() );
}

TEST_F( NavTest, 横のボタン行は端で止まる )
{
	screen.set_focus( footer[ 0 ], FocusReason::PROGRAM );
	press_nav( NavDir::LEFT );
	EXPECT_EQ( footer[ 0 ], screen.get_focused() );

	press_nav( NavDir::RIGHT );
	press_nav( NavDir::RIGHT );
	press_nav( NavDir::RIGHT );
	EXPECT_EQ( footer[ 2 ], screen.get_focused() );
}

TEST_F( NavTest, 無効と非表示の部品は飛ばす )
{
	rows[ 1 ]->set_enabled( false );
	rows[ 2 ]->set_visible( false );
	screen.invalidate_layout();

	press_nav( NavDir::DOWN );
	EXPECT_EQ( rows[ 3 ], screen.get_focused() );
}

TEST_F( NavTest, フォーカス中の部品が無効になると近くの部品へ移る )
{
	bool enabled = true;
	rows[ 1 ]->set_enabled_if( [&enabled] { return enabled; } );

	press_nav( NavDir::DOWN );
	ASSERT_EQ( rows[ 1 ], screen.get_focused() );

	enabled = false;
	step();

	EXPECT_EQ( rows[ 2 ], screen.get_focused() );
	EXPECT_FALSE( rows[ 1 ]->is_focused() );
}

TEST( StackWrapTest, wrap指定のリストは端で反対側へ回る )
{
	ScreenFixture f;
	auto root = std::make_unique< Stack >( Axis::VERTICAL );
	root->set_wrap( true );
	auto* a = root->add< Button >( "a", nullptr );
	root->add< Button >( "b", nullptr );
	auto* c = root->add< Button >( "c", nullptr );
	f.screen.set_root( std::move( root ), nullptr );

	f.press_nav( NavDir::UP );
	EXPECT_EQ( c, f.screen.get_focused() );

	f.press_nav( NavDir::DOWN );
	EXPECT_EQ( a, f.screen.get_focused() );
}

// ---- 決定・キャンセル ----

TEST_F( NavTest, 決定でフォーカス中のボタンが押される )
{
	press_nav( NavDir::DOWN );
	confirm();
	EXPECT_EQ( 1, clicked );
}

TEST_F( NavTest, キャンセルで画面の戻るが呼ばれる )
{
	cancel();
	EXPECT_EQ( 1, back_count );
}

TEST_F( NavTest, 開いた直後の抑止フレームでは入力を捨てる )
{
	screen.suppress_input( 1 );

	cancel();	// 捨てられる（ESC でポーズを開いた同じ ESC で閉じない）
	EXPECT_EQ( 0, back_count );

	cancel();
	EXPECT_EQ( 1, back_count );
}

// ---- ポインタ ----

TEST_F( NavTest, ホバーでフォーカスが移る )
{
	const Rect& r = rows[ 2 ]->rect();
	move_pointer( r.center_x(), r.center_y() );
	EXPECT_EQ( rows[ 2 ], screen.get_focused() );
}

TEST_F( NavTest, 部品の外のクリックではフォーカス中のボタンは押されない )
{
	// 旧実装の不具合 A1 : どこをクリックしてもフォーカス中のボタンが押されていた
	click( 1900.f, 1070.f );
	EXPECT_EQ( -1, clicked );
}

TEST_F( NavTest, 部品の上で押して離すとクリック )
{
	const Rect& r = rows[ 2 ]->rect();
	click( r.center_x(), r.center_y() );
	EXPECT_EQ( 2, clicked );
}

TEST_F( NavTest, 押した後に外で離すとクリックにならない )
{
	const Rect& r = rows[ 2 ]->rect();
	click( r.center_x(), r.center_y(), 1900.f, 1070.f );
	EXPECT_EQ( -1, clicked );
}

// ---- ダイアログ ----

namespace
{

class DialogTest : public NavTest
{
protected:
	std::vector< int > pressed;

	void open( float timeout = 0.f )
	{
		DialogSpec spec;
		spec.message = "message";
		spec.buttons = { { "yes", [this] { pressed.push_back( 0 ); } }, { "no", [this] { pressed.push_back( 1 ); } } };
		spec.cancel_index = 1;
		spec.default_index = 0;
		spec.timeout_sec = timeout;
		screen.show_dialog( std::move( spec ) );
		step();	// post されたコマンドで開く
	}
};

} // namespace

TEST_F( DialogTest, ダイアログは最前面の層になり背面に入力が届かない )
{
	open();
	ASSERT_EQ( 2u, screen.get_layer_count() );

	auto* dialog = dynamic_cast< Dialog* >( screen.get_focused()->parent()->parent() );
	ASSERT_NE( nullptr, dialog );

	// 背面の行をクリックしても押されない
	const Rect& r = rows[ 2 ]->rect();
	click( r.center_x(), r.center_y() );
	EXPECT_EQ( -1, clicked );

	// 背面の「戻る」も呼ばれない（キャンセルはダイアログが受ける）
	cancel();
	EXPECT_EQ( 0, back_count );
	ASSERT_EQ( 1u, pressed.size() );
	EXPECT_EQ( 1, pressed[ 0 ] );
	EXPECT_EQ( 1u, screen.get_layer_count() );
}

TEST_F( DialogTest, 決定で既定のボタンが押され閉じる )
{
	open();
	confirm();

	ASSERT_EQ( 1u, pressed.size() );
	EXPECT_EQ( 0, pressed[ 0 ] );
	EXPECT_EQ( 1u, screen.get_layer_count() );
	EXPECT_EQ( rows[ 0 ], screen.get_focused() );	// 背面のフォーカスは元のまま
}

TEST_F( DialogTest, 時間切れでキャンセル扱い )
{
	open( 0.5f );

	for ( int i = 0; i < 20; i++ ) step( core::ui::UIInput(), 0.05f );

	ASSERT_EQ( 1u, pressed.size() );
	EXPECT_EQ( 1, pressed[ 0 ] );
	EXPECT_EQ( 1u, screen.get_layer_count() );
}

TEST_F( DialogTest, コールバックから次のダイアログを開ける )
{
	// 旧設計のレビュー指摘（致命 2）: 閉じる処理が新しいダイアログを巻き込まないこと
	DialogSpec first;
	first.message = "first";
	first.buttons = { { "next", [this]
	{
		DialogSpec second;
		second.message = "second";
		second.buttons = { { "ok", [this] { pressed.push_back( 2 ); } } };
		screen.show_dialog( std::move( second ) );
	} } };
	screen.show_dialog( std::move( first ) );
	step();

	confirm();	// first を押す → second が開き、first は閉じる
	ASSERT_EQ( 2u, screen.get_layer_count() );

	render();
	EXPECT_TRUE( renderer.has_text( "second" ) );
	EXPECT_FALSE( renderer.has_text( "first" ) );

	confirm();
	EXPECT_EQ( 1u, screen.get_layer_count() );
	ASSERT_EQ( 1u, pressed.size() );
	EXPECT_EQ( 2, pressed[ 0 ] );
}

TEST_F( DialogTest, 二度押しされても一度しか呼ばれない )
{
	open();

	core::ui::UIInput in;
	in.confirm = true;
	in.cancel = true;	// 同じフレームに決定とキャンセル
	step( in );

	EXPECT_EQ( 1u, pressed.size() );
}

// ---- post ----

TEST_F( NavTest, postしたコマンドはupdateの最後に実行され実行中に積んだものも同じフレームで処理する )
{
	std::vector< int > order;

	screen.post( [&]
	{
		order.push_back( 1 );
		screen.post( [&] { order.push_back( 2 ); } );
	} );

	EXPECT_TRUE( order.empty() );
	step();

	ASSERT_EQ( 2u, order.size() );
	EXPECT_EQ( 1, order[ 0 ] );
	EXPECT_EQ( 2, order[ 1 ] );
}

// ---- 操作ヒント ----

TEST_F( NavTest, 操作ヒントは最後に使ったデバイスの表記になる )
{
	strings.set( "hint.key.confirm", "Enter" );
	strings.set( "hint.pad.confirm", "A" );
	strings.set( "hint.key.cancel", "Esc" );
	strings.set( "hint.pad.cancel", "B" );
	strings.set( "hint.select", "決定" );
	strings.set( "hint.back", "戻る" );

	step();
	EXPECT_EQ( "Enter 決定    Esc 戻る", screen.build_hint_text() );

	core::ui::UIInput pad;
	pad.last_device = InputDevice::GAMEPAD;
	step( pad );
	EXPECT_EQ( "A 決定    B 戻る", screen.build_hint_text() );
}

TEST_F( NavTest, 説明文はフォーカス中の部品のもの )
{
	strings.set( "desc.row1", "2 行目の説明" );
	rows[ 1 ]->set_description_key( "desc.row1" );

	EXPECT_EQ( "", screen.get_description() );
	press_nav( NavDir::DOWN );
	EXPECT_EQ( "2 行目の説明", screen.get_description() );
}

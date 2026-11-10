#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/Button.h>
#include <core/ui/Choice.h>
#include <core/ui/ClickTracker.h>
#include <core/ui/KeyBind.h>
#include <core/ui/Label.h>
#include <core/ui/ScrollView.h>
#include <core/ui/Slider.h>
#include <core/ui/Stack.h>
#include <core/ui/TabView.h>
#include <core/ui/Toggle.h>

using namespace core::ui;
using ui_test::ScreenFixture;

namespace
{

/// 縦の Stack に部品を並べるだけの画面
class WidgetTest : public ::testing::Test, public ScreenFixture
{
protected:
	Stack* root = nullptr;
	int back_count = 0;

	void SetUp() override
	{
		auto r = std::make_unique< Stack >( Axis::VERTICAL );
		root = r.get();
		screen.set_root( std::move( r ), [this] { back_count++; } );
	}

	/// 部品を追加した後に呼ぶ（レイアウトとフォーカスを確定させる）
	void ready()
	{
		screen.invalidate_layout();
		step();
		screen.focus_into( root );
	}
};

} // namespace

// ---- Slider ----

TEST_F( WidgetTest, スライダーは左右でstepずつ動き端でクランプされる )
{
	float value = 0.5f;
	root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.25f );
	ready();

	press_nav( NavDir::RIGHT );
	EXPECT_FLOAT_EQ( 0.75f, value );
	press_nav( NavDir::RIGHT );
	press_nav( NavDir::RIGHT );
	EXPECT_FLOAT_EQ( 1.f, value );
	press_nav( NavDir::LEFT );
	EXPECT_FLOAT_EQ( 0.75f, value );
}

TEST_F( WidgetTest, スライダーは決定では値が変わらない )
{
	// 旧実装の不具合 A1 : Enter でマウスの x 位置へ値が飛んでいた
	float value = 0.5f;
	root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.1f );
	ready();

	confirm();
	EXPECT_FLOAT_EQ( 0.5f, value );
}

TEST_F( WidgetTest, スライダーは溝をクリックとドラッグで値になりラベル部分では動かない )
{
	float value = 0.f;
	auto* slider = root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.1f );
	ready();

	const Rect track = slider->get_track_rect();

	// ラベル部分をクリック → 変わらない
	click( slider->rect().x + 10.f, track.center_y() );
	EXPECT_FLOAT_EQ( 0.f, value );

	// 溝の右端寄り（80%）をクリック
	click( track.x + track.w * 0.8f, track.center_y() );
	EXPECT_NEAR( 0.8f, value, 1e-4f );

	// 溝を押してドラッグ
	core::ui::UIInput p;
	p.pointer_x = track.x + track.w * 0.2f; p.pointer_y = track.center_y(); p.pointer_pressed = true; p.pointer_down = true;
	step( p );
	EXPECT_NEAR( 0.2f, value, 1e-4f );

	core::ui::UIInput d;
	d.pointer_x = track.x + track.w * 0.6f; d.pointer_y = track.center_y() + 200.f; d.pointer_down = true; d.pointer_moved = true;
	step( d );	// 行の外へ外れてもドラッグ中は追従する
	EXPECT_NEAR( 0.6f, value, 1e-4f );
}

// ---- Choice ----

TEST_F( WidgetTest, 選択肢は左右で送り端で止まる )
{
	int index = 0;
	root->add< Choice >( "res", std::vector< string_t >{ "a", "b", "c" }, [&] { return index; }, [&] ( int i ) { index = i; } );
	ready();

	press_nav( NavDir::LEFT );
	EXPECT_EQ( 0, index );
	press_nav( NavDir::RIGHT );
	press_nav( NavDir::RIGHT );
	press_nav( NavDir::RIGHT );
	EXPECT_EQ( 2, index );
}

TEST_F( WidgetTest, 選択肢のwrap指定は反対端へ回る )
{
	int index = 0;
	root->add< Choice >( "res", std::vector< string_t >{ "a", "b", "c" }, [&] { return index; }, [&] ( int i ) { index = i; } )->set_wrap( true );
	ready();

	press_nav( NavDir::LEFT );
	EXPECT_EQ( 2, index );
}

TEST_F( WidgetTest, 選択肢に無い現在値はカスタム表示し右で先頭へ )
{
	int index = -1;
	auto* choice = root->add< Choice >( "res", std::vector< string_t >{ "1920 x 1080", "1280 x 720" }, [&] { return index; }, [&] ( int i ) { index = i; } );
	choice->set_custom_text( [] { return string_t( "カスタム（1366 x 768）" ); } );
	ready();

	render();
	EXPECT_TRUE( renderer.has_text( "カスタム（1366 x 768）" ) );

	press_nav( NavDir::RIGHT );
	EXPECT_EQ( 0, index );
}

TEST_F( WidgetTest, 選択肢の矢印クリック )
{
	int index = 1;
	auto* choice = root->add< Choice >( "res", std::vector< string_t >{ "a", "b", "c" }, [&] { return index; }, [&] ( int i ) { index = i; } );
	ready();

	const Rect v = choice->get_value_rect();
	click( v.x + 5.f, v.center_y() );	// 左矢印
	EXPECT_EQ( 0, index );
	click( v.right() - 5.f, v.center_y() );	// 右矢印
	EXPECT_EQ( 1, index );
}

// ---- Toggle ----

TEST_F( WidgetTest, トグルは決定と左右とクリックで反転する )
{
	bool value = false;
	auto* toggle = root->add< Toggle >( "mute", [&] { return value; }, [&] ( bool v ) { value = v; } );
	ready();

	confirm();
	EXPECT_TRUE( value );
	press_nav( NavDir::LEFT );
	EXPECT_FALSE( value );
	click( toggle->rect().center_x(), toggle->rect().center_y() );
	EXPECT_TRUE( value );
}

TEST_F( WidgetTest, 値はgetterから毎回読むので外部の変更が表示に出る )
{
	bool value = false;
	root->add< Toggle >( "mute", [&] { return value; }, [&] ( bool v ) { value = v; } );
	strings.set( "ui.on", "オン" );
	strings.set( "ui.off", "オフ" );
	ready();

	render();
	EXPECT_TRUE( renderer.has_text( "オフ" ) );

	value = true;	// ストア側で revert / 既定値などが起きた想定
	render();
	EXPECT_TRUE( renderer.has_text( "オン" ) );
}

// ---- KeyBind ----

TEST_F( WidgetTest, キー割り当ては決定で待ちになり次のキーを受け取る )
{
	int key = 'W';
	auto* bind = root->add< KeyBind >( "up", [&] { return key; }, [&] ( int k ) { key = k; }, [] ( int k ) { return string_t( 1, static_cast< char >( k ) ); } );
	ready();

	// 決定と同じフレームに押されたキーは無視（決定に使った Enter が割り当てられないように）
	core::ui::UIInput start;
	start.confirm = true;
	start.pushed_key = 'X';
	step( start );
	EXPECT_TRUE( bind->is_waiting() );
	EXPECT_EQ( 'W', key );

	core::ui::UIInput k;
	k.pushed_key = 'I';
	step( k );
	EXPECT_FALSE( bind->is_waiting() );
	EXPECT_EQ( 'I', key );
}

TEST_F( WidgetTest, キー待ち中のキャンセルは中止だけで画面は閉じない )
{
	int key = 'W';
	auto* bind = root->add< KeyBind >( "up", [&] { return key; }, [&] ( int k ) { key = k; }, nullptr );
	ready();

	confirm();
	ASSERT_TRUE( bind->is_waiting() );

	cancel();
	EXPECT_FALSE( bind->is_waiting() );
	EXPECT_EQ( 'W', key );
	EXPECT_EQ( 0, back_count );

	cancel();	// 待っていなければ画面の「戻る」
	EXPECT_EQ( 1, back_count );
}

TEST_F( WidgetTest, キー待ち中は方向キーで移動しない )
{
	int key = 'W';
	auto* bind = root->add< KeyBind >( "up", [&] { return key; }, [&] ( int k ) { key = k; }, nullptr );
	root->add< Button >( "next", nullptr );
	ready();

	confirm();
	press_nav( NavDir::DOWN );
	EXPECT_EQ( bind, screen.get_focused() );
}

// ---- ScrollView ----

namespace
{

/// 表示範囲の高さを固定する ScrollView 置き場（高さ 300 の表示範囲に 10 行 = 行 72 + 間隔 8 で内容 792）
class FixedHeight : public Widget
{
public:
	float height;
	explicit FixedHeight( float h ) : height( h ) { }
	float measure_height( float, const UIContext& ) const override { return height; }
	void arrange( const Rect& r, const UIContext& ctx ) override
	{
		rect_ = Rect( r.x, r.y, r.w, height );
		for ( auto& c : children_ ) c->arrange( rect_, ctx );
	}
};

class ScrollFixture : public ::testing::Test, public ScreenFixture
{
protected:
	ScrollView* scroll = nullptr;
	std::vector< Button* > rows;

	void SetUp() override
	{
		auto box = std::make_unique< FixedHeight >( 300.f );
		scroll = box->add< ScrollView >();
		auto* list = scroll->add< Stack >( Axis::VERTICAL );

		for ( int i = 0; i < 10; i++ )
		{
			rows.push_back( list->add< Button >( "row" + std::to_string( i ), nullptr ) );
		}

		screen.set_root( std::move( box ), nullptr );
	}
};

} // namespace

TEST( ScrollKeyTest, フォーカスできる部品の無い縦長の内容は上下のキーでスクロールする )
{
	ui_test::ScreenFixture f;
	auto box = std::make_unique< FixedHeight >( 300.f );
	auto* scroll = box->add< ScrollView >();
	auto* list = scroll->add< Stack >( Axis::VERTICAL );

	for ( int i = 0; i < 30; i++ )
	{
		list->add< Label >( "line" + std::to_string( i ) );		// フォーカスできない（クレジットなど）
	}

	f.screen.set_root( std::move( box ), nullptr );
	f.step();
	ASSERT_GT( scroll->get_max_scroll(), 0.f );

	f.press_nav( NavDir::DOWN );
	EXPECT_GT( scroll->get_scroll(), 0.f );

	const float_t after_down = scroll->get_scroll();
	f.press_nav( NavDir::UP );
	EXPECT_LT( scroll->get_scroll(), after_down );
}

TEST_F( ScrollFixture, 方向キーで画面外の行へ進むと自動でスクロールする )
{
	EXPECT_FLOAT_EQ( 0.f, scroll->get_scroll() );

	for ( int i = 0; i < 5; i++ ) press_nav( NavDir::DOWN );
	step();

	EXPECT_EQ( rows[ 5 ], screen.get_focused() );
	EXPECT_GT( scroll->get_scroll(), 0.f );
	EXPECT_LE( rows[ 5 ]->rect().bottom(), scroll->rect().bottom() + 0.01f );
	EXPECT_GE( rows[ 5 ]->rect().y, scroll->rect().y - 0.01f );
}

TEST_F( ScrollFixture, ホバーではスクロールしない )
{
	// 3 行目（下端で半分はみ出す）にホバーしても、スクロールしない
	const Rect r = rows[ 3 ]->rect();
	move_pointer( r.center_x(), scroll->rect().bottom() - 5.f );
	step();

	EXPECT_FLOAT_EQ( 0.f, scroll->get_scroll() );
}

TEST_F( ScrollFixture, はみ出した行は描画もヒットもしない )
{
	// 行 0〜2 は完全に収まる（0〜232）。行 3 は 240〜312 で下端 300 からはみ出す
	render();
	EXPECT_TRUE( renderer.has_text( "row2" ) );
	EXPECT_FALSE( renderer.has_text( "row3" ) );

	// 行 3 の見えている部分をクリックしても当たらない
	move_pointer( rows[ 3 ]->rect().center_x(), 295.f );
	EXPECT_NE( rows[ 3 ], screen.get_focused() );
}

TEST_F( ScrollFixture, ホイールでスクロールし範囲で止まる )
{
	core::ui::UIInput w;
	w.pointer_x = 100.f; w.pointer_y = 100.f; w.wheel = -1;	// 手前へ = 下へ進む
	step( w );
	step();
	const float after_one = scroll->get_scroll();
	EXPECT_GT( after_one, 0.f );

	w.wheel = -100;
	step( w );
	step();
	EXPECT_FLOAT_EQ( scroll->get_max_scroll(), scroll->get_scroll() );

	w.wheel = 100;
	step( w );
	step();
	EXPECT_FLOAT_EQ( 0.f, scroll->get_scroll() );
}

// ---- TabView ----

namespace
{

class TabTest : public ::testing::Test, public ScreenFixture
{
protected:
	TabView* tabs = nullptr;
	std::vector< Button* > page_a;
	std::vector< Button* > page_b;

	void SetUp() override
	{
		auto root = std::make_unique< Stack >( Axis::VERTICAL );
		tabs = root->add< TabView >();

		auto* a = tabs->add_page< Stack >( "tab.a", Axis::VERTICAL );
		for ( int i = 0; i < 3; i++ ) page_a.push_back( a->add< Button >( "a" + std::to_string( i ), nullptr ) );

		auto* b = tabs->add_page< Stack >( "tab.b", Axis::VERTICAL );
		for ( int i = 0; i < 3; i++ ) page_b.push_back( b->add< Button >( "b" + std::to_string( i ), nullptr ) );

		screen.set_root( std::move( root ), nullptr );
	}

	void tab( int d ) { core::ui::UIInput in; in.tab_next = d > 0; in.tab_prev = d < 0; step( in ); }
};

} // namespace

TEST_F( TabTest, タブ切替でページとフォーカスが入れ替わる )
{
	EXPECT_EQ( page_a[ 0 ], screen.get_focused() );

	tab( +1 );
	EXPECT_EQ( 1u, tabs->get_current() );
	EXPECT_EQ( page_b[ 0 ], screen.get_focused() );

	// 非表示ページの部品はフォーカスできない
	EXPECT_FALSE( page_a[ 0 ]->can_focus() );

	render();
	EXPECT_TRUE( renderer.has_text( "b0" ) );
	EXPECT_FALSE( renderer.has_text( "a0" ) );
}

TEST_F( TabTest, ページに戻ると最後のフォーカス位置へ )
{
	press_nav( NavDir::DOWN );
	press_nav( NavDir::DOWN );
	ASSERT_EQ( page_a[ 2 ], screen.get_focused() );

	tab( +1 );
	tab( -1 );
	EXPECT_EQ( page_a[ 2 ], screen.get_focused() );
}

TEST_F( TabTest, 端のタブから反対端へ回り見出しクリックでも切り替わる )
{
	tab( -1 );
	EXPECT_EQ( 1u, tabs->get_current() );

	// 見出し行の左半分（タブ a）をクリック
	click( tabs->rect().x + 10.f, tabs->rect().y + 10.f );
	EXPECT_EQ( 0u, tabs->get_current() );
}

TEST( TabInitialPageTest, 最初に表示するページを決めるとそのページにフォーカスが入る )
{
	ScreenFixture f;
	auto root = std::make_unique< Stack >( Axis::VERTICAL );
	auto* tabs = root->add< TabView >();
	tabs->add_page< Stack >( "tab.a", Axis::VERTICAL )->add< Button >( "a0", nullptr );
	auto* b0 = tabs->add_page< Stack >( "tab.b", Axis::VERTICAL )->add< Button >( "b0", nullptr );

	tabs->set_initial_page( 1 );
	tabs->set_initial_page( 5 );		// 範囲外は無視する
	f.screen.set_root( std::move( root ), nullptr );

	EXPECT_EQ( 1u, tabs->get_current() );
	EXPECT_EQ( b0, f.screen.get_focused() );
}

TEST_F( TabTest, 操作ヒントにタブ切替が出る )
{
	strings.set( "hint.key.tab", "Q / E" );
	strings.set( "hint.tab", "タブ切替" );
	EXPECT_NE( string_t::npos, screen.build_hint_text().find( "Q / E タブ切替" ) );
}

// ---- Label ----

TEST( LabelTest, 幅を超えるとコードポイント単位で折り返す )
{
	ui_test::MockRenderer r;
	// font_size 10 → 1 文字 5。幅 20 → 4 文字ずつ
	const auto lines = Label::wrap_lines( "あいうえおかきくけ", 20.f, 10.f, r );

	ASSERT_EQ( 3u, lines.size() );
	EXPECT_EQ( "あいうえ", lines[ 0 ] );
	EXPECT_EQ( "おかきく", lines[ 1 ] );
	EXPECT_EQ( "け", lines[ 2 ] );
}

TEST( LabelTest, 改行文字で必ず改行する )
{
	ui_test::MockRenderer r;
	const auto lines = Label::wrap_lines( "一行目\n二行目", 1000.f, 10.f, r );

	ASSERT_EQ( 2u, lines.size() );
	EXPECT_EQ( "一行目", lines[ 0 ] );
	EXPECT_EQ( "二行目", lines[ 1 ] );
}

// ---- クリック判定（ClickTracker）----

namespace
{

PointerEvent pointer( PointerEvent::Type type, bool inside )
{
	return PointerEvent{ type, 0.f, 0.f, inside };
}

} // namespace

TEST( ClickTrackerTest, 押して中で離すとクリックが確定する )
{
	ClickTracker c;

	EXPECT_FALSE( c.feed( pointer( PointerEvent::Type::PRESS, true ) ) );
	EXPECT_TRUE( c.is_pressed() );
	EXPECT_FALSE( c.feed( pointer( PointerEvent::Type::DRAG, true ) ) );
	EXPECT_TRUE( c.is_pressed() );
	EXPECT_TRUE( c.feed( pointer( PointerEvent::Type::RELEASE, true ) ) );
	EXPECT_FALSE( c.is_pressed() );
}

TEST( ClickTrackerTest, 外で離すとクリックにならず押下も解ける )
{
	ClickTracker c;

	c.feed( pointer( PointerEvent::Type::PRESS, true ) );
	c.feed( pointer( PointerEvent::Type::DRAG, false ) );
	EXPECT_FALSE( c.feed( pointer( PointerEvent::Type::RELEASE, false ) ) );
	EXPECT_FALSE( c.is_pressed() );
}

TEST( ClickTrackerTest, 押していないのに離してもクリックにならない )
{
	ClickTracker c;

	EXPECT_FALSE( c.feed( pointer( PointerEvent::Type::RELEASE, true ) ) );
}

TEST( ClickTrackerTest, 取り消した後の離しはクリックにならない )
{
	ClickTracker c;

	c.feed( pointer( PointerEvent::Type::PRESS, true ) );
	c.cancel();
	EXPECT_FALSE( c.is_pressed() );
	EXPECT_FALSE( c.feed( pointer( PointerEvent::Type::RELEASE, true ) ) );
}

TEST_F( WidgetTest, 捕捉が外れたボタンは押した見た目が解け次の離しでクリックしない )
{
	int clicks = 0;
	auto* button = root->add< Button >( "button", [&clicks] { clicks++; } );
	ready();

	UIContext c { theme, strings, renderer, screen };
	const PointerEvent press { PointerEvent::Type::PRESS, button->rect().center_x(), button->rect().center_y(), true };
	const PointerEvent release { PointerEvent::Type::RELEASE, button->rect().center_x(), button->rect().center_y(), true };

	button->on_pointer( press, c );
	button->on_capture_lost();
	button->on_pointer( release, c );
	EXPECT_EQ( 0, clicks );

	button->on_pointer( press, c );
	button->on_pointer( release, c );
	EXPECT_EQ( 1, clicks );
}

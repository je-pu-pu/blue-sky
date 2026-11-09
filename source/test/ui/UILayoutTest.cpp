#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/AnchorBox.h>
#include <core/ui/Button.h>
#include <core/ui/Stack.h>
#include <core/ui/VirtualScreen.h>
#include <core/Service.h>

#include <cstdint>

using namespace core::ui;

// ---- サービスの差し替え（設計上の問題 B28） ----

TEST( ServiceTest, Scopeで差し替えたサービスは破棄で元に戻る )
{
	auto* const before = core::get_physics_manager();
	auto* const fake = reinterpret_cast< core::physics::PhysicsManager* >( static_cast< std::uintptr_t >( 0x1000 ) );		// 参照はしない

	{
		core::Service::Scope scope;
		scope.service().set_physics_manager( fake );
		EXPECT_EQ( fake, core::get_physics_manager() );
	}

	EXPECT_EQ( before, core::get_physics_manager() );
}

// ---- 横に並べるときの幅（設計上の問題 B3） ----

namespace
{

class WidthTest : public ::testing::Test, public ui_test::ScreenFixture
{
};

} // namespace

TEST_F( WidthTest, 横のStackは決まった幅と内容の幅を先に取り残りを重みで分ける )
{
	auto row = std::make_unique< Stack >( Axis::HORIZONTAL );
	row->set_spacing( 0.f );
	Stack* stack = row.get();

	auto* content = stack->add< Button >( "abcd", nullptr );		// MockRenderer : 文字幅 = 文字数 × font_size × 0.5
	auto* fill1 = stack->add< Button >( "x", nullptr );
	auto* fill2 = stack->add< Button >( "y", nullptr );
	auto* fixed = stack->add< Button >( "z", nullptr );

	stack->set_size( content, { Stack::SizeMode::CONTENT } );
	stack->set_size( fill2, { Stack::SizeMode::FILL, 3.f } );
	stack->set_size( fixed, { Stack::SizeMode::FIXED, 100.f } );

	screen.set_root( std::move( row ), [] { } );
	step();

	const float_t content_w = 4.f * theme.font_size_normal * 0.5f + theme.row_padding_x * 2.f;
	EXPECT_FLOAT_EQ( content_w, content->rect().w );
	EXPECT_FLOAT_EQ( 100.f, fixed->rect().w );

	// 残りを 1 : 3 で分ける
	const float_t remaining = 1920.f - content_w - 100.f;
	EXPECT_FLOAT_EQ( remaining * 0.25f, fill1->rect().w );
	EXPECT_FLOAT_EQ( remaining * 0.75f, fill2->rect().w );

	// 左から隙間なく並ぶ
	EXPECT_FLOAT_EQ( content->rect().right(), fill1->rect().x );
	EXPECT_FLOAT_EQ( fill2->rect().right(), fixed->rect().x );
	EXPECT_FLOAT_EQ( 1920.f, fixed->rect().right() );
}

TEST_F( WidthTest, 縦のStackは決まった高さと内容の高さを先に取り残りを重みで分ける )
{
	auto column = std::make_unique< Stack >( Axis::VERTICAL );
	column->set_spacing( 0.f );
	Stack* stack = column.get();

	auto* header = stack->add< Button >( "header", nullptr );		// 指定なし : 内容の高さ
	auto* body1 = stack->add< Button >( "body1", nullptr );
	auto* body2 = stack->add< Button >( "body2", nullptr );
	auto* footer = stack->add< Button >( "footer", nullptr );

	stack->set_size( body1, { Stack::SizeMode::FILL, 1.f } );
	stack->set_size( body2, { Stack::SizeMode::FILL, 2.f } );
	stack->set_size( footer, { Stack::SizeMode::FIXED, 60.f } );

	screen.set_root( std::move( column ), [] { } );
	step();

	const float_t header_h = header->rect().h;
	EXPECT_GT( header_h, 0.f );
	EXPECT_FLOAT_EQ( 60.f, footer->rect().h );

	const float_t remaining = 1080.f - header_h - 60.f;
	EXPECT_FLOAT_EQ( remaining / 3.f, body1->rect().h );
	EXPECT_FLOAT_EQ( remaining * 2.f / 3.f, body2->rect().h );
	EXPECT_FLOAT_EQ( 1080.f, footer->rect().bottom() );		// 下端にぴったり
}

TEST_F( WidthTest, 縦のset_flexは以前と同じく残りの高さを1つの子に与える )
{
	auto column = std::make_unique< Stack >( Axis::VERTICAL );
	column->set_spacing( 0.f );
	auto* top = column->add< Button >( "top", nullptr );
	auto* fill = column->add< Button >( "fill", nullptr );
	auto* bottom = column->add< Button >( "bottom", nullptr );
	column->set_flex( fill );
	screen.set_root( std::move( column ), [] { } );
	step();

	EXPECT_FLOAT_EQ( 1080.f - top->rect().h - bottom->rect().h, fill->rect().h );
	EXPECT_FLOAT_EQ( 1080.f, bottom->rect().bottom() );
}

TEST_F( WidthTest, 指定しなければ以前と同じく等分する )
{
	auto row = std::make_unique< Stack >( Axis::HORIZONTAL );
	row->set_spacing( 0.f );
	auto* a = row->add< Button >( "a", nullptr );
	auto* b = row->add< Button >( "bbbbbbbb", nullptr );
	screen.set_root( std::move( row ), [] { } );
	step();

	EXPECT_FLOAT_EQ( 960.f, a->rect().w );
	EXPECT_FLOAT_EQ( 960.f, b->rect().w );
}

// ---- 仮想画面（設計上の問題 B13） ----

TEST( VirtualScreenTest, 16対9では設計の大きさのまま )
{
	const VirtualScreen v = compute_virtual_screen( 1280.f, 720.f, 1920.f, 1080.f );

	EXPECT_FLOAT_EQ( 1920.f, v.width );
	EXPECT_FLOAT_EQ( 1080.f, v.height );
	EXPECT_FLOAT_EQ( 1280.f / 1920.f, v.scale );
}

TEST( VirtualScreenTest, 横長の画面では高さを合わせて幅を伸ばし縦長では幅を合わせて高さを伸ばす )
{
	// 21:9（3440x1440）: 高さ 1080 に合わせ、幅は 2580
	const VirtualScreen wide = compute_virtual_screen( 3440.f, 1440.f, 1920.f, 1080.f );
	EXPECT_FLOAT_EQ( 1080.f, wide.height );
	EXPECT_FLOAT_EQ( 3440.f / ( 1440.f / 1080.f ), wide.width );
	EXPECT_FLOAT_EQ( 3440.f, wide.to_physical_x( wide.width ) );		// 画面の右端まで届く（余白なし）

	// 4:3（1024x768）: 幅 1920 に合わせ、高さは 1440
	const VirtualScreen narrow = compute_virtual_screen( 1024.f, 768.f, 1920.f, 1080.f );
	EXPECT_FLOAT_EQ( 1920.f, narrow.width );
	EXPECT_FLOAT_EQ( 1440.f, narrow.height );
	EXPECT_FLOAT_EQ( 768.f, narrow.to_physical_y( narrow.height ) );

	// 最小化中など（0）: 設計の大きさのまま
	const VirtualScreen zero = compute_virtual_screen( 0.f, 0.f, 1920.f, 1080.f );
	EXPECT_FLOAT_EQ( 1920.f, zero.width );
	EXPECT_FLOAT_EQ( 1.f, zero.scale );
}

// ---- 位置の基準（アンカー） ----

TEST( AnchorBoxTest, 九つの基準で端や角から間隔を空けて置く )
{
	const Rect parent( 0.f, 0.f, 1000.f, 500.f );

	const Rect tl = AnchorBox::place( parent, 100.f, 50.f, Anchor::TOP_LEFT, 10.f );
	EXPECT_FLOAT_EQ( 10.f, tl.x );
	EXPECT_FLOAT_EQ( 10.f, tl.y );

	const Rect br = AnchorBox::place( parent, 100.f, 50.f, Anchor::BOTTOM_RIGHT, 10.f );
	EXPECT_FLOAT_EQ( 1000.f - 10.f - 100.f, br.x );
	EXPECT_FLOAT_EQ( 500.f - 10.f - 50.f, br.y );

	const Rect c = AnchorBox::place( parent, 100.f, 50.f, Anchor::CENTER, 10.f );
	EXPECT_FLOAT_EQ( 450.f, c.x );
	EXPECT_FLOAT_EQ( 225.f, c.y );

	const Rect top = AnchorBox::place( parent, 100.f, 50.f, Anchor::TOP, 10.f );
	EXPECT_FLOAT_EQ( 450.f, top.x );
	EXPECT_FLOAT_EQ( 10.f, top.y );

	// 親より大きければ縮めて収める
	const Rect big = AnchorBox::place( parent, 2000.f, 50.f, Anchor::RIGHT, 10.f );
	EXPECT_FLOAT_EQ( 980.f, big.w );
	EXPECT_FLOAT_EQ( 10.f, big.x );
}

namespace
{

class LayoutTest : public ::testing::Test, public ui_test::ScreenFixture
{
};

} // namespace

TEST_F( LayoutTest, セーフエリアの内側に層を置き右下に寄せた部品は端から空く )
{
	theme.safe_area = 0.05f;		// 各辺 5%

	auto box = std::make_unique< AnchorBox >( Anchor::BOTTOM_RIGHT );
	box->set_width( 200.f );
	auto* button = box->add< Button >( "hud", nullptr );
	screen.set_root( std::move( box ), [] { } );
	step();

	const Rect safe = screen.get_safe_rect();
	EXPECT_FLOAT_EQ( 1920.f * 0.05f, safe.x );
	EXPECT_FLOAT_EQ( 1080.f * 0.05f, safe.y );
	EXPECT_FLOAT_EQ( 1920.f * 0.9f, safe.w );

	// 右下の部品はセーフエリアの右下の角に接する
	EXPECT_FLOAT_EQ( safe.right(), button->rect().right() );
	EXPECT_FLOAT_EQ( safe.bottom(), button->rect().bottom() );
	EXPECT_FLOAT_EQ( 200.f, button->rect().w );
}

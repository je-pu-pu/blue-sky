#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/AnchorBox.h>
#include <core/ui/Button.h>
#include <core/ui/Stack.h>
#include <core/ui/VirtualScreen.h>

using namespace core::ui;

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

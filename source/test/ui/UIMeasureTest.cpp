#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/AnchorBox.h>
#include <core/ui/Button.h>
#include <core/ui/Label.h>
#include <core/ui/Stack.h>

#include <cmath>

using namespace core::ui;

// ---- 2 次元の計測（設計上の問題 B3）: measure( 使える大きさ ) → 希望の大きさ ----

namespace
{

class MeasureTest : public ::testing::Test, public ui_test::ScreenFixture
{
};

} // namespace

TEST_F( MeasureTest, 入れ子の横のStackを内容に合わせた幅にすると子の幅の合計になる )
{
	auto root = std::make_unique< Stack >( Axis::HORIZONTAL );
	auto* inner = root->add< Stack >( Axis::HORIZONTAL );
	auto* a = inner->add< Button >( "ab", nullptr );
	auto* b = inner->add< Button >( "abcdefgh", nullptr );
	inner->set_size( a, { Stack::SizeMode::CONTENT } );
	inner->set_size( b, { Stack::SizeMode::CONTENT } );
	auto* rest = root->add< Button >( "rest", nullptr );
	root->set_size( inner, { Stack::SizeMode::CONTENT } );
	screen.set_root( std::move( root ), [] { } );
	step();

	// 以前は Stack が内容に合わせた幅を持てず（0）、入れ子の Stack を CONTENT にすると幅 0 になった
	EXPECT_LT( a->rect().w, b->rect().w );
	EXPECT_FLOAT_EQ( a->rect().w + theme.spacing + b->rect().w, inner->rect().w );

	// 残りは FILL の子が使う
	EXPECT_FLOAT_EQ( inner->rect().right() + theme.spacing, rest->rect().x );
	EXPECT_GT( rest->rect().w, 0.f );
}

TEST_F( MeasureTest, 使える幅が狭ければ文字は折り返して高さが増える )
{
	auto root = std::make_unique< Stack >( Axis::VERTICAL );
	auto* label = root->add< Label >();
	label->set_text( "aaaa bbbb cccc dddd eeee ffff gggg hhhh" );
	label->set_wrap( true );
	screen.set_root( std::move( root ), [] { } );
	step();

	const UIContext ctx { theme, strings, renderer, screen };
	const Extent wide = label->measure( Extent( 10000.f, INFINITY ), ctx );
	const Extent narrow = label->measure( Extent( wide.w / 3.f, INFINITY ), ctx );

	// 広ければ 1 行（内容に合わせた幅）、狭ければ使える幅に収まり、行が増える
	EXPECT_LT( wide.w, 10000.f );
	EXPECT_LE( narrow.w, wide.w / 3.f );
	EXPECT_GT( narrow.h, wide.h * 2.f );
}

TEST_F( MeasureTest, 幅の指定が無いAnchorBoxは子の希望の大きさで置く )
{
	auto box = std::make_unique< AnchorBox >( Anchor::BOTTOM_RIGHT );
	auto* button = box->add< Button >( "ok", nullptr );
	screen.set_root( std::move( box ), [] { } );
	step();

	const Rect safe = screen.get_safe_rect();

	// 以前は使える幅いっぱいに広がった。今はボタンの内容に合わせた幅で、右下の角に置く
	EXPECT_LT( button->rect().w, safe.w * 0.5f );
	EXPECT_FLOAT_EQ( safe.right(), button->rect().right() );
	EXPECT_FLOAT_EQ( safe.bottom(), button->rect().bottom() );
}

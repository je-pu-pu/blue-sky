#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/Brush.h>
#include <core/ui/Button.h>
#include <core/ui/Image.h>
#include <core/ui/Slider.h>
#include <core/ui/Stack.h>
#include <core/ui/Theme.h>
#include <core/ui/Toggle.h>

#include <Scene/OverlayRenderRange.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace core::ui;
using ui_test::MockRenderer;
using ui_test::ScreenFixture;

namespace
{

void expect_rect( const Rect& expected, const Rect& actual )
{
	EXPECT_FLOAT_EQ( expected.x, actual.x );
	EXPECT_FLOAT_EQ( expected.y, actual.y );
	EXPECT_FLOAT_EQ( expected.w, actual.w );
	EXPECT_FLOAT_EQ( expected.h, actual.h );
}

/// 「N 行目: キー: 理由」の形のエラーに、そのキーのものがあるか
bool has_error_for( const std::vector< std::string >& errors, const std::string& key )
{
	return std::any_of( errors.begin(), errors.end(), [&] ( const std::string& e ) { return e.find( " " + key + ": " ) != std::string::npos; } );
}

} // namespace

// ---- draw_brush ----

TEST( UIBrushTest, 色だけのBrushは矩形を1つ塗る )
{
	MockRenderer r;
	draw_brush( r, Rect( 10.f, 20.f, 30.f, 40.f ), Brush( core::Color( 1.f, 0.f, 0.f, 1.f ) ) );

	ASSERT_EQ( 1u, r.rects.size() );
	expect_rect( Rect( 10.f, 20.f, 30.f, 40.f ), r.rects[ 0 ] );
	EXPECT_TRUE( r.images.empty() );
}

TEST( UIBrushTest, 既定のBrushと大きさ0の矩形は何も描かない )
{
	MockRenderer r;
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 100.f ), Brush() );
	draw_brush( r, Rect( 0.f, 0.f, 0.f, 100.f ), Brush( core::Color( 1.f, 1.f, 1.f, 1.f ) ) );

	EXPECT_TRUE( r.rects.empty() );
	EXPECT_TRUE( r.images.empty() );
	EXPECT_FALSE( Brush().is_visible() );
}

TEST( UIBrushTest, 画像のBrushは画像全体を引き伸ばして描き色を掛け合わせる )
{
	MockRenderer r;
	r.image_sizes[ "ui/a.png" ] = { 64.f, 32.f };

	const core::Color tint( 1.f, 0.5f, 0.5f, 0.8f );
	draw_brush( r, Rect( 0.f, 0.f, 200.f, 50.f ), Brush::from_image( "ui/a.png", Insets(), tint ) );

	ASSERT_EQ( 1u, r.images.size() );
	EXPECT_EQ( "ui/a.png", r.images[ 0 ].image );
	expect_rect( Rect( 0.f, 0.f, 200.f, 50.f ), r.images[ 0 ].rect );
	expect_rect( Rect( 0.f, 0.f, 64.f, 32.f ), r.images[ 0 ].source );
	EXPECT_FLOAT_EQ( 0.8f, r.images[ 0 ].tint.a() );
	EXPECT_TRUE( r.rects.empty() );
}

TEST( UIBrushTest, 切り出し範囲を指定するとその部分だけを使う )
{
	MockRenderer r;
	r.image_sizes[ "ui/atlas.png" ] = { 256.f, 256.f };

	Brush b = Brush::from_image( "ui/atlas.png" );
	b.source = Rect( 64.f, 0.f, 32.f, 16.f );
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 100.f ), b );

	ASSERT_EQ( 1u, r.images.size() );
	expect_rect( Rect( 64.f, 0.f, 32.f, 16.f ), r.images[ 0 ].source );
}

TEST( UIBrushTest, 画像が読めないときは色で代用し色も透明なら何も描かない )
{
	MockRenderer r;

	Brush b = Brush::from_image( "ui/missing.png" );
	draw_brush( r, Rect( 0.f, 0.f, 10.f, 10.f ), b );
	EXPECT_TRUE( r.rects.empty() );
	EXPECT_TRUE( r.images.empty() );

	b.color = core::Color( 0.f, 0.f, 0.f, 1.f );
	draw_brush( r, Rect( 0.f, 0.f, 10.f, 10.f ), b );
	EXPECT_EQ( 1u, r.rects.size() );
	EXPECT_TRUE( r.images.empty() );
}

TEST( UIBrushTest, 掛け合わせる色が透明なら画像は描かない )
{
	MockRenderer r;
	r.image_sizes[ "ui/a.png" ] = { 8.f, 8.f };

	draw_brush( r, Rect( 0.f, 0.f, 10.f, 10.f ), Brush::from_image( "ui/a.png", Insets(), core::Color( 1.f, 1.f, 1.f, 0.f ) ) );

	EXPECT_TRUE( r.images.empty() );
	EXPECT_TRUE( r.rects.empty() );
}

TEST( UIBrushTest, 九分割は四隅を伸ばさず9枚に分けて隙間なく並べる )
{
	MockRenderer r;
	r.image_sizes[ "ui/frame.png" ] = { 64.f, 64.f };

	draw_brush( r, Rect( 100.f, 200.f, 300.f, 120.f ), Brush::from_image( "ui/frame.png", Insets( 8.f, 4.f, 16.f, 12.f ) ) );

	ASSERT_EQ( 9u, r.images.size() );

	// 左上の隅 : 元画像の 8x4 を同じ大きさで
	expect_rect( Rect( 100.f, 200.f, 8.f, 4.f ), r.images[ 0 ].rect );
	expect_rect( Rect( 0.f, 0.f, 8.f, 4.f ), r.images[ 0 ].source );

	// 中央 : 残り全部
	expect_rect( Rect( 108.f, 204.f, 300.f - 8.f - 16.f, 120.f - 4.f - 12.f ), r.images[ 4 ].rect );
	expect_rect( Rect( 8.f, 4.f, 64.f - 8.f - 16.f, 64.f - 4.f - 12.f ), r.images[ 4 ].source );

	// 右下の隅
	expect_rect( Rect( 400.f - 16.f, 320.f - 12.f, 16.f, 12.f ), r.images[ 8 ].rect );
	expect_rect( Rect( 48.f, 52.f, 16.f, 12.f ), r.images[ 8 ].source );

	// 隣り合う片は辺を共有する（隙間・重なりなし）
	for ( int row = 0; row < 3; row++ )
	{
		for ( int col = 0; col < 2; col++ )
		{
			const Rect& a = r.images[ row * 3 + col ].rect;
			const Rect& b = r.images[ row * 3 + col + 1 ].rect;
			EXPECT_FLOAT_EQ( a.right(), b.x );
		}
	}
}

TEST( UIBrushTest, 九分割の枠幅は倍率で拡大できる )
{
	MockRenderer r;
	r.image_sizes[ "ui/frame.png" ] = { 32.f, 32.f };

	Brush b = Brush::from_image( "ui/frame.png", Insets( 8.f, 8.f, 8.f, 8.f ) );
	b.slice_scale = 2.f;
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 100.f ), b );

	ASSERT_EQ( 9u, r.images.size() );
	expect_rect( Rect( 0.f, 0.f, 16.f, 16.f ), r.images[ 0 ].rect );
	expect_rect( Rect( 0.f, 0.f, 8.f, 8.f ), r.images[ 0 ].source );
}

TEST( UIBrushTest, 描画先が枠幅より小さいときは枠を比率を保って縮め中央は描かない )
{
	MockRenderer r;
	r.image_sizes[ "ui/frame.png" ] = { 64.f, 64.f };

	// 横 : 枠 10 + 30 = 40 に対して幅 20 → 5 と 15 に縮む。縦は十分
	draw_brush( r, Rect( 0.f, 0.f, 20.f, 100.f ), Brush::from_image( "ui/frame.png", Insets( 10.f, 10.f, 30.f, 10.f ) ) );

	// 中央の列（幅 0）は描かないので 3 行 × 2 列
	ASSERT_EQ( 6u, r.images.size() );
	EXPECT_FLOAT_EQ( 5.f, r.images[ 0 ].rect.w );
	EXPECT_FLOAT_EQ( 15.f, r.images[ 1 ].rect.w );
	EXPECT_FLOAT_EQ( 20.f, r.images[ 1 ].rect.right() );
}

TEST( UIBrushTest, 九分割の枠幅が切り出し範囲より大きいときは範囲に収める )
{
	MockRenderer r;
	r.image_sizes[ "ui/frame.png" ] = { 16.f, 16.f };

	draw_brush( r, Rect( 0.f, 0.f, 100.f, 100.f ), Brush::from_image( "ui/frame.png", Insets( 16.f, 16.f, 16.f, 16.f ) ) );

	// 元画像側で 8 + 8 に縮む → 中央の片は無い → 四隅だけ
	ASSERT_EQ( 4u, r.images.size() );
	for ( const auto& c : r.images )
	{
		EXPECT_FLOAT_EQ( 8.f, c.source.w );
		EXPECT_FLOAT_EQ( 8.f, c.source.h );
		EXPECT_GE( c.source.x, 0.f );
		EXPECT_LE( c.source.right(), 16.f );
	}
}

// ---- Image ----

TEST( UIImageWidgetTest, FITは縦横比を保って中央に収める )
{
	Rect dst, src;

	// 横長の画像を正方形へ : 上下に余白
	Image::layout( Image::ScaleMode::FIT, Rect( 0.f, 0.f, 100.f, 100.f ), Rect( 0.f, 0.f, 200.f, 100.f ), dst, src );
	expect_rect( Rect( 0.f, 25.f, 100.f, 50.f ), dst );
	expect_rect( Rect( 0.f, 0.f, 200.f, 100.f ), src );

	// 縦長の画像を正方形へ : 左右に余白
	Image::layout( Image::ScaleMode::FIT, Rect( 0.f, 0.f, 100.f, 100.f ), Rect( 0.f, 0.f, 50.f, 100.f ), dst, src );
	expect_rect( Rect( 25.f, 0.f, 50.f, 100.f ), dst );
}

TEST( UIImageWidgetTest, FILLは矩形を覆い画像の中央を切り取る )
{
	Rect dst, src;

	Image::layout( Image::ScaleMode::FILL, Rect( 0.f, 0.f, 100.f, 100.f ), Rect( 0.f, 0.f, 200.f, 100.f ), dst, src );
	expect_rect( Rect( 0.f, 0.f, 100.f, 100.f ), dst );
	expect_rect( Rect( 50.f, 0.f, 100.f, 100.f ), src );

	// 切り出し範囲がずれていても、その中で中央を取る
	Image::layout( Image::ScaleMode::FILL, Rect( 0.f, 0.f, 200.f, 100.f ), Rect( 10.f, 20.f, 100.f, 100.f ), dst, src );
	expect_rect( Rect( 10.f, 45.f, 100.f, 50.f ), src );
}

TEST( UIImageWidgetTest, STRETCHはそのまま引き伸ばす )
{
	Rect dst, src;

	Image::layout( Image::ScaleMode::STRETCH, Rect( 5.f, 5.f, 100.f, 30.f ), Rect( 0.f, 0.f, 64.f, 64.f ), dst, src );
	expect_rect( Rect( 5.f, 5.f, 100.f, 30.f ), dst );
	expect_rect( Rect( 0.f, 0.f, 64.f, 64.f ), src );
}

namespace
{

class ImageWidgetTest : public ::testing::Test, public ScreenFixture
{
protected:
	Stack* root = nullptr;

	void SetUp() override
	{
		renderer.image_sizes[ "ui/logo.png" ] = { 400.f, 100.f };
		theme.focus_marker_width = 0.f;		// 画像の描き方だけを数える（フォーカスの印は別のテスト）

		auto r = std::make_unique< Stack >( Axis::VERTICAL );
		root = r.get();
		screen.set_root( std::move( r ), [] { } );
	}

	void ready()
	{
		screen.invalidate_layout();
		step();
	}
};

} // namespace

TEST_F( ImageWidgetTest, 高さ未指定なら幅と画像の縦横比で高さが決まる )
{
	auto* image = root->add< Image >( "ui/logo.png" );
	ready();

	EXPECT_FLOAT_EQ( image->rect().w / 4.f, image->rect().h );

	render();
	ASSERT_EQ( 1u, renderer.images.size() );
	expect_rect( image->rect(), renderer.images[ 0 ].rect );
}

TEST_F( ImageWidgetTest, 高さを指定すればその高さになり読めない画像は何も描かない )
{
	auto* fixed = root->add< Image >( "ui/logo.png" );
	fixed->set_height( 50.f );
	auto* missing = root->add< Image >( "ui/missing.png" );
	ready();

	EXPECT_FLOAT_EQ( 50.f, fixed->rect().h );
	EXPECT_FLOAT_EQ( 0.f, missing->rect().h );

	render();
	ASSERT_EQ( 1u, renderer.images.size() );
	EXPECT_EQ( "ui/logo.png", renderer.images[ 0 ].image );
}

TEST_F( ImageWidgetTest, 画像はフォーカスを受けない )
{
	root->add< Image >( "ui/logo.png" );
	auto* button = root->add< Button >( "ok", nullptr );
	ready();
	screen.focus_into( root );

	EXPECT_TRUE( button->is_focused() );
}

// ---- テーマの画像を部品が使う ----

TEST_F( ImageWidgetTest, テーマの面に画像を指定するとボタンが九分割で描かれる )
{
	renderer.image_sizes[ "ui/button.png" ] = { 48.f, 48.f };
	theme.row = Brush::from_image( "ui/button.png", Insets( 16.f, 16.f, 16.f, 16.f ) );
	theme.row_focused = theme.row;

	root->add< Button >( "ok", nullptr );
	ready();
	render();

	EXPECT_EQ( 9u, renderer.images.size() );
	EXPECT_TRUE( renderer.rects.empty() );
	EXPECT_TRUE( renderer.has_text( "ok" ) );
}

TEST_F( ImageWidgetTest, スライダーのつまみはテーマで指定したときだけ値の位置に描く )
{
	float value = 0.5f;
	auto* slider = root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.1f );
	ready();

	render();
	EXPECT_TRUE( renderer.images.empty() );

	renderer.image_sizes[ "ui/knob.png" ] = { 24.f, 48.f };
	theme.slider_knob = Brush::from_image( "ui/knob.png" );
	render();

	ASSERT_EQ( 1u, renderer.images.size() );
	const Rect track = slider->get_track_rect();
	EXPECT_FLOAT_EQ( track.x + track.w * 0.5f, renderer.images[ 0 ].rect.center_x() );
	EXPECT_FLOAT_EQ( track.center_y(), renderer.images[ 0 ].rect.center_y() );
	EXPECT_FLOAT_EQ( theme.slider_knob_width, renderer.images[ 0 ].rect.w );
}

TEST_F( ImageWidgetTest, トグルのオンの見た目を指定すると帯の代わりに欄全体へ描く )
{
	bool value = true;
	root->add< Toggle >( "mute", [&] { return value; }, [&] ( bool v ) { value = v; } );
	renderer.image_sizes[ "ui/check.png" ] = { 32.f, 32.f };
	theme.toggle_on = Brush::from_image( "ui/check.png" );
	ready();

	render();
	EXPECT_EQ( 1u, renderer.images.size() );

	value = false;
	render();
	EXPECT_TRUE( renderer.images.empty() );
}

// ---- 物理ピクセルへのそろえ（レビュー指摘 4） ----

TEST( UIBrushTest, 半端な倍率と原点のずれでも九分割の継ぎ目は物理ピクセルで隙間も重なりもなく角の太さは位置によらない )
{
	MockRenderer r;
	r.pixel_scale = 2.f / 3.f;		// 1920x1080 の仮想座標を 1280x720 で表示
	r.offset_x = 0.4f;				// 16:9 でない画面の中央寄せで生じる端数
	r.offset_y = 13.6f;
	r.image_sizes[ "ui/frame.png" ] = { 48.f, 48.f };

	const Brush b = Brush::from_image( "ui/frame.png", Insets( 16.f, 16.f, 16.f, 16.f ) );

	// 位置と大きさを少しずつずらしても、物理ピクセルで継ぎ目が一致し、四隅の太さは一定
	for ( int i = 0; i < 12; i++ )
	{
		r.clear();
		draw_brush( r, Rect( 100.3f + i * 0.37f, 50.1f + i * 0.29f, 300.f + i * 0.21f, 90.f + i * 0.43f ), b );
		ASSERT_EQ( 9u, r.images.size() );

		// 物理ピクセルに写した各片（本物の Renderer と同じ規則）
		core::ui::PixelRect p[ 9 ];
		for ( int k = 0; k < 9; k++ )
		{
			p[ k ] = r.to_pixel( r.images[ k ].rect );
		}

		for ( int row = 0; row < 3; row++ )
		{
			for ( int col = 0; col < 2; col++ )
			{
				EXPECT_EQ( p[ row * 3 + col ].right, p[ row * 3 + col + 1 ].left ) << "横の継ぎ目 i=" << i;
			}
		}
		for ( int col = 0; col < 3; col++ )
		{
			for ( int row = 0; row < 2; row++ )
			{
				EXPECT_EQ( p[ row * 3 + col ].bottom, p[ ( row + 1 ) * 3 + col ].top ) << "縦の継ぎ目 i=" << i;
			}
		}

		// 16 * 2/3 = 10.67 → 11 ピクセル
		for ( int k : { 0, 2, 6, 8 } )
		{
			EXPECT_EQ( 11, p[ k ].right - p[ k ].left ) << "角の幅 i=" << i << " k=" << k;
			EXPECT_EQ( 11, p[ k ].bottom - p[ k ].top ) << "角の高さ i=" << i << " k=" << k;
		}
	}
}

TEST( UIBrushTest, 半端な倍率で九分割を切り取っても角の画像は欠けない )
{
	MockRenderer r;
	r.pixel_scale = 2.f / 3.f;
	r.offset_x = 0.4f;
	r.image_sizes[ "ui/bar.png" ] = { 48.f, 16.f };

	// 横だけ枠 16 の 9 分割。切り取りは描画先と同じ範囲（スライダーの値 1.0 に相当）
	const Rect dst( 100.3f, 50.f, 300.f, 16.f );
	draw_brush( r, dst, Brush::from_image( "ui/bar.png", Insets( 16.f, 0.f, 16.f, 0.f ) ), & dst );

	ASSERT_EQ( 3u, r.images.size() );

	// 両端の角は元画像の 16 ピクセルをまるごと使う（切り取りで削られない）
	EXPECT_FLOAT_EQ( 16.f, r.images[ 0 ].source.w );
	EXPECT_FLOAT_EQ( 16.f, r.images[ 2 ].source.w );
}

// ---- 切り取り（レビュー指摘 6） ----

TEST( UIBrushTest, 切り取ると描画先と元画像の範囲を同じ比率で狭める )
{
	MockRenderer r;
	r.image_sizes[ "ui/bar.png" ] = { 100.f, 10.f };

	const Rect clip( 0.f, 0.f, 50.f, 20.f );
	draw_brush( r, Rect( 0.f, 0.f, 200.f, 20.f ), Brush::from_image( "ui/bar.png" ), & clip );

	ASSERT_EQ( 1u, r.images.size() );
	expect_rect( Rect( 0.f, 0.f, 50.f, 20.f ), r.images[ 0 ].rect );
	expect_rect( Rect( 0.f, 0.f, 25.f, 10.f ), r.images[ 0 ].source );
}

TEST( UIBrushTest, 九分割を切り取ると右端の角は描かず左の角は潰さない )
{
	MockRenderer r;
	r.image_sizes[ "ui/bar.png" ] = { 48.f, 16.f };

	// 横だけ枠 8 の 9 分割を幅 200 に描いて、左 30 だけ見せる
	const Rect clip( 0.f, 0.f, 30.f, 16.f );
	draw_brush( r, Rect( 0.f, 0.f, 200.f, 16.f ), Brush::from_image( "ui/bar.png", Insets( 8.f, 0.f, 8.f, 0.f ) ), & clip );

	ASSERT_EQ( 2u, r.images.size() );
	expect_rect( Rect( 0.f, 0.f, 8.f, 16.f ), r.images[ 0 ].rect );		// 左の角はそのまま
	expect_rect( Rect( 0.f, 0.f, 8.f, 16.f ), r.images[ 0 ].source );
	EXPECT_FLOAT_EQ( 30.f, r.images[ 1 ].rect.right() );				// 中央は切り取り位置まで
}

TEST( UIBrushTest, 単色も切り取れる )
{
	MockRenderer r;
	const Rect clip( 10.f, 0.f, 20.f, 100.f );
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 10.f ), Brush( core::Color( 1.f, 1.f, 1.f, 1.f ) ), & clip );

	ASSERT_EQ( 1u, r.rects.size() );
	expect_rect( Rect( 10.f, 0.f, 20.f, 10.f ), r.rects[ 0 ] );
}

TEST( UIBrushTest, 切り出し範囲が画像の外にはみ出すときは画像の中に収める )
{
	MockRenderer r;
	r.image_sizes[ "ui/atlas.png" ] = { 64.f, 64.f };

	Brush b = Brush::from_image( "ui/atlas.png" );
	b.source = Rect( 48.f, 48.f, 32.f, 32.f );
	draw_brush( r, Rect( 0.f, 0.f, 10.f, 10.f ), b );

	ASSERT_EQ( 1u, r.images.size() );
	expect_rect( Rect( 48.f, 48.f, 16.f, 16.f ), r.images[ 0 ].source );
}

TEST( UIBrushTest, アトラスの一部を九分割できる )
{
	MockRenderer r;
	r.image_sizes[ "ui/atlas.png" ] = { 256.f, 256.f };

	Brush b = Brush::from_image( "ui/atlas.png", Insets( 4.f, 4.f, 4.f, 4.f ) );
	b.source = Rect( 64.f, 32.f, 16.f, 16.f );
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 40.f ), b );

	ASSERT_EQ( 9u, r.images.size() );
	expect_rect( Rect( 64.f, 32.f, 4.f, 4.f ), r.images[ 0 ].source );
	expect_rect( Rect( 76.f, 44.f, 4.f, 4.f ), r.images[ 8 ].source );
}

// ---- 描けるかの判定（レビュー指摘 5） ----

TEST( UIBrushTest, 読めない画像は描けない扱いで色があれば描ける扱い )
{
	MockRenderer r;
	r.image_sizes[ "ui/a.png" ] = { 8.f, 8.f };

	EXPECT_FALSE( can_draw( r, Brush() ) );
	EXPECT_FALSE( can_draw( r, Brush::from_image( "ui/missing.png" ) ) );
	EXPECT_TRUE( can_draw( r, Brush::from_image( "ui/a.png" ) ) );

	Brush with_color = Brush::from_image( "ui/missing.png" );
	with_color.color = core::Color( 0.f, 0.f, 0.f, 1.f );
	EXPECT_TRUE( can_draw( r, with_color ) );

	const Brush missing = Brush::from_image( "ui/missing.png" );
	const Brush fallback( core::Color( 1.f, 0.f, 0.f, 1.f ) );
	EXPECT_EQ( & fallback, & pick_brush( r, missing, fallback ) );
}

TEST_F( ImageWidgetTest, トグルのオン表示の画像が読めないときは帯で代用する )
{
	bool value = true;
	root->add< Toggle >( "mute", [&] { return value; }, [&] ( bool v ) { value = v; } );
	theme.toggle_on = Brush::from_image( "ui/missing.png" );
	ready();

	render();
	EXPECT_TRUE( renderer.images.empty() );

	// 行の背景・値の欄・左端の帯（幅 8）
	ASSERT_EQ( 3u, renderer.rects.size() );
	EXPECT_FLOAT_EQ( 8.f, renderer.rects.back().w );
}

TEST_F( ImageWidgetTest, スライダーのつまみの画像が読めないときは描かない )
{
	float value = 0.5f;
	root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.1f );
	theme.slider_knob = Brush::from_image( "ui/missing.png" );
	ready();

	render();
	EXPECT_TRUE( renderer.images.empty() );
}

TEST_F( ImageWidgetTest, スライダーの充填は溝の形のまま値の位置で切り取る )
{
	float value = 0.25f;
	auto* slider = root->add< Slider >( "volume", [&] { return value; }, [&] ( float v ) { value = v; }, 0.f, 1.f, 0.05f );
	renderer.image_sizes[ "ui/fill.png" ] = { 32.f, 16.f };
	theme.slider_fill = Brush::from_image( "ui/fill.png" );
	ready();

	render();
	ASSERT_EQ( 1u, renderer.images.size() );

	const Rect track = slider->get_track_rect();
	expect_rect( Rect( track.x, track.y, track.w * 0.25f, track.h ), renderer.images[ 0 ].rect );
	EXPECT_FLOAT_EQ( 8.f, renderer.images[ 0 ].source.w );		// 元画像も 1/4
}

TEST_F( ImageWidgetTest, 用途別の面が未指定なら共通の面で代用する )
{
	auto* button = root->add< Button >( "ok", nullptr );
	button->set_enabled( false );
	renderer.image_sizes[ "ui/row.png" ] = { 16.f, 16.f };
	theme.row = Brush::from_image( "ui/row.png" );
	ready();

	render();
	ASSERT_EQ( 1u, renderer.images.size() );
	EXPECT_EQ( "ui/row.png", renderer.images[ 0 ].image );

	renderer.image_sizes[ "ui/row_disabled.png" ] = { 16.f, 16.f };
	theme.row_disabled = Brush::from_image( "ui/row_disabled.png" );
	render();
	ASSERT_EQ( 1u, renderer.images.size() );
	EXPECT_EQ( "ui/row_disabled.png", renderer.images[ 0 ].image );
}

// ---- テーマファイル（レビュー指摘 7, 8, 15） ----

TEST( UIThemeLoadTest, 面の色と画像と九分割を読み込める )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 10, load_theme( theme,
		"panel.image = ui/panel.png\n"
		"panel.slice = 24\n"
		"panel.slice_scale = 0.5\n"
		"panel.tint = 1 1 1 0.5\n"
		"panel.color = #102030\n"
		"row.image = ui/row.png\n"
		"row.slice = 1 2 3 4\n"
		"row.source = 0 64 128 32\n"
		"track = #FF000080\n"
		"slider_fill.image = ui/fill.png\n", & errors ) );
	EXPECT_TRUE( errors.empty() );

	EXPECT_EQ( "ui/panel.png", theme.panel.image );
	EXPECT_FLOAT_EQ( 24.f, theme.panel.slice.left );
	EXPECT_FLOAT_EQ( 24.f, theme.panel.slice.bottom );
	EXPECT_FLOAT_EQ( 0.5f, theme.panel.slice_scale );
	EXPECT_FLOAT_EQ( 0.5f, theme.panel.tint.a() );
	EXPECT_FLOAT_EQ( 0x10 / 255.f, theme.panel.color.r() );
	EXPECT_FLOAT_EQ( 0x30 / 255.f, theme.panel.color.b() );
	EXPECT_FLOAT_EQ( 1.f, theme.panel.color.a() );

	EXPECT_FLOAT_EQ( 1.f, theme.row.slice.left );
	EXPECT_FLOAT_EQ( 2.f, theme.row.slice.top );
	EXPECT_FLOAT_EQ( 3.f, theme.row.slice.right );
	EXPECT_FLOAT_EQ( 4.f, theme.row.slice.bottom );
	expect_rect( Rect( 0.f, 64.f, 128.f, 32.f ), theme.row.source );

	EXPECT_FALSE( theme.track.has_image() );
	EXPECT_FLOAT_EQ( 1.f, theme.track.color.r() );
	EXPECT_NEAR( 0.5f, theme.track.color.a(), 0.01f );

	EXPECT_EQ( "ui/fill.png", theme.slider_fill.image );
}

TEST( UIThemeLoadTest, 文字の色と寸法を読み込め行末のコメントは無視する )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 3, load_theme( theme,
		"# 先頭のコメント\n"
		"text = 0.5 0.25 1          # 文字の色\n"
		"row_height = 80            # 行の高さ\n"
		"header = #00FF00\n", & errors ) );
	EXPECT_TRUE( errors.empty() );

	EXPECT_FLOAT_EQ( 0.25f, theme.text.g() );
	EXPECT_FLOAT_EQ( 1.f, theme.text.a() );
	EXPECT_FLOAT_EQ( 80.f, theme.row_height );
	EXPECT_FLOAT_EQ( 1.f, theme.header.g() );
}

TEST( UIThemeLoadTest, 画像を空にすると単色に戻る )
{
	Theme theme;
	theme.panel = Brush::from_image( "ui/panel.png" );
	EXPECT_EQ( 1, load_theme( theme, "panel.image =\n" ) );
	EXPECT_FALSE( theme.panel.has_image() );
}

TEST( UIThemeLoadTest, 画像のパスは逆スラッシュを区切りに直しエスケープとして扱わない )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 1, load_theme( theme, "panel.image = ui\\new\\panel.png\n", & errors ) );
	EXPECT_TRUE( errors.empty() );
	EXPECT_EQ( "ui/new/panel.png", theme.panel.image );
}

TEST( UIThemeLoadTest, 解釈できない行は行番号と理由を報告して読み飛ばしほかの行は反映する )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 1, load_theme( theme,
		"unknown_key = 1\n"
		"panel.unknown = 1\n"
		"panel.slice = 1 2\n"
		"row.slice = -1\n"
		"text = #12345\n"
		"row_height = abc\n"
		"spacing = 12\n"
		"no_equal_sign\n", & errors ) );
	EXPECT_FLOAT_EQ( 12.f, theme.spacing );

	EXPECT_EQ( 7u, errors.size() );
	EXPECT_TRUE( has_error_for( errors, "unknown_key" ) );
	EXPECT_TRUE( has_error_for( errors, "panel.unknown" ) );
	EXPECT_TRUE( has_error_for( errors, "panel.slice" ) );
	EXPECT_TRUE( has_error_for( errors, "row.slice" ) );
	EXPECT_TRUE( has_error_for( errors, "text" ) );
	EXPECT_TRUE( has_error_for( errors, "row_height" ) );
	EXPECT_EQ( 0u, errors[ 0 ].rfind( "1 行目: ", 0 ) );
	EXPECT_EQ( 0u, errors.back().rfind( "8 行目: ", 0 ) );

	// 失敗した項目は既定値のまま
	EXPECT_FLOAT_EQ( Theme().row_height, theme.row_height );
}

TEST( UIThemeLoadTest, 有限でない数値と範囲外の値は受け付けない )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 0, load_theme( theme,
		"panel.slice = nan\n"
		"panel.slice_scale = inf\n"
		"row_height = 0\n"
		"font_size_normal = -10\n"
		"label_ratio = 1.5\n"
		"text = 2 0 0\n"
		"padding = -1\n", & errors ) );

	EXPECT_EQ( 7u, errors.size() );
	EXPECT_TRUE( theme.panel.slice.is_zero() );
	EXPECT_FLOAT_EQ( 1.f, theme.panel.slice_scale );
	EXPECT_FLOAT_EQ( Theme().row_height, theme.row_height );
	EXPECT_FLOAT_EQ( Theme().label_ratio, theme.label_ratio );
}

TEST( UIThemeLoadTest, 同じキーが重複すると後の行で上書きし警告する )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 2, load_theme( theme, "spacing = 4\nspacing = 6\n", & errors ) );
	EXPECT_FLOAT_EQ( 6.f, theme.spacing );
	ASSERT_EQ( 1u, errors.size() );
	EXPECT_NE( std::string::npos, errors[ 0 ].find( "1 行目と重複" ) );
}

namespace
{

/// 画像の問い合わせを記録するレンダラ
class AskRecordingRenderer : public MockRenderer
{
public:
	mutable std::vector< std::string > asked;

	bool get_image_size( const std::string& image, float& w, float& h ) const override
	{
		asked.push_back( image );
		return MockRenderer::get_image_size( image, w, h );
	}
};

} // namespace

TEST( UIThemeLoadTest, 先読みはテーマの画像をすべて問い合わせる )
{
	Theme theme;
	theme.panel = Brush::from_image( "ui/panel.png" );
	theme.slider_knob = Brush::from_image( "ui/knob.png" );

	AskRecordingRenderer r;
	preload_theme_images( theme, r );

	EXPECT_EQ( 2u, r.asked.size() );
}

// ---- 九分割の辺・中央を並べて埋める ----

TEST( UIBrushTest, 辺をroundにすると模様を整数枚に合わせて並べ引き伸ばさない )
{
	MockRenderer r;
	r.image_sizes[ "ui/chain.png" ] = { 48.f, 48.f };		// 枠 16、辺の模様 1 枚は 16x16

	Brush b = Brush::from_image( "ui/chain.png", Insets( 16.f, 16.f, 16.f, 16.f ) );
	b.edge = FillMode::ROUND;

	// 横 : 辺の長さ 100 → 16 の模様が 6.25 枚 → 6 枚（1 枚 16.67）。縦 : 辺の長さ 32 → 2 枚
	draw_brush( r, Rect( 0.f, 0.f, 132.f, 64.f ), b );

	// 四隅 4 + 上下の辺 6×2 + 左右の辺 2×2 + 中央 1
	ASSERT_EQ( 21u, r.images.size() );

	// 上の辺（先頭の角の次から 6 枚）: 同じ幅で隙間なく並び、どれも元画像の辺全体を使う
	for ( int i = 1; i <= 6; i++ )
	{
		const auto& c = r.images[ i ];
		EXPECT_NEAR( 100.f / 6.f, c.rect.w, 1e-3f );
		EXPECT_FLOAT_EQ( 16.f, c.rect.h );
		expect_rect( Rect( 16.f, 0.f, 16.f, 16.f ), c.source );

		if ( i > 1 )
		{
			EXPECT_FLOAT_EQ( r.images[ i - 1 ].rect.right(), c.rect.x );
		}
	}
	EXPECT_FLOAT_EQ( 116.f, r.images[ 6 ].rect.right() );
}

TEST( UIBrushTest, 辺をrepeatにすると等倍で並べ両端を同じだけ欠けさせる )
{
	MockRenderer r;
	r.image_sizes[ "ui/chain.png" ] = { 48.f, 48.f };

	Brush b = Brush::from_image( "ui/chain.png", Insets( 16.f, 16.f, 16.f, 16.f ) );
	b.edge = FillMode::REPEAT;

	// 高さ 32 は枠だけで埋まる（左右の辺と中央は無い）。上の辺の長さ 40 → 16 の模様が 2.5 枚 → 両端 4 ずつ欠ける
	draw_brush( r, Rect( 0.f, 0.f, 72.f, 32.f ), b );

	// 四隅 4 + 上下の辺 3×2
	ASSERT_EQ( 10u, r.images.size() );

	expect_rect( Rect( 16.f, 0.f, 12.f, 16.f ), r.images[ 1 ].rect );	// 左端の欠けた 1 枚 : 模様の右 12
	expect_rect( Rect( 20.f, 0.f, 12.f, 16.f ), r.images[ 1 ].source );
	expect_rect( Rect( 28.f, 0.f, 16.f, 16.f ), r.images[ 2 ].rect );	// 真ん中は等倍
	expect_rect( Rect( 16.f, 0.f, 16.f, 16.f ), r.images[ 2 ].source );
	expect_rect( Rect( 44.f, 0.f, 12.f, 16.f ), r.images[ 3 ].rect );	// 右端の欠けた 1 枚 : 模様の左 12
	expect_rect( Rect( 16.f, 0.f, 12.f, 16.f ), r.images[ 3 ].source );
}

TEST( UIBrushTest, 中央をrepeatにすると縦横に並べ模様の大きさは枠と同じ倍率になる )
{
	MockRenderer r;
	r.image_sizes[ "ui/tile.png" ] = { 24.f, 24.f };		// 枠 4、中央の模様 16x16

	Brush b = Brush::from_image( "ui/tile.png", Insets( 4.f, 4.f, 4.f, 4.f ) );
	b.slice_scale = 2.f;		// 枠 8、模様は 32x32
	b.center = FillMode::REPEAT;

	// 中央 64x32 → 横 2 枚 × 縦 1 枚（ちょうど）
	draw_brush( r, Rect( 0.f, 0.f, 80.f, 48.f ), b );

	// 四隅 4 + 辺 4（引き伸ばし）+ 中央 2
	ASSERT_EQ( 10u, r.images.size() );

	const auto& c0 = r.images[ 4 ];		// 2 行目（中段）の左の辺の次
	expect_rect( Rect( 8.f, 8.f, 32.f, 32.f ), c0.rect );
	expect_rect( Rect( 4.f, 4.f, 16.f, 16.f ), c0.source );
	expect_rect( Rect( 40.f, 8.f, 32.f, 32.f ), r.images[ 5 ].rect );
}

TEST( UIBrushTest, 並べる枚数が多すぎるときは引き伸ばしにする )
{
	MockRenderer r;
	r.image_sizes[ "ui/dot.png" ] = { 3.f, 3.f };		// 枠 1、辺の模様 1 ピクセル

	Brush b = Brush::from_image( "ui/dot.png", Insets( 1.f, 1.f, 1.f, 1.f ) );
	b.edge = FillMode::REPEAT;

	draw_brush( r, Rect( 0.f, 0.f, 1000.f, 2.f ), b );

	// 上下の辺は 998 枚になるので引き伸ばし 1 枚ずつ : 四隅 4 + 上下の辺 2
	EXPECT_EQ( 6u, r.images.size() );
}

TEST( UIThemeLoadTest, 辺と中央の埋め方を読み込める )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 3, load_theme( theme, "panel.edge = round\npanel.center = repeat\nrow.edge = tile\nrow.center = none\ntrack.edge = none\n", & errors ) );
	EXPECT_EQ( FillMode::ROUND, theme.panel.edge );
	EXPECT_EQ( FillMode::REPEAT, theme.panel.center );
	EXPECT_EQ( FillMode::STRETCH, theme.row.edge );
	EXPECT_EQ( FillMode::NONE, theme.row.center );
	EXPECT_EQ( FillMode::STRETCH, theme.track.edge );		// 辺を描かない指定は受け付けない（枠が途切れる）
	ASSERT_EQ( 2u, errors.size() );
	EXPECT_TRUE( has_error_for( errors, "row.edge" ) );
	EXPECT_TRUE( has_error_for( errors, "track.edge" ) );
}

TEST( UIBrushTest, 中央をnoneにすると枠だけを描く )
{
	MockRenderer r;
	r.image_sizes[ "ui/frame.png" ] = { 48.f, 48.f };

	Brush b = Brush::from_image( "ui/frame.png", Insets( 16.f, 16.f, 16.f, 16.f ) );
	b.center = FillMode::NONE;
	draw_brush( r, Rect( 0.f, 0.f, 100.f, 100.f ), b );

	ASSERT_EQ( 8u, r.images.size() );
	for ( const auto& c : r.images )
	{
		EXPECT_FALSE( c.rect.x == 16.f && c.rect.y == 16.f ) << "中央が描かれている";
	}
}

TEST( UIThemeLoadTest, 色の省略形と色の指定は同じ項目として重複を警告し属性が空のキーは受け付けない )
{
	Theme theme;
	std::vector< std::string > errors;
	EXPECT_EQ( 2, load_theme( theme, "panel = #FF0000\npanel.color = #00FF00\nrow. = #0000FF\n", & errors ) );
	EXPECT_FLOAT_EQ( 1.f, theme.panel.color.g() );

	ASSERT_EQ( 2u, errors.size() );
	EXPECT_NE( std::string::npos, errors[ 0 ].find( "1 行目と重複" ) );
	EXPECT_TRUE( has_error_for( errors, "row." ) );
}

TEST( UIThemeLoadTest, 日本語を含むパスのテーマファイルを開ける )
{
	const std::filesystem::path dir = std::filesystem::temp_directory_path() / u8"blue-sky テーマ試験";
	std::filesystem::create_directories( dir );
	const std::filesystem::path file = dir / u8"テーマ.txt";

	{
		std::ofstream out( file, std::ios::binary );
		out << "spacing = 7\n";
	}

	const std::u8string utf8 = file.u8string();

	Theme theme;
	EXPECT_TRUE( load_theme_file( theme, reinterpret_cast< const char* >( utf8.c_str() ) ) );
	EXPECT_FLOAT_EQ( 7.f, theme.spacing );

	std::filesystem::remove_all( dir );
}

TEST_F( ImageWidgetTest, 画像の切り出し範囲が画像の外にはみ出すときは画像の中に収める )
{
	auto* image = root->add< Image >( "ui/logo.png" );		// 400x100
	image->set_source( Rect( 300.f, 0.f, 200.f, 100.f ) );
	image->set_scale_mode( Image::ScaleMode::STRETCH );
	image->set_height( 50.f );
	ready();

	render();
	ASSERT_EQ( 1u, renderer.images.size() );
	expect_rect( Rect( 300.f, 0.f, 100.f, 100.f ), renderer.images[ 0 ].source );
}

// ---- オーバーレイの描画範囲（再レビュー指摘 2, 3） ----

TEST( OverlayRenderRangeTest, オーバーレイが無ければベースだけを描く )
{
	const auto r = blue_sky::compute_overlay_render_range( {} );
	EXPECT_TRUE( r.render_base );
	EXPECT_EQ( 0u, r.first_overlay );
}

TEST( OverlayRenderRangeTest, メニューの上にメニューを重ねると下のメニューは描かずベースは描く )
{
	// [ ポーズ, オプション ]（どちらも透過・下のオーバーレイを隠す）
	const auto r = blue_sky::compute_overlay_render_range( { { true, true }, { true, true } } );
	EXPECT_TRUE( r.render_base );
	EXPECT_EQ( 1u, r.first_overlay );
}

TEST( OverlayRenderRangeTest, 下のオーバーレイを隠さない透過のものはすべて描く )
{
	const auto r = blue_sky::compute_overlay_render_range( { { true, false }, { true, false } } );
	EXPECT_TRUE( r.render_base );
	EXPECT_EQ( 0u, r.first_overlay );
}

TEST( OverlayRenderRangeTest, 不透明なオーバーレイより下はベースも描かない )
{
	const auto r = blue_sky::compute_overlay_render_range( { { true, false }, { false, false }, { true, false } } );
	EXPECT_FALSE( r.render_base );
	EXPECT_EQ( 1u, r.first_overlay );
}

TEST( OverlayRenderRangeTest, 隠された不透明なオーバーレイがあってもベースは描かない )
{
	// [ 不透明な全画面, UI メニュー ] : 描くのはメニューだけ。ベースは不透明なものに覆われている
	const auto r = blue_sky::compute_overlay_render_range( { { false, false }, { true, true } } );
	EXPECT_FALSE( r.render_base );
	EXPECT_EQ( 1u, r.first_overlay );
}


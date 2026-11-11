#include "pch.h"

#include <blue_sky/ui/Renderer.h>
#include <core/graphics/TextStyle.h>

#include "../graphics/FakeGraphics.h"

/**
 * UI の描画（blue_sky::ui::Renderer）のテスト
 *
 * 以前は GraphicsManager の全体を受け取っていたので、テストでは代わりの MockRenderer（core::ui::Renderer の偽物）しか使えず、
 * 本物の Renderer の写し方・文字と Sprite の切り替え・画像の読み込みは画面で確かめるしかなかった。
 * 今は 2D を描く窓口（Canvas2D）を受け取るので、偽物で確かめる（設計上の問題 D3）
 */

using blue_sky::ui::Renderer;
using test_graphics::FakeCanvas2D;
using Rect = core::ui::Rect;

namespace
{

core::graphics::TextStyle style_of( float_t font_size, float_t outline_width = 0.f )
{
	core::graphics::TextStyle s;
	s.font_size = font_size;
	s.outline_width = outline_width;
	return s;
}

void set_screen( FakeCanvas2D& canvas, int w, int h )
{
	canvas.screen_width = w;
	canvas.screen_height = h;
	canvas.render_target_width = static_cast< float_t >( w );
	canvas.render_target_height = static_cast< float_t >( h );
}

} // namespace

TEST( UIRendererTest, 設計の解像度の画面では仮想座標のまま既定の大きさで文字を積む )
{
	FakeCanvas2D canvas;
	Renderer renderer( & canvas );

	renderer.draw_text( Rect( 100.f, 200.f, 300.f, 50.f ), "abc", style_of( 0.f ) );

	const std::vector< std::string > expected = { "queue_text abc 100,200,400,250 大きさ 64 縁 0" };
	EXPECT_EQ( canvas.calls, expected );
}

TEST( UIRendererTest, 半分の画面では矩形と文字の大きさと縁取りも半分 )
{
	FakeCanvas2D canvas;
	set_screen( canvas, 960, 540 );
	Renderer renderer( & canvas );

	EXPECT_FLOAT_EQ( renderer.get_screen_width(), 1920.f );		// 仮想座標の大きさは変わらない
	EXPECT_FLOAT_EQ( renderer.get_pixel_scale(), 0.5f );

	renderer.draw_text( Rect( 100.f, 200.f, 300.f, 50.f ), "abc", style_of( 40.f, 4.f ) );

	const std::vector< std::string > expected = { "queue_text abc 50,100,200,125 大きさ 20 縁 2" };
	EXPECT_EQ( canvas.calls, expected );
}

TEST( UIRendererTest, 文字とSpriteを切り替えるとき先に積んだものを描く )
{
	FakeCanvas2D canvas;
	Renderer renderer( & canvas );

	renderer.draw_text( Rect( 0.f, 0.f, 100.f, 20.f ), "a", style_of( 10.f ) );
	renderer.draw_rect( Rect( 0.f, 0.f, 10.f, 10.f ), core::Color::White );
	renderer.draw_rect( Rect( 10.f, 0.f, 10.f, 10.f ), core::Color::White );	// Sprite が続く間は begin しない
	renderer.draw_text( Rect( 0.f, 0.f, 100.f, 20.f ), "b", style_of( 10.f ) );
	renderer.flush();

	const std::vector< std::string > expected = {
		"queue_text a 0,0,100,20 大きさ 10 縁 0",
		"flush_text",						// 後から描く四角が文字の上に重なるように、先に文字を描く
		"sprite.begin",
		"sprite.draw_color 0,0,10,10",
		"sprite.draw_color 10,0,20,10",
		"sprite.end",						// 文字の前に Sprite を閉じる
		"queue_text b 0,0,100,20 大きさ 10 縁 0",
		"flush_text",						// flush で積んだ文字を描く
	};
	EXPECT_EQ( canvas.calls, expected );
}

TEST( UIRendererTest, 画像は名前を正規化して初回だけ読み乗算済みで描く )
{
	FakeCanvas2D canvas;
	canvas.images.insert( "ui:ui/a.png" );
	Renderer renderer( & canvas );

	float_t w = 0.f, h = 0.f;
	ASSERT_TRUE( renderer.get_image_size( ".\\ui\\a.png", w, h ) );
	EXPECT_FLOAT_EQ( w, 200.f );
	EXPECT_FLOAT_EQ( h, 100.f );

	renderer.draw_image( Rect( 0.f, 0.f, 100.f, 50.f ), ".\\ui\\a.png", Rect( 0.f, 0.f, 200.f, 100.f ), core::Color::White );

	const std::vector< std::string > expected = {
		"load ui:ui/a.png media/texture/ui/a.png",		// 2 回目（draw_image）は読まない
		"sprite.begin",
		"sprite.draw_uv 0,0,100,50 乗算済み",
	};
	EXPECT_EQ( canvas.calls, expected );
}

TEST( UIRendererTest, 読めない画像は覚えておき読み直さず描かない )
{
	FakeCanvas2D canvas;
	Renderer renderer( & canvas );

	float_t w = 0.f, h = 0.f;
	EXPECT_FALSE( renderer.get_image_size( "missing.png", w, h ) );		// 例外は Renderer が受け止める
	EXPECT_FALSE( renderer.get_image_size( "missing.png", w, h ) );
	renderer.draw_image( Rect( 0.f, 0.f, 100.f, 50.f ), "missing.png", Rect( 0.f, 0.f, 1.f, 1.f ), core::Color::White );

	const std::vector< std::string > expected = { "load ui:missing.png media/texture/missing.png" };
	EXPECT_EQ( canvas.calls, expected );
}

TEST( UIRendererTest, 文字の幅と行の高さは画面の倍率で測って仮想座標に戻す )
{
	FakeCanvas2D canvas;
	set_screen( canvas, 960, 540 );
	Renderer renderer( & canvas );

	// 偽物の幅は 文字数 × 大きさ × 0.5。大きさ 40 は画面では 20 で測る → 4 × 20 × 0.5 = 40 ピクセル → 仮想座標で 80
	EXPECT_FLOAT_EQ( renderer.measure_text_width( "abcd", 40.f ), 80.f );
	EXPECT_EQ( canvas.calls.back(), "measure_text_width 20" );

	// 大きさ 0 は既定の大きさ（64）
	EXPECT_FLOAT_EQ( renderer.measure_text_width( "abcd", 0.f ), 128.f );
	EXPECT_EQ( canvas.calls.back(), "measure_text_width 32" );

	EXPECT_FLOAT_EQ( renderer.measure_line_height( 40.f ), 50.f );		// 20 × 1.25 = 25 ピクセル → 50
}

TEST( UIRendererTest, 画面を塗るときは描画先の全体を塗る )
{
	FakeCanvas2D canvas;
	canvas.render_target_width = 800.f;
	canvas.render_target_height = 600.f;
	Renderer renderer( & canvas );

	renderer.fill_screen( core::Color::Black );

	const std::vector< std::string > expected = { "sprite.begin", "sprite.draw_color 0,0,800,600" };
	EXPECT_EQ( canvas.calls, expected );
}

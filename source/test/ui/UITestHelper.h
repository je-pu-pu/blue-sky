#pragma once

#include <core/ui/PixelSnap.h>
#include <core/ui/Renderer.h>
#include <core/ui/Screen.h>
#include <core/ui/StringTable.h>
#include <core/ui/Theme.h>
#include <core/ui/UIInput.h>

#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ui_test
{

/**
 * 描画を記録するだけのレンダラ（GPU 不要）
 *
 * テキスト幅は「UTF-8 のコードポイント数 × font_size × 0.5」、行の高さは font_size × 1.25 とする。
 */
class MockRenderer : public core::ui::Renderer
{
public:
	struct TextCall
	{
		core::ui::Rect rect;
		std::string text;
	};

	struct ImageCall
	{
		core::ui::Rect rect;
		std::string image;
		core::ui::Rect source;
		core::Color tint;
	};

	std::vector< TextCall > texts;
	std::vector< core::ui::Rect > rects;
	std::vector< ImageCall > images;
	int fill_count = 0;

	/// 読める画像とその大きさ（ここに無い画像は「読めない」扱い）
	std::map< std::string, std::pair< float, float > > image_sizes;

	/// 仮想座標 1 あたりの物理ピクセルと、物理画面での原点のずれ（本物の Renderer の等方スケール・中央寄せに相当）
	float pixel_scale = 1.f;
	float offset_x = 0.f;
	float offset_y = 0.f;

	float get_pixel_scale() const override { return pixel_scale; }

	core::ui::Rect snap_to_pixels( const core::ui::Rect& r ) const override
	{
		return core::ui::snap_rect_to_pixels( r, pixel_scale, offset_x, offset_y );
	}

	/// 本物の Renderer と同じ規則で物理ピクセルに写す
	core::ui::PixelRect to_pixel( const core::ui::Rect& r ) const
	{
		return core::ui::to_pixel_rect( r, pixel_scale, offset_x, offset_y );
	}

	static size_t count_codepoints( const char* s )
	{
		size_t n = 0;
		for ( ; *s; s++ ) if ( ( static_cast< unsigned char >( *s ) & 0xC0 ) != 0x80 ) n++;
		return n;
	}

	float get_screen_width() const override { return 1920.f; }
	float get_screen_height() const override { return 1080.f; }

	void draw_text( const core::ui::Rect& r, const char* text, const core::graphics::TextStyle& ) override { texts.push_back( { r, text } ); }
	void draw_rect( const core::ui::Rect& r, const core::Color& ) override { rects.push_back( r ); }
	void fill_screen( const core::Color& ) override { fill_count++; }

	bool get_image_size( const std::string& image, float& w, float& h ) const override
	{
		auto i = image_sizes.find( image );
		if ( i == image_sizes.end() ) return false;
		w = i->second.first;
		h = i->second.second;
		return true;
	}

	void draw_image( const core::ui::Rect& r, const std::string& image, const core::ui::Rect& source, const core::Color& tint ) override
	{
		images.push_back( { r, image, source, tint } );
	}

	float measure_text_width( const char* text, float font_size ) const override
	{
		return static_cast< float >( count_codepoints( text ) ) * font_size * 0.5f;
	}

	float measure_line_height( float font_size ) const override { return font_size * 1.25f; }

	void clear() { texts.clear(); rects.clear(); images.clear(); fill_count = 0; }

	bool has_text( const std::string& t ) const
	{
		for ( const auto& c : texts ) if ( c.text == t ) return true;
		return false;
	}
};

/**
 * Screen を 1 つ持つテスト用の土台
 */
class ScreenFixture
{
public:
	core::ui::Theme theme;
	core::ui::StringTable strings;
	MockRenderer renderer;
	core::ui::Screen screen;

	static constexpr float DT = 1.f / 60.f;

	ScreenFixture() : screen( theme, strings, renderer ) { }

	/// 1 フレーム進める
	void step( const core::ui::UIInput& in = core::ui::UIInput(), float dt = DT ) { screen.update( in, dt ); }

	void press_nav( core::ui::NavDir d )
	{
		core::ui::UIInput in;
		in.up = d == core::ui::NavDir::UP;
		in.down = d == core::ui::NavDir::DOWN;
		in.left = d == core::ui::NavDir::LEFT;
		in.right = d == core::ui::NavDir::RIGHT;
		step( in );
	}

	void confirm() { core::ui::UIInput in; in.confirm = true; step( in ); }
	void cancel() { core::ui::UIInput in; in.cancel = true; step( in ); }

	/// ポインタを動かす
	void move_pointer( float x, float y ) { core::ui::UIInput in; in.pointer_x = x; in.pointer_y = y; in.pointer_moved = true; step( in ); }

	/// その位置でクリック（押す → 離す の 2 フレーム）
	void click( float x, float y ) { click( x, y, x, y ); }

	void click( float x, float y, float release_x, float release_y )
	{
		core::ui::UIInput p;
		p.pointer_x = x; p.pointer_y = y; p.pointer_pressed = true; p.pointer_down = true;
		step( p );

		core::ui::UIInput r;
		r.pointer_x = release_x; r.pointer_y = release_y; r.pointer_released = true;
		step( r );
	}

	void render() { renderer.clear(); screen.render( renderer ); }
};

} // namespace ui_test

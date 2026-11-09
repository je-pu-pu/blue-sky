#include "Renderer.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/graphics/Sprite.h>

#include <win/Rect.h>

#include <algorithm>
#include <cmath>

namespace blue_sky::ui
{

Renderer::Renderer( GraphicsManager* gm )
	: gm_( gm )
{
}

float_t Renderer::get_scale() const
{
	const float_t sx = static_cast< float_t >( gm_->get_screen_width() ) / DESIGN_WIDTH;
	const float_t sy = static_cast< float_t >( gm_->get_screen_height() ) / DESIGN_HEIGHT;
	return std::min( sx, sy );
}

float_t Renderer::get_offset_x() const
{
	return ( static_cast< float_t >( gm_->get_screen_width() ) - DESIGN_WIDTH * get_scale() ) * 0.5f;
}

float_t Renderer::get_offset_y() const
{
	return ( static_cast< float_t >( gm_->get_screen_height() ) - DESIGN_HEIGHT * get_scale() ) * 0.5f;
}

Renderer::Rect Renderer::to_physical( const Rect& r ) const
{
	const float_t s = get_scale();
	return Rect( get_offset_x() + r.x * s, get_offset_y() + r.y * s, r.w * s, r.h * s );
}

float_t Renderer::physical_to_virtual_x( float_t px ) const
{
	return ( px - get_offset_x() ) / get_scale();
}

float_t Renderer::physical_to_virtual_y( float_t py ) const
{
	return ( py - get_offset_y() ) / get_scale();
}

void Renderer::draw_text( const Rect& rect, const char_t* text, const core::graphics::TextStyle& style )
{
	if ( ! text || ! *text )
	{
		return;
	}

	const float_t s = get_scale();
	const Rect p = to_physical( rect );

	core::graphics::TextStyle scaled = style;
	scaled.outline_width *= s;
	scaled.font_size = ( style.font_size > 0.f ? style.font_size : gm_->get_default_font_size() ) * s;

	gm_->draw_text( p.x, p.y, p.right(), p.bottom(), text, scaled );
}

void Renderer::draw_rect( const Rect& rect, const Color& color )
{
	const Rect p = to_physical( rect );

	// 端数で 1 ピクセルの隙間ができないよう、左上は切り捨て・右下は切り上げ
	const LONG left = static_cast< LONG >( std::floor( p.x ) );
	const LONG top = static_cast< LONG >( std::floor( p.y ) );
	const LONG right = static_cast< LONG >( std::ceil( p.right() ) );
	const LONG bottom = static_cast< LONG >( std::ceil( p.bottom() ) );

	if ( right <= left || bottom <= top )
	{
		return;
	}

	auto* sprite = gm_->get_sprite();

	sprite->begin();
	sprite->draw_color( win::Rect( left, top, right, bottom ), color );
	sprite->end();
}

void Renderer::fill_screen( const Color& color )
{
	auto* sprite = gm_->get_sprite();

	sprite->begin();
	sprite->draw_color( win::Rect( 0, 0, gm_->get_screen_width(), gm_->get_screen_height() ), color );
	sprite->end();
}

float_t Renderer::measure_text_width( const char_t* text, float_t font_size ) const
{
	// 物理ピクセルで測って仮想座標に戻す（フォントのヒンティング差を含めて実際の描画と一致させる）
	const float_t s = get_scale();

	if ( s <= 0.f )
	{
		return 0.f;
	}

	const float_t size = ( font_size > 0.f ? font_size : gm_->get_default_font_size() ) * s;
	return gm_->measure_text_width( text, size ) / s;
}

float_t Renderer::measure_line_height( float_t font_size ) const
{
	const float_t s = get_scale();

	if ( s <= 0.f )
	{
		return 0.f;
	}

	const float_t size = ( font_size > 0.f ? font_size : gm_->get_default_font_size() ) * s;
	return gm_->measure_text_line_height( size ) / s;
}

} // namespace blue_sky::ui

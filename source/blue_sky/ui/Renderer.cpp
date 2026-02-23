#include "Renderer.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/graphics/Sprite.h>

#include <win/Rect.h>

namespace blue_sky::ui
{

Renderer::Renderer( GraphicsManager* gm )
	: gm_( gm )
{
}

float_t Renderer::get_scale_x() const
{
	return static_cast< float_t >( gm_->get_screen_width() ) / DESIGN_WIDTH;
}

float_t Renderer::get_scale_y() const
{
	return static_cast< float_t >( gm_->get_screen_height() ) / DESIGN_HEIGHT;
}

int Renderer::get_physical_width() const
{
	return gm_->get_screen_width();
}

int Renderer::get_physical_height() const
{
	return gm_->get_screen_height();
}

float_t Renderer::physical_to_virtual_x( float_t px ) const
{
	return px / get_scale_x();
}

float_t Renderer::physical_to_virtual_y( float_t py ) const
{
	return py / get_scale_y();
}

void Renderer::draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	float_t px = x * sx;
	float_t py = y * sy;

	gm_->draw_text( px, py, px + w * sx, py + h * sy, text, color );
}

void Renderer::draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const core::graphics::TextStyle& style )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	float_t px = x * sx;
	float_t py = y * sy;

	core::graphics::TextStyle scaled = style;
	scaled.outline_width *= sy;
	if ( scaled.font_size > 0.f )
	{
		scaled.font_size *= sy;
	}

	gm_->draw_text( px, py, px + w * sx, py + h * sy, text, scaled );
}

void Renderer::draw_rect( float_t x, float_t y, float_t w, float_t h, const Color& color )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	auto* sprite = gm_->get_sprite();

	sprite->begin();
	sprite->draw_color( win::Rect::Size( static_cast< LONG >( x * sx ), static_cast< LONG >( y * sy ), static_cast< LONG >( w * sx ), static_cast< LONG >( h * sy ) ), color );
	sprite->end();
}

Renderer::Sprite* Renderer::begin_sprite()
{
	auto* sprite = gm_->get_sprite();
	sprite->begin();
	return sprite;
}

void Renderer::end_sprite()
{
	gm_->get_sprite()->end();
}

} // namespace blue_sky::ui

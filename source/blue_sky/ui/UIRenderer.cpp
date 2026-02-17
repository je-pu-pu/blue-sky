#include "UIRenderer.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/graphics/Sprite.h>

#include <win/Rect.h>

namespace blue_sky::ui
{

UIRenderer::UIRenderer( GraphicsManager* gm )
	: gm_( gm )
{
}

float_t UIRenderer::get_scale_x() const
{
	return static_cast< float_t >( gm_->get_screen_width() ) / DESIGN_WIDTH;
}

float_t UIRenderer::get_scale_y() const
{
	return static_cast< float_t >( gm_->get_screen_height() ) / DESIGN_HEIGHT;
}

int UIRenderer::get_physical_width() const
{
	return gm_->get_screen_width();
}

int UIRenderer::get_physical_height() const
{
	return gm_->get_screen_height();
}

float_t UIRenderer::physical_to_virtual_x( float_t px ) const
{
	return px / get_scale_x();
}

float_t UIRenderer::physical_to_virtual_y( float_t py ) const
{
	return py / get_scale_y();
}

void UIRenderer::draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	float_t px = x * sx;
	float_t py = y * sy;

	gm_->draw_text( px, py, px + w * sx, py + h * sy, text, color );
}

void UIRenderer::draw_text_center( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	float_t px = x * sx;
	float_t py = y * sy;

	gm_->draw_text_center( px, py, px + w * sx, py + h * sy, text, color );
}

void UIRenderer::draw_rect( float_t x, float_t y, float_t w, float_t h, const Color& color )
{
	float_t sx = get_scale_x();
	float_t sy = get_scale_y();

	auto* sprite = gm_->get_sprite();

	sprite->begin();
	sprite->draw_color( win::Rect::Size( static_cast< LONG >( x * sx ), static_cast< LONG >( y * sy ), static_cast< LONG >( w * sx ), static_cast< LONG >( h * sy ) ), color );
	sprite->end();
}

UIRenderer::Sprite* UIRenderer::begin_sprite()
{
	auto* sprite = gm_->get_sprite();
	sprite->begin();
	return sprite;
}

void UIRenderer::end_sprite()
{
	gm_->get_sprite()->end();
}

} // namespace blue_sky::ui

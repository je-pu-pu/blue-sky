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

int UIRenderer::get_screen_width() const
{
	return gm_->get_screen_width();
}

int UIRenderer::get_screen_height() const
{
	return gm_->get_screen_height();
}

void UIRenderer::draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color )
{
	gm_->draw_text( x, y, x + w, y + h, text, color );
}

void UIRenderer::draw_text_center( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color )
{
	gm_->draw_text_center( x, y, x + w, y + h, text, color );
}

void UIRenderer::draw_rect( float_t x, float_t y, float_t w, float_t h, const Color& color )
{
	auto* sprite = gm_->get_sprite();

	sprite->begin();
	sprite->draw_color( win::Rect::Size( static_cast< LONG >( x ), static_cast< LONG >( y ), static_cast< LONG >( w ), static_cast< LONG >( h ) ), color );
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

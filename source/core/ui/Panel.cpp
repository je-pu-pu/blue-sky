#include "Panel.h"
#include "TextFit.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

void Panel::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const auto& t = ctx.theme;
	const float_t w = std::min( width_ > 0.f ? width_ : t.panel_width, rect.w );
	const float_t inner_w = w - t.padding * 2.f;
	const float_t footer_h = show_description_ ? t.footer_height : t.footer_height * 0.5f;

	float_t h = rect.h - t.panel_margin_y * 2.f;

	if ( fit_height_ )
	{
		const float_t body_h = get_body() ? get_body()->measure_height( inner_w, ctx ) : 0.f;
		h = std::min( h, t.padding * 2.f + text_row_height( ctx.renderer, t.title_height, t.font_size_title, t.row_text_padding ) + body_h + t.padding + footer_h );
	}

	panel_rect_ = Rect( rect.x + ( rect.w - w ) * 0.5f, rect.y + ( rect.h - h ) * 0.5f, w, h );

	const Rect inner = panel_rect_.inset( t.padding );

	title_rect_ = Rect( inner.x, inner.y, inner.w, text_row_height( ctx.renderer, t.title_height, t.font_size_title, t.row_text_padding ) );
	hint_rect_ = Rect( inner.x, inner.bottom() - footer_h * ( show_description_ ? 0.5f : 1.f ), inner.w, footer_h * ( show_description_ ? 0.5f : 1.f ) );
	description_rect_ = show_description_ ? Rect( inner.x, inner.bottom() - footer_h, inner.w, footer_h * 0.5f ) : Rect();

	if ( Widget* body = get_body() )
	{
		const float_t top = title_rect_.bottom();
		const float_t bottom = inner.bottom() - footer_h - t.padding;
		body->arrange( Rect( inner.x, top, inner.w, std::max( 0.f, bottom - top ) ), ctx );
	}
}

void Panel::render( Renderer& renderer, const UIContext& ctx ) const
{
	const auto& t = ctx.theme;

	if ( dim_screen_ )
	{
		renderer.fill_screen( t.screen_dim );
	}

	draw_brush( renderer, panel_rect_, t.panel );
	draw_text_fit( renderer, title_rect_, ctx.tr( title_key_ ), make_text_style( t.text, t.font_size_title ) );

	render_children( renderer, ctx );

	if ( show_description_ )
	{
		draw_text_fit( renderer, description_rect_, ctx.screen.get_description(), make_text_style( t.text_dim, t.font_size_small ) );
	}

	draw_text_fit( renderer, hint_rect_, ctx.screen.build_hint_text(), make_text_style( t.text_dim, t.font_size_small ) );
}

} // namespace core::ui

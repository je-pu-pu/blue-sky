#include "Button.h"
#include "TextFit.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

void Button::click( UIContext& ctx )
{
	if ( ! is_enabled() )
	{
		return;
	}

	ctx.screen.play( UISoundEvent::CONFIRM );

	// コールバックの中で自分が破棄されても安全なようにコピーしてから呼ぶ
	auto f = on_click_;

	if ( f )
	{
		f();
	}
}

float_t Button::measure_width( const UIContext& ctx ) const
{
	return ctx.renderer.measure_text_width( ctx.tr( text_key_ ).c_str(), ctx.theme.font_size_normal ) + ctx.theme.row_padding_x * 2.f;
}

float_t Button::measure_height( float_t, const UIContext& ctx ) const
{
	// 文字を大きくしても行からはみ出さないよう、文字の行の高さ＋余白より小さくしない
	return std::max( ctx.theme.row_height, ctx.renderer.measure_line_height( ctx.theme.font_size_normal ) + ctx.theme.row_text_padding );
}

bool Button::on_confirm( UIContext& ctx )
{
	click( ctx );
	return true;
}

bool Button::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	switch ( e.type )
	{
		case PointerEvent::Type::PRESS:
			pressed_ = true;
			return true;

		case PointerEvent::Type::RELEASE:
		{
			const bool was_pressed = pressed_;
			pressed_ = false;

			if ( was_pressed && e.inside )
			{
				click( ctx );
			}

			return true;
		}

		default:
			return true;
	}
}

void Button::render( Renderer& renderer, const UIContext& ctx ) const
{
	const auto& t = ctx.theme;
	const Brush& bg = ! is_enabled() ? pick_brush( renderer, t.row_disabled, t.row )
		: ( pressed_ ? t.row_pressed : ( focused_ ? t.row_focused : t.row ) );
	const Color& fg = ! is_enabled() ? t.text_disabled : ( focused_ ? t.text_focused : t.text );

	draw_brush( renderer, rect_, bg );
	draw_text_fit( renderer, rect_, ctx.tr( text_key_ ), make_text_style( fg, t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Button::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.select" } );
}

} // namespace core::ui

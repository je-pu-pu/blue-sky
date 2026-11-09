#include "Button.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

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

float_t Button::measure_height( float_t, const UIContext& ctx ) const
{
	return ctx.theme.row_height;
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
	const Color& bg = pressed_ ? t.row_pressed : ( focused_ ? t.row_focused : t.row );
	const Color& fg = ! is_enabled() ? t.text_disabled : ( focused_ ? t.text_focused : t.text );

	renderer.draw_rect( rect_, bg );
	renderer.draw_text( rect_, ctx.tr( text_key_ ).c_str(), make_text_style( fg, t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Button::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.select" } );
}

} // namespace core::ui

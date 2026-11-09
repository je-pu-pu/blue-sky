#include "Toggle.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

namespace core::ui
{

void Toggle::flip( UIContext& ctx )
{
	if ( ! setter_ || ! is_enabled() )
	{
		return;
	}

	setter_( ! get_value() );
	ctx.screen.play( UISoundEvent::CHANGE );
}

bool Toggle::on_confirm( UIContext& ctx )
{
	flip( ctx );
	return true;
}

bool Toggle::on_nav( NavDir dir, UIContext& ctx )
{
	if ( dir == NavDir::LEFT || dir == NavDir::RIGHT )
	{
		flip( ctx );
		return true;
	}

	return false;
}

bool Toggle::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	if ( e.type == PointerEvent::Type::PRESS )
	{
		pressed_ = true;
	}
	else if ( e.type == PointerEvent::Type::RELEASE )
	{
		if ( pressed_ && e.inside )
		{
			flip( ctx );
		}

		pressed_ = false;
	}

	return true;
}

void Toggle::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const bool on = get_value();

	// 値エリア : 背景 + 「オン／オフ」
	const Rect box = value_rect_.inset( 0.f, rect_.h * 0.15f );
	renderer.draw_rect( box, t.track );

	if ( on )
	{
		renderer.draw_rect( Rect( box.x, box.y, 8.f, box.h ), t.accent );
	}

	renderer.draw_text( box, ctx.tr( on ? "ui.on" : "ui.off" ).c_str(), make_text_style( value_color( ctx ), t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Toggle::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.toggle" } );
}

} // namespace core::ui

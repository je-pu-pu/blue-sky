#include "Toggle.h"
#include "TextFit.h"
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
	ctx.screen.play( UISoundEvent::CHANGE, this );
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
	if ( click_.feed( e ) )
	{
		flip( ctx );
	}

	return true;
}

void Toggle::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const bool on = get_value();

	// 値エリア : 背景 + 「オン／オフ」
	const Rect box = value_rect_.inset( 0.f, rect_.h * t.value_box_inset );
	draw_brush( renderer, box, t.track );

	if ( on )
	{
		// オンの見た目がテーマにあれば欄全体に重ねる。なければ左端に帯
		if ( can_draw( renderer, t.toggle_on ) )
		{
			draw_brush( renderer, box, t.toggle_on );
		}
		else
		{
			draw_brush( renderer, Rect( box.x, box.y, t.toggle_mark_width, box.h ), t.accent );
		}
	}

	draw_text_fit( renderer, box, ctx.tr( on ? "ui.on" : "ui.off" ), make_text_style( value_color( ctx ), t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Toggle::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.toggle" } );
}

AccessibleInfo Toggle::describe( const UIContext& ctx ) const
{
	AccessibleInfo info = LabeledRow::describe( ctx );
	info.role = "toggle";
	info.value = ctx.tr( get_value() ? "ui.on" : "ui.off" );
	return info;
}

} // namespace core::ui

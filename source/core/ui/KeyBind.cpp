#include "KeyBind.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

namespace core::ui
{

void KeyBind::start( UIContext& ctx )
{
	if ( waiting_ || ! is_enabled() )
	{
		return;
	}

	waiting_ = true;
	ctx.screen.begin_key_capture( this );
	ctx.screen.play( UISoundEvent::CONFIRM );
}

bool KeyBind::on_confirm( UIContext& ctx )
{
	start( ctx );
	return true;
}

bool KeyBind::on_cancel( UIContext& ctx )
{
	if ( ! waiting_ )
	{
		return false;	// 待っていなければ画面の「戻る」に任せる
	}

	waiting_ = false;
	ctx.screen.end_key_capture();
	ctx.screen.play( UISoundEvent::CANCEL );
	return true;
}

bool KeyBind::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	if ( e.type == PointerEvent::Type::PRESS )
	{
		pressed_ = true;
	}
	else if ( e.type == PointerEvent::Type::RELEASE )
	{
		if ( pressed_ && e.inside )
		{
			start( ctx );
		}

		pressed_ = false;
	}

	return true;
}

bool KeyBind::on_key_captured( int code, UIContext& ctx )
{
	if ( ! waiting_ )
	{
		return false;
	}

	waiting_ = false;
	ctx.screen.end_key_capture();

	if ( setter_ )
	{
		setter_( code );
	}

	ctx.screen.play( UISoundEvent::CHANGE );
	return true;
}

void KeyBind::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const Rect box = value_rect_.inset( 0.f, rect_.h * 0.15f );

	renderer.draw_rect( box, waiting_ ? t.row_pressed : t.track );

	const string_t text = waiting_ ? ctx.tr( "ui.key_waiting" ) : ( name_of_ && getter_ ? name_of_( getter_() ) : string_t() );
	renderer.draw_text( box, text.c_str(), make_text_style( value_color( ctx ), t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void KeyBind::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.rebind" } );
}

} // namespace core::ui

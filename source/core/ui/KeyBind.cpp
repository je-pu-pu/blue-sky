#include "KeyBind.h"
#include "TextFit.h"
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
	ctx.screen.play( UISoundEvent::CONFIRM, this );
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
	ctx.screen.play( UISoundEvent::CANCEL, this );
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

	ctx.screen.play( UISoundEvent::CHANGE, this );
	return true;
}

void KeyBind::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const Rect box = value_rect_.inset( 0.f, rect_.h * t.value_box_inset );

	draw_brush( renderer, box, waiting_ ? t.row_pressed : t.track );

	const string_t text = waiting_ ? ctx.tr( "ui.key_waiting" ) : ( name_of_ && getter_ ? name_of_( getter_() ) : string_t() );
	draw_text_fit( renderer, box, text, make_text_style( value_color( ctx ), t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void KeyBind::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CONFIRM, "hint.rebind" } );
}

AccessibleInfo KeyBind::describe( const UIContext& ctx ) const
{
	AccessibleInfo info = LabeledRow::describe( ctx );
	info.role = "key_bind";
	info.value = waiting_ ? ctx.tr( "ui.key_waiting" ) : ( name_of_ && getter_ ? name_of_( getter_() ) : string_t() );
	return info;
}

} // namespace core::ui

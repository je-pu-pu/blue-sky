#include "Choice.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

void Choice::arrange( const Rect& rect, const UIContext& ctx )
{
	LabeledRow::arrange( rect, ctx );

	const float_t arrow_w = std::min( value_rect_.h, value_rect_.w * 0.25f );
	left_arrow_rect_ = Rect( value_rect_.x, value_rect_.y, arrow_w, value_rect_.h );
	right_arrow_rect_ = Rect( value_rect_.right() - arrow_w, value_rect_.y, arrow_w, value_rect_.h );
}

void Choice::step( int delta, UIContext& ctx )
{
	const int n = static_cast< int >( option_keys_.size() );

	if ( n == 0 || ! setter_ || ! is_enabled() )
	{
		return;
	}

	const int current = get_index();
	int next;

	if ( current < 0 )
	{
		// 選択肢に無い現在値からは、先頭（右）または末尾（左）へ
		next = delta > 0 ? 0 : n - 1;
	}
	else
	{
		next = current + delta;

		if ( wrap_ )
		{
			next = ( next % n + n ) % n;
		}
		else if ( next < 0 || next >= n )
		{
			return;	// 端で止まる
		}
	}

	if ( next == current )
	{
		return;
	}

	setter_( next );
	ctx.screen.play( UISoundEvent::CHANGE );
}

bool Choice::on_nav( NavDir dir, UIContext& ctx )
{
	if ( dir == NavDir::LEFT ) { step( -1, ctx ); return true; }
	if ( dir == NavDir::RIGHT ) { step( +1, ctx ); return true; }
	return false;
}

bool Choice::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	if ( e.type == PointerEvent::Type::PRESS )
	{
		pressed_ = true;
	}
	else if ( e.type == PointerEvent::Type::RELEASE )
	{
		if ( pressed_ && e.inside )
		{
			if ( left_arrow_rect_.contains( e.x, e.y ) ) step( -1, ctx );
			else if ( right_arrow_rect_.contains( e.x, e.y ) ) step( +1, ctx );
			else if ( value_rect_.contains( e.x, e.y ) ) step( +1, ctx );
		}

		pressed_ = false;
	}

	return true;
}

string_t Choice::get_display_text( const UIContext& ctx ) const
{
	const int index = get_index();

	if ( index >= 0 && index < static_cast< int >( option_keys_.size() ) )
	{
		return ctx.tr( option_keys_[ index ] );
	}

	return custom_text_ ? custom_text_() : string_t( "-" );
}

void Choice::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const Color fg = value_color( ctx );
	const int index = get_index();
	const int n = static_cast< int >( option_keys_.size() );

	// 端で進めない方向の矢印は薄くする
	const bool can_prev = wrap_ || index != 0;
	const bool can_next = wrap_ || index < 0 || index < n - 1;

	draw_brush( renderer, value_rect_.inset( 0.f, rect_.h * 0.15f ), t.track );
	renderer.draw_text( left_arrow_rect_, "<", make_text_style( can_prev ? fg : t.text_disabled, t.font_size_normal, core::graphics::HAlign::CENTER ) );
	renderer.draw_text( right_arrow_rect_, ">", make_text_style( can_next ? fg : t.text_disabled, t.font_size_normal, core::graphics::HAlign::CENTER ) );

	const Rect text_rect( left_arrow_rect_.right(), value_rect_.y, right_arrow_rect_.x - left_arrow_rect_.right(), value_rect_.h );
	renderer.draw_text( text_rect, get_display_text( ctx ).c_str(), make_text_style( fg, t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Choice::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CHANGE, "hint.change" } );
}

} // namespace core::ui

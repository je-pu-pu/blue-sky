#include "Slider.h"
#include "TextFit.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace core::ui
{

void Slider::arrange( const Rect& rect, const UIContext& ctx )
{
	LabeledRow::arrange( rect, ctx );

	// 値エリアの左 75% が溝、右 25% が数値
	const float_t track_w = value_rect_.w * ctx.theme.slider_track_width;
	const float_t track_h = value_rect_.h * ctx.theme.slider_track_height;
	track_rect_ = Rect( value_rect_.x, value_rect_.y + ( value_rect_.h - track_h ) * 0.5f, track_w, track_h );
	text_rect_ = Rect( value_rect_.x + track_w, value_rect_.y, value_rect_.w - track_w, value_rect_.h );
}

void Slider::set_value( float_t v, UIContext& ctx )
{
	if ( ! setter_ || ! is_enabled() )
	{
		return;
	}

	v = std::clamp( v, min_, max_ );

	if ( std::abs( v - get_value() ) < ( step_ > 0.f ? step_ * 0.5f : 1e-6f ) )
	{
		return;
	}

	setter_( v );
	ctx.screen.play( UISoundEvent::CHANGE, this );
}

float_t Slider::value_from_x( float_t x ) const
{
	if ( track_rect_.w <= 0.f || max_ <= min_ )
	{
		return min_;
	}

	const float_t ratio = std::clamp( ( x - track_rect_.x ) / track_rect_.w, 0.f, 1.f );
	float_t v = min_ + ratio * ( max_ - min_ );

	if ( step_ > 0.f )
	{
		v = min_ + std::round( ( v - min_ ) / step_ ) * step_;
	}

	return std::clamp( v, min_, max_ );
}

bool Slider::on_nav( NavDir dir, UIContext& ctx )
{
	if ( dir == NavDir::LEFT )
	{
		set_value( get_value() - step_, ctx );
		return true;
	}

	if ( dir == NavDir::RIGHT )
	{
		set_value( get_value() + step_, ctx );
		return true;
	}

	return false;
}

bool Slider::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	switch ( e.type )
	{
		case PointerEvent::Type::PRESS:
			// 溝（上下に少し余裕を持たせる）の上で押したときだけドラッグを始める
			if ( Rect( track_rect_.x, rect_.y, track_rect_.w, rect_.h ).contains( e.x, e.y ) )
			{
				dragging_ = true;
				set_value( value_from_x( e.x ), ctx );
			}
			return true;

		case PointerEvent::Type::DRAG:
			if ( dragging_ )
			{
				set_value( value_from_x( e.x ), ctx );
			}
			return true;

		case PointerEvent::Type::RELEASE:
			dragging_ = false;
			return true;
	}

	return false;
}

string_t Slider::format_value( float_t v ) const
{
	if ( formatter_ )
	{
		return formatter_( v );
	}

	std::ostringstream ss;
	ss << std::fixed << std::setprecision( 1 ) << v;
	return ss.str();
}

void Slider::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const float_t v = get_value();
	const float_t ratio = max_ > min_ ? std::clamp( ( v - min_ ) / ( max_ - min_ ), 0.f, 1.f ) : 0.f;

	draw_brush( renderer, track_rect_, t.track );
	// 充填 : 溝と同じ形で描き、値の位置で切り取る（形を縮めると 9 分割の角が潰れるため）
	const Brush& fill = is_enabled() ? pick_brush( renderer, t.slider_fill, t.accent ) : t.fill_disabled;
	const Rect fill_clip( track_rect_.x, track_rect_.y, track_rect_.w * ratio, track_rect_.h );
	draw_brush( renderer, track_rect_, fill, & fill_clip );

	// つまみ（テーマで指定され、実際に描けるときだけ）。中心を値の位置に置き、高さは溝の 2 倍
	if ( can_draw( renderer, t.slider_knob ) && t.slider_knob_width > 0.f )
	{
		const float_t knob_h = track_rect_.h * 2.f;
		const float_t knob_x = track_rect_.x + track_rect_.w * ratio - t.slider_knob_width * 0.5f;
		draw_brush( renderer, Rect( knob_x, track_rect_.center_y() - knob_h * 0.5f, t.slider_knob_width, knob_h ), t.slider_knob );
	}
	draw_text_fit( renderer, text_rect_, format_value( v ), make_text_style( value_color( ctx ), t.font_size_normal, core::graphics::HAlign::CENTER ) );
}

void Slider::collect_hints( std::vector< Hint >& hints ) const
{
	hints.push_back( Hint{ HintAction::CHANGE, "hint.adjust" } );
}

AccessibleInfo Slider::describe( const UIContext& ctx ) const
{
	AccessibleInfo info = LabeledRow::describe( ctx );
	info.role = "slider";
	info.value = format_value( get_value() );
	return info;
}

} // namespace core::ui

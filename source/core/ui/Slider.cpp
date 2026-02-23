#include "Slider.h"
#include <core/ui/Renderer.h>

#include <core/input/InputManager.h>

#include <sstream>
#include <iomanip>

namespace core::ui
{

float_t Slider::get_ratio() const
{
	if ( max_ <= min_ ) return 0.f;

	float_t r = ( value_ - min_ ) / ( max_ - min_ );
	if ( r < 0.f ) r = 0.f;
	if ( r > 1.f ) r = 1.f;
	return r;
}

void Slider::clamp_value()
{
	if ( value_ < min_ ) value_ = min_;
	if ( value_ > max_ ) value_ = max_;
}

void Slider::set_value_from_mouse( float_t mouse_vx )
{
	float_t track_x = x_ + width_ * LABEL_RATIO;
	float_t track_w = width_ * TRACK_RATIO;

	float_t ratio = ( mouse_vx - track_x ) / track_w;
	if ( ratio < 0.f ) ratio = 0.f;
	if ( ratio > 1.f ) ratio = 1.f;

	float_t new_value = min_ + ratio * ( max_ - min_ );

	// ステップにスナップ
	if ( step_ > 0.f )
	{
		new_value = min_ + static_cast< float_t >( static_cast< int >( ( new_value - min_ ) / step_ + 0.5f ) ) * step_;
	}

	clamp_value();

	if ( new_value != value_ )
	{
		value_ = new_value;
		clamp_value();

		if ( on_change_ )
		{
			on_change_( value_ );
		}
	}
}

string_t Slider::format_value() const
{
	if ( formatter_ )
	{
		return formatter_( value_ );
	}

	// デフォルト: 小数第1位まで
	std::stringstream ss;
	ss << std::fixed << std::setprecision( 1 ) << value_;
	return ss.str();
}

void Slider::update( core::input::InputManager* input, Renderer& renderer )
{
	if ( ! focused_ )
	{
		state_ = WidgetState::NORMAL;
		dragging_ = false;
		return;
	}

	state_ = WidgetState::FOCUSED;

	// キーボード : 左右キーでステップ調整
	if ( input->push( core::input::Button::LEFT ) )
	{
		value_ -= step_;
		clamp_value();

		if ( on_change_ ) on_change_( value_ );
	}

	if ( input->push( core::input::Button::RIGHT ) )
	{
		value_ += step_;
		clamp_value();

		if ( on_change_ ) on_change_( value_ );
	}

	// マウス : トラック領域のクリック/ドラッグ
	float_t mx = renderer.physical_to_virtual_x( static_cast< float_t >( input->get_mouse_x() ) );

	if ( input->push( core::input::Button::A ) )
	{
		float_t track_x = x_ + width_ * LABEL_RATIO;
		float_t track_w = width_ * TRACK_RATIO;

		if ( mx >= track_x && mx <= track_x + track_w )
		{
			dragging_ = true;
			set_value_from_mouse( mx );
		}
	}

	if ( dragging_ )
	{
		if ( input->press( core::input::Button::A ) )
		{
			set_value_from_mouse( mx );
			state_ = WidgetState::PRESSED;
		}
		else
		{
			dragging_ = false;
		}
	}
}

void Slider::render( Renderer& renderer )
{
	const Color& lbl_color = focused_ ? label_focused_color_ : label_color_;

	float_t label_x = x_;
	float_t label_w = width_ * LABEL_RATIO;

	float_t track_x = x_ + label_w;
	float_t track_w = width_ * TRACK_RATIO;

	float_t value_x = track_x + track_w;
	float_t value_w = width_ * VALUE_RATIO;

	// ラベル（左寄せ・垂直中央）
	core::graphics::TextStyle label_style;
	label_style.text_color = lbl_color;
	label_style.v_align = core::graphics::VAlign::CENTER;
	renderer.draw_text( label_x, y_, label_w, height_, label_.c_str(), label_style );

	// トラック背景
	float_t pad = height_ * TRACK_PADDING_Y;
	renderer.draw_rect( track_x, y_ + pad, track_w, height_ - pad * 2.f, track_bg_color_ );

	// トラック充填
	float_t ratio = get_ratio();
	const Color& fill_color = focused_ ? track_fill_focused_ : track_fill_color_;
	renderer.draw_rect( track_x, y_ + pad, track_w * ratio, height_ - pad * 2.f, fill_color );

	// 値表示（水平中央・垂直中央）
	core::graphics::TextStyle value_style;
	value_style.text_color = value_color_;
	value_style.h_align = core::graphics::HAlign::CENTER;
	value_style.v_align = core::graphics::VAlign::CENTER;
	renderer.draw_text( value_x, y_, value_w, height_, format_value().c_str(), value_style );
}

} // namespace core::ui

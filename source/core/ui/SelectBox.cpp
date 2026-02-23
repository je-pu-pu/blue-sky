#include "SelectBox.h"
#include <core/ui/Renderer.h>

#include <core/input/InputManager.h>

namespace core::ui
{

static const string_t EMPTY_STRING;

void SelectBox::set_selected( int index )
{
	if ( options_.empty() )
	{
		selected_ = 0;
		return;
	}

	if ( index < 0 ) index = 0;
	if ( index >= static_cast< int >( options_.size() ) ) index = static_cast< int >( options_.size() ) - 1;

	selected_ = index;
}

const string_t& SelectBox::get_selected_text() const
{
	if ( options_.empty() ) return EMPTY_STRING;
	return options_[ selected_ ];
}

void SelectBox::select_prev()
{
	if ( options_.empty() ) return;

	if ( selected_ > 0 )
	{
		selected_--;

		if ( on_change_ ) on_change_( selected_ );
	}
}

void SelectBox::select_next()
{
	if ( options_.empty() ) return;

	if ( selected_ < static_cast< int >( options_.size() ) - 1 )
	{
		selected_++;

		if ( on_change_ ) on_change_( selected_ );
	}
}

void SelectBox::update( core::input::InputManager* input, Renderer& renderer )
{
	if ( ! focused_ )
	{
		state_ = WidgetState::NORMAL;
		return;
	}

	state_ = WidgetState::FOCUSED;

	// キーボード : 左右キーで選択切替
	if ( input->push( core::input::Button::LEFT ) )
	{
		select_prev();
	}

	if ( input->push( core::input::Button::RIGHT ) )
	{
		select_next();
	}

	// マウス : 矢印クリック
	if ( input->push( core::input::Button::A ) )
	{
		float_t mx = renderer.physical_to_virtual_x( static_cast< float_t >( input->get_mouse_x() ) );
		float_t my = renderer.physical_to_virtual_y( static_cast< float_t >( input->get_mouse_y() ) );

		float_t content_x = x_ + width_ * LABEL_RATIO;
		float_t content_w = width_ * ( 1.f - LABEL_RATIO );

		if ( my >= y_ && my < y_ + height_ )
		{
			// 左矢印
			if ( mx >= content_x && mx < content_x + ARROW_WIDTH )
			{
				select_prev();
			}
			// 右矢印
			else if ( mx >= content_x + content_w - ARROW_WIDTH && mx < content_x + content_w )
			{
				select_next();
			}
		}
	}
}

void SelectBox::render( Renderer& renderer )
{
	const Color& lbl_color = focused_ ? label_focused_color_ : label_color_;
	const Color& arr_color = focused_ ? arrow_focused_color_ : arrow_color_;

	float_t label_x = x_;
	float_t label_w = width_ * LABEL_RATIO;

	float_t content_x = x_ + label_w;
	float_t content_w = width_ * ( 1.f - LABEL_RATIO );

	core::graphics::TextStyle center_style;
	center_style.h_align = core::graphics::HAlign::CENTER;
	center_style.v_align = core::graphics::VAlign::CENTER;

	// ラベル（左寄せ・垂直中央）
	core::graphics::TextStyle label_style;
	label_style.text_color = lbl_color;
	label_style.v_align = core::graphics::VAlign::CENTER;
	renderer.draw_text( label_x, y_, label_w, height_, label_.c_str(), label_style );

	// 背景
	renderer.draw_rect( content_x, y_, content_w, height_, bg_color_ );

	// 左矢印
	center_style.text_color = arr_color;
	renderer.draw_text( content_x, y_, ARROW_WIDTH, height_, "<", center_style );

	// 値テキスト
	float_t value_x = content_x + ARROW_WIDTH;
	float_t value_w = content_w - ARROW_WIDTH * 2.f;
	center_style.text_color = value_color_;
	renderer.draw_text( value_x, y_, value_w, height_, get_selected_text().c_str(), center_style );

	// 右矢印
	center_style.text_color = arr_color;
	renderer.draw_text( content_x + content_w - ARROW_WIDTH, y_, ARROW_WIDTH, height_, ">", center_style );
}

} // namespace core::ui

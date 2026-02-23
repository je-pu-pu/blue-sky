#include "UISelectBox.h"
#include "UIRenderer.h"

#include <blue_sky/Input.h>

namespace blue_sky::ui
{

static const string_t EMPTY_STRING;

void UISelectBox::set_selected( int index )
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

const string_t& UISelectBox::get_selected_text() const
{
	if ( options_.empty() ) return EMPTY_STRING;
	return options_[ selected_ ];
}

void UISelectBox::select_prev()
{
	if ( options_.empty() ) return;

	if ( selected_ > 0 )
	{
		selected_--;

		if ( on_change_ ) on_change_( selected_ );
	}
}

void UISelectBox::select_next()
{
	if ( options_.empty() ) return;

	if ( selected_ < static_cast< int >( options_.size() ) - 1 )
	{
		selected_++;

		if ( on_change_ ) on_change_( selected_ );
	}
}

void UISelectBox::update( Input* input, UIRenderer& renderer )
{
	if ( ! focused_ )
	{
		state_ = WidgetState::NORMAL;
		return;
	}

	state_ = WidgetState::FOCUSED;

	// キーボード : 左右キーで選択切替
	if ( input->push( Input::Button::LEFT ) )
	{
		select_prev();
	}

	if ( input->push( Input::Button::RIGHT ) )
	{
		select_next();
	}

	// マウス : 矢印クリック
	if ( input->push( Input::Button::A ) )
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

void UISelectBox::render( UIRenderer& renderer )
{
	const Color& lbl_color = focused_ ? label_focused_color_ : label_color_;
	const Color& arr_color = focused_ ? arrow_focused_color_ : arrow_color_;

	float_t label_x = x_;
	float_t label_w = width_ * LABEL_RATIO;

	float_t content_x = x_ + label_w;
	float_t content_w = width_ * ( 1.f - LABEL_RATIO );

	// ラベル
	renderer.draw_text( label_x, y_, label_w, height_, label_.c_str(), lbl_color );

	// 背景
	renderer.draw_rect( content_x, y_, content_w, height_, bg_color_ );

	// 左矢印
	renderer.draw_text_center( content_x, y_, ARROW_WIDTH, height_, "<", arr_color );

	// 値テキスト
	float_t value_x = content_x + ARROW_WIDTH;
	float_t value_w = content_w - ARROW_WIDTH * 2.f;
	renderer.draw_text_center( value_x, y_, value_w, height_, get_selected_text().c_str(), value_color_ );

	// 右矢印
	renderer.draw_text_center( content_x + content_w - ARROW_WIDTH, y_, ARROW_WIDTH, height_, ">", arr_color );
}

} // namespace blue_sky::ui

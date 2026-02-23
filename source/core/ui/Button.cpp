#include "Button.h"
#include <core/ui/Renderer.h>

#include <core/input/InputManager.h>

namespace core::ui
{

void Button::update( core::input::InputManager* input, Renderer& )
{
	if ( ! focused_ )
	{
		state_ = WidgetState::NORMAL;
		return;
	}

	// マウス左ボタンが押されている間は PRESSED
	if ( input->press( core::input::Button::A ) )
	{
		state_ = WidgetState::PRESSED;
	}
	else
	{
		state_ = WidgetState::FOCUSED;
	}

	// 決定
	if ( input->push( core::input::Button::A ) )
	{
		if ( on_click_ )
		{
			on_click_();
		}
	}
}

void Button::render( Renderer& renderer )
{
	// 背景矩形
	const Color& bg = ( state_ == WidgetState::PRESSED ) ? color_pressed_
	                 : ( state_ == WidgetState::FOCUSED ) ? color_focused_
	                 : color_normal_;

	renderer.draw_rect( x_, y_, width_, height_, bg );

	// テキスト中央描画
	core::graphics::TextStyle style;
	style.text_color = text_color_;
	style.h_align = core::graphics::HAlign::CENTER;
	style.v_align = core::graphics::VAlign::CENTER;
	renderer.draw_text( x_, y_, width_, height_, text_.c_str(), style );
}

} // namespace core::ui

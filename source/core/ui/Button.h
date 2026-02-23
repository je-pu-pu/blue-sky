#pragma once

#include "Widget.h"

#include <functional>

namespace core::ui
{

/**
 * ボタンウィジェット
 *
 * 背景矩形 + テキストで構成される。
 * WidgetState に応じて背景色が 3 段階変化する。
 */
class Button : public Widget
{
private:
	string_t text_;
	std::function< void() > on_click_;

	Color color_normal_   = Color( 0.15f, 0.15f, 0.2f, 0.9f );
	Color color_focused_  = Color( 0.25f, 0.25f, 0.35f, 0.9f );
	Color color_pressed_  = Color( 0.4f, 0.4f, 0.55f, 0.9f );
	Color text_color_     = Color( 1.f, 1.f, 1.f, 1.f );

public:
	Button() { set_hint_text( "Enter/Click Select    ESC Back" ); }

	void set_text( const string_t& text ) { text_ = text; }
	const string_t& get_text() const { return text_; }

	void set_on_click( std::function< void() > callback ) { on_click_ = callback; }

	void set_colors( const Color& normal, const Color& focused, const Color& pressed )
	{
		color_normal_ = normal;
		color_focused_ = focused;
		color_pressed_ = pressed;
	}

	void set_text_color( const Color& c ) { text_color_ = c; }

	void update( core::input::InputManager* input, Renderer& renderer ) override;
	void render( Renderer& renderer ) override;

}; // class Button

} // namespace core::ui

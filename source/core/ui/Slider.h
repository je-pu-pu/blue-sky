#pragma once

#include "Widget.h"

#include <functional>

namespace core::ui
{

/**
 * スライダーウィジェット
 *
 * [ラベル 30%] [トラック 50%] [値表示 20%] のレイアウト。
 * トラック: 背景矩形 + 充填矩形（値に比例した幅）。
 */
class Slider : public Widget
{
public:
	using Formatter = std::function< string_t( float_t ) >;

private:
	string_t label_;
	float_t min_ = 0.f;
	float_t max_ = 1.f;
	float_t step_ = 0.1f;
	float_t value_ = 0.5f;

	std::function< void( float_t ) > on_change_;
	Formatter formatter_;

	bool dragging_ = false;

	// レイアウト比率
	static constexpr float_t LABEL_RATIO = 0.30f;
	static constexpr float_t TRACK_RATIO = 0.50f;
	static constexpr float_t VALUE_RATIO = 0.20f;

	// トラック上下余白
	static constexpr float_t TRACK_PADDING_Y = 0.25f;

	// 色
	Color track_bg_color_       = Color( 0.1f, 0.1f, 0.15f, 0.9f );
	Color track_fill_color_     = Color( 0.3f, 0.5f, 0.8f, 0.9f );
	Color track_fill_focused_   = Color( 0.4f, 0.65f, 1.f, 0.9f );
	Color label_color_          = Color( 0.8f, 0.8f, 0.8f, 1.f );
	Color label_focused_color_  = Color( 1.f, 1.f, 0.5f, 1.f );
	Color value_color_          = Color( 0.9f, 0.9f, 0.9f, 1.f );

	float_t get_ratio() const;
	void clamp_value();
	void set_value_from_mouse( float_t mouse_vx );

	string_t format_value() const;

public:
	Slider() { set_hint_text( "Arrow/AD Adjust    ESC Back" ); }

	void set_label( const string_t& label ) { label_ = label; }
	void set_range( float_t min_val, float_t max_val ) { min_ = min_val; max_ = max_val; }
	void set_step( float_t s ) { step_ = s; }
	void set_value( float_t v ) { value_ = v; clamp_value(); }
	float_t get_value() const { return value_; }

	void set_on_change( std::function< void( float_t ) > callback ) { on_change_ = callback; }
	void set_format( Formatter fmt ) { formatter_ = fmt; }

	void update( core::input::InputManager* input, Renderer& renderer ) override;
	void render( Renderer& renderer ) override;

}; // class Slider

} // namespace core::ui

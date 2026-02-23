#pragma once

#include "Widget.h"

#include <vector>
#include <functional>

namespace blue_sky::ui
{

/**
 * セレクトボックスウィジェット
 *
 * [ラベル 30%] [< 矢印] [値テキスト] [> 矢印] のレイアウト。
 */
class SelectBox : public Widget
{
private:
	string_t label_;
	std::vector< string_t > options_;
	int selected_ = 0;

	std::function< void( int ) > on_change_;

	// レイアウト比率
	static constexpr float_t LABEL_RATIO = 0.30f;
	static constexpr float_t ARROW_WIDTH = 60.f;

	// 色
	Color label_color_         = Color( 0.8f, 0.8f, 0.8f, 1.f );
	Color label_focused_color_ = Color( 1.f, 1.f, 0.5f, 1.f );
	Color value_color_         = Color( 0.9f, 0.9f, 0.9f, 1.f );
	Color arrow_color_         = Color( 0.6f, 0.6f, 0.7f, 1.f );
	Color arrow_focused_color_ = Color( 1.f, 1.f, 0.5f, 1.f );
	Color bg_color_            = Color( 0.1f, 0.1f, 0.15f, 0.9f );

	void select_prev();
	void select_next();

public:
	SelectBox() { set_hint_text( "Arrow/AD Change    ESC Back" ); }

	void set_label( const string_t& label ) { label_ = label; }

	void add_option( const string_t& option ) { options_.push_back( option ); }
	void clear_options() { options_.clear(); selected_ = 0; }

	void set_selected( int index );
	int get_selected() const { return selected_; }

	const string_t& get_selected_text() const;

	void set_on_change( std::function< void( int ) > callback ) { on_change_ = callback; }

	void update( Input* input, Renderer& renderer ) override;
	void render( Renderer& renderer ) override;

}; // class SelectBox

} // namespace blue_sky::ui

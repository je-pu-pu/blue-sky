#pragma once

#include <core/type.h>

#include <vector>

namespace core::input
{
	class InputManager;
}

namespace core::ui
{
	class Renderer;
	class Widget;

	/**
	 * ウィジェットコンテナ
	 *
	 * 複数の Widget を縦並びで管理し、
	 * フォーカス移動（上下キー / マウスホバー）と入力転送を行う。
	 */
	class WidgetContainer
	{
	private:
		std::vector< Widget* > widgets_;
		int focused_index_ = 0;

		// レイアウト
		float_t x_ = 0.f;
		float_t y_ = 0.f;
		float_t width_ = 1440.f;
		float_t item_height_ = 100.f;
		float_t spacing_ = 0.f;

		// マウス操作用
		int last_mouse_x_ = -1;
		int last_mouse_y_ = -1;

		void update_focus( int new_index );

	public:
		void add_widget( Widget* widget ) { widgets_.push_back( widget ); }

		void set_position( float_t x, float_t y ) { x_ = x; y_ = y; }
		void set_width( float_t w ) { width_ = w; }
		void set_item_height( float_t h ) { item_height_ = h; }
		void set_spacing( float_t s ) { spacing_ = s; }

		/// 追加済みウィジェットの位置・サイズを一括設定
		void layout();

		void update( core::input::InputManager* input, Renderer& renderer );
		void render( Renderer& renderer );

		Widget* get_focused_widget() const;
		int get_focused_index() const { return focused_index_; }

		int get_widget_count() const { return static_cast< int >( widgets_.size() ); }

	}; // class WidgetContainer

} // namespace core::ui

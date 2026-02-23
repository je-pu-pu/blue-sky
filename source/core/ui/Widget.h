#pragma once

#include <core/type.h>

#include <string>

namespace core::input
{
	class InputManager;
}

namespace core::ui
{
	class Renderer;

	enum class WidgetState
	{
		NORMAL,
		FOCUSED,
		PRESSED
	};

	/**
	 * UI ウィジェットの抽象基底クラス
	 *
	 * ボタン・スライダー・セレクトボックスなどの共通インターフェースを定義する。
	 */
	class Widget
	{
	protected:
		float_t x_ = 0.f;
		float_t y_ = 0.f;
		float_t width_ = 400.f;
		float_t height_ = 80.f;

		bool focused_ = false;
		WidgetState state_ = WidgetState::NORMAL;

		string_t hint_text_;

	public:
		virtual ~Widget() = default;

		virtual void update( core::input::InputManager* input, Renderer& renderer ) = 0;
		virtual void render( Renderer& renderer ) = 0;

		void set_position( float_t x, float_t y ) { x_ = x; y_ = y; }
		void set_size( float_t w, float_t h ) { width_ = w; height_ = h; }

		float_t get_x() const { return x_; }
		float_t get_y() const { return y_; }
		float_t get_width() const { return width_; }
		float_t get_height() const { return height_; }

		bool hit_test( float_t px, float_t py ) const
		{
			return px >= x_ && px <= x_ + width_ && py >= y_ && py < y_ + height_;
		}

		void set_focused( bool f )
		{
			focused_ = f;
			if ( ! f ) state_ = WidgetState::NORMAL;
			else if ( state_ == WidgetState::NORMAL ) state_ = WidgetState::FOCUSED;
		}

		bool is_focused() const { return focused_; }

		void set_hint_text( const string_t& text ) { hint_text_ = text; }
		const string_t& get_hint_text() const { return hint_text_; }

	}; // class Widget

} // namespace core::ui

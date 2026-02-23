#pragma once

#include <core/type.h>
#include <core/graphics/TextStyle.h>

namespace core::ui
{

	/**
	 * UI 描画の抽象基底クラス
	 *
	 * 仮想座標系でのテキスト・矩形描画インターフェースを定義する。
	 * 具象実装はグラフィックス API 固有の処理を提供する。
	 */
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual float_t get_screen_width() const = 0;
		virtual float_t get_screen_height() const = 0;

		virtual float_t physical_to_virtual_x( float_t px ) const = 0;
		virtual float_t physical_to_virtual_y( float_t py ) const = 0;

		virtual void draw_text( float_t x, float_t y, float_t w, float_t h,
		                        const char_t* text, const Color& color ) = 0;
		virtual void draw_text( float_t x, float_t y, float_t w, float_t h,
		                        const char_t* text, const core::graphics::TextStyle& style ) = 0;

		virtual void draw_rect( float_t x, float_t y, float_t w, float_t h,
		                        const Color& color ) = 0;

	}; // class Renderer

} // namespace core::ui

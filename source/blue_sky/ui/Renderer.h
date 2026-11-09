#pragma once

#include <blue_sky/type.h>
#include <core/ui/Renderer.h>

namespace blue_sky::graphics
{
	class GraphicsManager;
}

namespace blue_sky::ui
{

/**
 * core::ui::Renderer の具象実装（GraphicsManager の Sprite / MSDF テキストを使う）
 *
 * 仮想座標系は 1920x1080。物理画面へは**等方スケール**（縦横同じ倍率）で写し、
 * 余った左右・上下は中央寄せの余白にする（16:9 以外の解像度でも UI が歪まない）。
 * 余白も含めて塗りたいときは fill_screen() を使う。
 */
class Renderer : public core::ui::Renderer
{
public:
	using GraphicsManager = blue_sky::graphics::GraphicsManager;
	using Rect = core::ui::Rect;

	/// 仮想座標系の基準解像度
	static constexpr float_t DESIGN_WIDTH  = 1920.f;
	static constexpr float_t DESIGN_HEIGHT = 1080.f;

private:
	GraphicsManager* gm_;

	float_t get_scale() const;
	float_t get_offset_x() const;
	float_t get_offset_y() const;

	Rect to_physical( const Rect& ) const;

public:
	explicit Renderer( GraphicsManager* );

	float_t get_screen_width() const override { return DESIGN_WIDTH; }
	float_t get_screen_height() const override { return DESIGN_HEIGHT; }

	/// 物理ピクセル座標 → 仮想座標（マウス座標用。範囲外はクランプしない）
	float_t physical_to_virtual_x( float_t px ) const;
	float_t physical_to_virtual_y( float_t py ) const;

	void draw_text( const Rect& rect, const char_t* text, const core::graphics::TextStyle& style ) override;
	void draw_rect( const Rect& rect, const Color& color ) override;
	void fill_screen( const Color& color ) override;

	float_t measure_text_width( const char_t* text, float_t font_size ) const override;
	float_t measure_line_height( float_t font_size ) const override;

}; // class Renderer

} // namespace blue_sky::ui

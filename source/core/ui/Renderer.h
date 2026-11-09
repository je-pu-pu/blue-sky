#pragma once

#include <core/type.h>
#include <core/graphics/TextStyle.h>
#include <core/ui/Rect.h>

namespace core::ui
{

/**
 * UI 描画の抽象インターフェース
 *
 * すべての座標は仮想座標系（get_screen_width() x get_screen_height()、既定 1920x1080）。
 * 具象実装（blue_sky::ui::Renderer）が物理ピクセルへの変換を行う。
 * テストでは描画を記録するモックを使う。
 */
class Renderer
{
public:
	virtual ~Renderer() = default;

	/// 仮想画面のサイズ
	virtual float_t get_screen_width() const = 0;
	virtual float_t get_screen_height() const = 0;

	/// 矩形内にテキストを描画する（配置は style.h_align / v_align。font_size は仮想座標のピクセル）
	virtual void draw_text( const Rect& rect, const char_t* text, const core::graphics::TextStyle& style ) = 0;

	/// 塗りつぶし矩形
	virtual void draw_rect( const Rect& rect, const Color& color ) = 0;

	/// 物理画面全体を塗る（等方スケールで生じる左右・上下の余白も含む）
	virtual void fill_screen( const Color& color ) = 0;

	/// テキストの幅（最初の改行まで）
	virtual float_t measure_text_width( const char_t* text, float_t font_size ) const = 0;

	/// 1 行の高さ
	virtual float_t measure_line_height( float_t font_size ) const = 0;

}; // class Renderer

} // namespace core::ui

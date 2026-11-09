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

	/**
	 * 画像の大きさ（ピクセル）を返す
	 *
	 * 画像名の解釈（ファイルの場所など）は具象実装が決める。
	 * 読めない画像なら false を返す（呼び出し側は単色などで代用する）。
	 */
	virtual bool get_image_size( const string_t& image, float_t& width, float_t& height ) const = 0;

	/**
	 * 画像の一部を矩形に描く
	 *
	 * @param rect 描画先（仮想座標）
	 * @param image 画像名
	 * @param source 切り出し範囲（画像のピクセル座標）
	 * @param tint 掛け合わせる色
	 *
	 * 9 分割などの分解は呼び出し側（draw_brush）で済ませてから呼ぶ。
	 */
	virtual void draw_image( const Rect& rect, const string_t& image, const Rect& source, const Color& tint ) = 0;

	/// テキストの幅（最初の改行まで）
	virtual float_t measure_text_width( const char_t* text, float_t font_size ) const = 0;

	/// 1 行の高さ
	virtual float_t measure_line_height( float_t font_size ) const = 0;

	/// 仮想座標の 1 が物理ピクセルでいくつか（既定 1）。9 分割の枠幅を整数ピクセルにそろえるのに使う
	virtual float_t get_pixel_scale() const { return 1.f; }

	/**
	 * 矩形の四辺を物理ピクセルの境界にそろえた矩形（仮想座標）を返す（既定はそのまま）
	 *
	 * 具象実装の draw_rect / draw_image は、受け取った矩形をこれと同じ規則で物理ピクセルに丸めること
	 * （そろえた矩形を渡せば、そのままのピクセルに描かれる）。
	 */
	virtual Rect snap_to_pixels( const Rect& rect ) const { return rect; }

}; // class Renderer

} // namespace core::ui

#pragma once

#include <core/type.h>

namespace core::graphics {

/**
 * 水平方向のテキスト配置
 *
 * draw_text() に渡す矩形 (left, top, right, bottom) 内での水平位置を指定する。
 */
enum class HAlign {
	LEFT,    ///< 矩形の左端に揃える（デフォルト）
	CENTER,  ///< 矩形の水平中央に揃える
	RIGHT    ///< 矩形の右端に揃える
};

/**
 * 垂直方向のテキスト配置
 *
 * draw_text() に渡す矩形 (left, top, right, bottom) 内での垂直位置を指定する。
 */
enum class VAlign {
	TOP,     ///< 矩形の上端に揃える（デフォルト）
	CENTER,  ///< 矩形の垂直中央に揃える
	BOTTOM   ///< 矩形の下端に揃える
};

/**
 * テキスト描画スタイル
 *
 * フォントサイズ、色、アウトライン、配置をまとめた構造体。
 * GraphicsManager::draw_text() や ui::Renderer::draw_text() に渡して使用する。
 *
 * @par 使用例
 * @code
 * TextStyle style;
 * style.text_color = Color::White;
 * style.h_align = HAlign::CENTER;
 * style.v_align = VAlign::CENTER;
 * renderer.draw_text( x, y, w, h, "Hello", style );
 * @endcode
 *
 * @note デフォルトは LEFT+TOP なので、既存の draw_text() 呼び出しは挙動が変わらない。
 *       集成体初期化 TextStyle{ color, outline_color, outline_width } も引き続き使える。
 */
struct TextStyle {
	Color text_color = Color::White;     ///< テキスト本体の色
	Color outline_color = Color::Black;  ///< アウトラインの色
	float outline_width = 0.f;           ///< アウトラインの太さ（スクリーンピクセル単位、0 = アウトラインなし）
	float font_size = 0.f;               ///< フォントサイズ（ピクセル）。0 = GraphicsManager のデフォルトサイズを使用
	HAlign h_align = HAlign::LEFT;       ///< 水平配置（矩形内での左寄せ/中央/右寄せ）
	VAlign v_align = VAlign::TOP;        ///< 垂直配置（矩形内での上寄せ/中央/下寄せ）
};

} // namespace core::graphics

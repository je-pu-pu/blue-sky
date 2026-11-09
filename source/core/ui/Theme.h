#pragma once

#include <core/type.h>
#include <core/graphics/TextStyle.h>

namespace core::ui
{

/**
 * UI の見た目の定義
 *
 * 色・フォントサイズ・寸法を 1 か所にまとめる。寸法は仮想座標（1920x1080 基準）。
 * 部品は自前で色を持たず、ここから読む。
 */
struct Theme
{
	// 色
	Color screen_dim		= Color( 0.f, 0.f, 0.f, 0.55f );		///< 画面全体の暗幕
	Color dialog_dim		= Color( 0.f, 0.f, 0.f, 0.5f );			///< ダイアログ背面の暗幕
	Color panel				= Color( 0.05f, 0.06f, 0.1f, 0.92f );	///< パネル背景
	Color row				= Color( 0.12f, 0.13f, 0.18f, 0.85f );	///< 行の背景
	Color row_focused		= Color( 0.22f, 0.28f, 0.42f, 0.95f );	///< フォーカス中の行
	Color row_pressed		= Color( 0.32f, 0.42f, 0.62f, 0.95f );	///< 押している行
	Color track				= Color( 0.06f, 0.07f, 0.1f, 1.f );		///< スライダーの溝
	Color accent			= Color( 0.35f, 0.6f, 1.f, 1.f );		///< スライダーの充填・選択中タブ
	Color text				= Color( 0.92f, 0.92f, 0.92f, 1.f );
	Color text_focused		= Color( 1.f, 1.f, 0.6f, 1.f );
	Color text_dim			= Color( 0.6f, 0.62f, 0.68f, 1.f );		///< 説明文・ヒント
	Color text_disabled		= Color( 0.4f, 0.4f, 0.44f, 1.f );
	Color header			= Color( 0.55f, 0.75f, 1.f, 1.f );		///< 見出し
	Color outline			= Color( 0.f, 0.f, 0.f, 1.f );

	// フォントサイズ（仮想座標のピクセル）
	float_t font_size_title		= 72.f;
	float_t font_size_normal	= 40.f;
	float_t font_size_small		= 30.f;

	// 寸法
	float_t panel_width			= 1500.f;	///< パネルの幅
	float_t panel_margin_y		= 48.f;		///< パネルの上下の画面端との余白
	float_t padding				= 32.f;		///< パネル内側の余白
	float_t title_height		= 110.f;
	float_t footer_height		= 120.f;	///< 説明欄＋ヒント行
	float_t row_height			= 72.f;
	float_t spacing				= 8.f;		///< 行の間隔
	float_t label_ratio			= 0.42f;	///< ラベル付き行のラベル幅の比率
	float_t row_padding_x		= 24.f;		///< 行の左右の余白
	float_t tab_height			= 72.f;
	float_t dialog_width		= 1100.f;

}; // struct Theme

/// テキストスタイルを作る補助（既定は左寄せ・垂直中央・縁取りなし）
inline core::graphics::TextStyle make_text_style(
	const Color& color,
	float_t font_size,
	core::graphics::HAlign h = core::graphics::HAlign::LEFT,
	core::graphics::VAlign v = core::graphics::VAlign::CENTER )
{
	core::graphics::TextStyle style;
	style.text_color = color;
	style.font_size = font_size;
	style.h_align = h;
	style.v_align = v;
	return style;
}

} // namespace core::ui

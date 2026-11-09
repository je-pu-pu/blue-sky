#pragma once

#include <core/type.h>
#include <core/graphics/TextStyle.h>
#include <core/ui/Brush.h>

#include <vector>

namespace core::ui
{
	class Renderer;

/**
 * UI の見た目の定義
 *
 * 色・画像・フォントサイズ・寸法を 1 か所にまとめる。寸法は仮想座標（1920x1080 基準）。
 * 部品は自前で色を持たず、ここから読む。
 * 背景・溝などの「面」は Brush（単色または画像）、文字の色は Color。
 * ファイルからの上書きは load_theme() を使う。
 */
struct Theme
{
	// 画面全体の暗幕（物理画面全体を塗るため単色のみ）
	Color screen_dim		= Color( 0.f, 0.f, 0.f, 0.55f );		///< 画面全体の暗幕
	Color dialog_dim		= Color( 0.f, 0.f, 0.f, 0.5f );			///< ダイアログ背面の暗幕

	// 面（単色または画像）
	// 用途別の面（slider_fill など）は、未指定・画像が読めないときに共通の面（accent / row）で代用する。
	// 共通の面 1 つを形の違う複数の用途（細い帯と太い充填など）に使うと、9 分割の画像が潰れるため分けてある。
	Brush panel				= Color( 0.05f, 0.06f, 0.1f, 0.92f );	///< パネル・ダイアログの背景
	Brush row				= Color( 0.12f, 0.13f, 0.18f, 0.85f );	///< 行・ボタン・タブの背景
	Brush row_focused		= Color( 0.22f, 0.28f, 0.42f, 0.95f );	///< フォーカス中の行・選択中のタブ
	Brush row_pressed		= Color( 0.32f, 0.42f, 0.62f, 0.95f );	///< 押している行・入力待ちのキー割り当て
	Brush row_disabled;										///< 無効なボタンの背景（未指定なら row）
	Brush track				= Color( 0.06f, 0.07f, 0.1f, 1.f );		///< 値の欄・スライダーの溝・スクロールバーの溝
	Brush accent			= Color( 0.35f, 0.6f, 1.f, 1.f );		///< 強調色。下の用途別の面の代用
	Brush slider_fill;										///< スライダーの充填（未指定なら accent）。溝の形のまま、値の位置で切り取って描く
	Brush fill_disabled		= Color( 0.4f, 0.4f, 0.44f, 1.f );		///< 無効なスライダーの充填
	Brush slider_knob;										///< スライダーのつまみ（未指定なら描かない）
	Brush toggle_on;										///< オンのトグルの値の欄（未指定なら左端に accent の帯）
	Brush tab_badge;										///< タブの見出しの印（未指定なら accent。丸い画像を指定すると丸になる）
	Brush tab_indicator;									///< 選択中のタブの下線（未指定なら accent）
	Brush scroll_thumb;										///< スクロールバーのつまみ（未指定なら accent）
	Brush dialog_header;									///< ダイアログ上端の帯（未指定なら accent）

	// 文字の色
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
	float_t slider_knob_width	= 24.f;		///< スライダーのつまみの幅（高さは溝の高さの 2 倍）

	// 部品の中の比率・寸法（以前は各部品のコードに直接書かれていて、テーマファイルで変えられなかった）
	float_t value_box_inset			= 0.15f;	///< 値の欄の上下の余白（行の高さに対する比率。Toggle / Choice / KeyBind / TextField）
	float_t slider_track_width		= 0.75f;	///< スライダーの溝の幅（値の欄に対する比率。残りは数値の表示）
	float_t slider_track_height		= 0.4f;		///< スライダーの溝の高さ（値の欄に対する比率）
	float_t row_icon_ratio			= 0.6f;		///< 行のアイコンの大きさ（行の高さに対する比率）
	float_t tab_badge_size			= 10.f;		///< タブの見出しの印（丸）の大きさ
	float_t choice_arrow_width		= 0.25f;	///< 選択肢の左右の矢印の幅（値の欄に対する比率の上限）
	float_t accent_bar_thickness	= 6.f;		///< 選択中のタブの下線・ダイアログ上端の帯の太さ
	float_t toggle_mark_width		= 8.f;		///< オンのトグルの左端の帯の幅（toggle_on が未指定のとき）
	float_t scrollbar_width			= 10.f;		///< スクロールバーの幅
	float_t scrollbar_min_thumb		= 24.f;		///< スクロールバーのつまみの最小の長さ
	float_t menu_width				= 900.f;	///< 小さいメニュー（ポーズなど）のパネルの幅
	float_t section_spacing			= 24.f;		///< パネルの中の大きな区切り（タブとボタン行の間など）

	/// 行の高さを、文字の行の高さ（＋上下の余白）より小さくしない（文字を大きくしても行からはみ出さない）
	float_t row_text_padding		= 16.f;

	/// 内容が変わるたびに増える（開いている画面が配置し直すきっかけ。文字の大きさの変更など）
	uint_t revision = 0;

}; // struct Theme

/**
 * 文字の大きさの倍率を掛けたテーマを作る（アクセシビリティの「文字の大きさ」）
 *
 * フォントサイズだけを倍率で変える。行の高さは、部品が文字の行の高さに合わせて広げる。
 */
inline Theme scale_theme_text( const Theme& base, float_t text_scale )
{
	Theme t = base;
	t.font_size_title = base.font_size_title * text_scale;
	t.font_size_normal = base.font_size_normal * text_scale;
	t.font_size_small = base.font_size_small * text_scale;
	return t;
}

/**
 * テーマ定義（`キー = 値` の行）で Theme を上書きする
 *
 * @code
 * # 面は 単色 / 画像 のどちらでも書ける
 * panel.color       = #0F1219EB          # #RRGGBB / #RRGGBBAA / "r g b [a]"（0〜1）
 * panel             = #0F1219EB          # .color の省略形
 * panel.image       = ui/panel.png       # 画像名（media/texture/ からの相対。\ は / に直す）
 * panel.slice       = 24 24 24 24        # 9 分割の枠幅（左 上 右 下。1 つだけなら四辺同じ）
 * panel.slice_scale = 1
 * panel.source      = 0 0 64 64          # 切り出し範囲（x y 幅 高さ）
 * panel.tint        = 1 1 1 0.95
 * panel.edge        = round              # 9 分割の四辺の埋め方（stretch / repeat / round）
 * panel.center      = repeat             # 9 分割の中央の埋め方
 * text              = #EBEBEB            # 文字の色
 * row_height        = 72                 # 寸法
 * @endcode
 *
 * - 書かれた項目だけを上書きする。`panel.image` を書くと、色だけの面から画像の面に変わる（空にすると単色に戻る）。
 * - 面の属性はキーの最後の `.` で分ける（将来 `button.focused.image` のような階層の面名を足せるように）。
 * - `#` で始まる行と、値の後ろの「空白 + #」以降はコメント。値にエスケープ（`\n` など）は無い。
 * - 数値は有限の値だけを受け付け、項目ごとに範囲を検査する（寸法は 0 以上、フォントサイズと行の高さは正、比率は 0〜1、色の成分は 0〜1）。
 * - 解釈できない行は errors に「行番号: キー: 理由」を追加して読み飛ばす（ほかの行は反映する）。同じキーが 2 回あれば後勝ちで、警告を追加する。
 *
 * @return 反映した項目の数
 */
int load_theme( Theme& theme, const string_t& text, std::vector< string_t >* errors = nullptr );

/// ファイルから読み込んで上書きする。開けなければ false
bool load_theme_file( Theme& theme, const char_t* path, std::vector< string_t >* errors = nullptr );

/// テーマが使う画像をすべて先に読み込む（初めて画面を開いたフレームで読み込みが走って引っかからないように）
void preload_theme_images( const Theme& theme, const Renderer& renderer );

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

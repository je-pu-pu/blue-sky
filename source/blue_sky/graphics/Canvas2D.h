#pragma once

#include <type/type.h>

namespace core::graphics
{
	class Sprite;
	class Texture;
	struct TextStyle;
}

namespace blue_sky::graphics
{

/**
 * 2D の画像と文字を描く窓口（画面の大きさ・Sprite・文字を積んで描く・文字を測る・UI の画像を読む）
 *
 * UI の描画（ui::Renderer）が使う。GraphicsManager が実装する。
 * テストでは偽物（と偽の Sprite）を渡し、仮想座標から画面への写し方・文字と Sprite の切り替え・画像の読み込みを確かめる（設計上の問題 D3）
 */
class Canvas2D
{
public:
	virtual ~Canvas2D() = default;

	/// 画面の大きさ（ピクセル）
	virtual int get_screen_width() const = 0;
	virtual int get_screen_height() const = 0;

	/// 今の描画先の大きさ（ビューポート。UI を描くときに、描画先に合わせて写すのに使う）
	virtual float_t get_render_target_width() const = 0;
	virtual float_t get_render_target_height() const = 0;

	/// 2D の四角形を描く（begin 〜 end の間で描く）
	virtual core::graphics::Sprite* get_sprite() const = 0;

	/// 文字を積むだけで、まだ描かない（flush_text で描く）。続けて描く文字列を 1 回の描画にまとめる
	virtual void queue_text( float_t left, float_t top, float_t right, float_t bottom, const char_t* text, const core::graphics::TextStyle& style ) const = 0;

	/// 積んだ文字を描く
	virtual void flush_text() const = 0;

	/// 文字列（最初の改行まで）の描画幅（ピクセル）。font_size が 0 以下ならデフォルトサイズ。フォント未準備なら 0
	virtual float_t measure_text_width( const char_t* text, float_t font_size ) const = 0;

	/// 1 行の高さ（ピクセル）。font_size が 0 以下ならデフォルトサイズ
	virtual float_t measure_text_line_height( float_t font_size ) const = 0;

	/// TextStyle::font_size が 0 のときに使われるフォントサイズ（ピクセル）
	virtual float_t get_default_font_size() const = 0;

	/// UI 用の画像を名前付きで読む（乗算済みアルファ・ミップなし。読み込み済みならそれを返す）。失敗すると例外
	virtual core::graphics::Texture* load_named_texture_premultiplied( const char_t* name, const char_t* file_name ) = 0;
};

} // namespace blue_sky::graphics

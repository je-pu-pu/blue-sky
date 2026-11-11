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
private:
	bool right_to_left_ = false;
	float_t opacity_ = 1.f;

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

	/// 右から左へ書く言語か（行の始まり・終わりの配置を左右のどちらにするか。Screen が文字列テーブルから設定する）
	void set_right_to_left( bool rtl ) { right_to_left_ = rtl; }
	bool is_right_to_left() const { return right_to_left_; }

	/// 以降の描画の不透明度（0 〜 1。閉じている途中の層を薄くするのに Screen が設定する）。実装は色の α に掛ける
	void set_opacity( float_t opacity ) { opacity_ = opacity < 0.f ? 0.f : ( opacity > 1.f ? 1.f : opacity ); }
	float_t get_opacity() const { return opacity_; }

	/// 色に不透明度を掛ける（実装の描画関数で使う）
	Color apply_opacity( Color c ) const { c.a() *= opacity_; return c; }

	/**
	 * 積んである描画を描く（Screen::render の最後に呼ぶ）
	 *
	 * 実装は、呼ばれた順番を保ったまま、同じ種類の描画（Sprite どうし・文字どうし）が続く間はまとめて描いてよい。
	 * 以前は矩形 1 つ・文字列 1 つごとに描画の準備と後始末をしていた（設計上の問題 B15）。
	 */
	virtual void flush() { }

	/**
	 * 矩形の四辺を物理ピクセルの境界にそろえた矩形（仮想座標）を返す（既定はそのまま）
	 *
	 * 具象実装の draw_rect / draw_image は、受け取った矩形をこれと同じ規則で物理ピクセルに丸めること
	 * （そろえた矩形を渡せば、そのままのピクセルに描かれる）。
	 */
	virtual Rect snap_to_pixels( const Rect& rect ) const { return rect; }

}; // class Renderer

} // namespace core::ui

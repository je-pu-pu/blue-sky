#pragma once

#include <blue_sky/type.h>

namespace core::graphics
{
	class Texture;
	class Sprite;
}

namespace blue_sky::graphics
{
	class GraphicsManager;
}

namespace blue_sky::ui
{

/**
 * 2D 描画 API
 *
 * GraphicsManager の Sprite / DirectWrite を薄くラップし、
 * HUD やメニュー画面から共通で使えるシンプルなインターフェースを提供する。
 *
 * テキスト描画は 1 回の呼び出しで完結する（内部で 2D/3D コンテキストの切り替えを行う）。
 * スプライト描画は begin_sprite() / end_sprite() で囲んでバッチ処理する。
 */
class UIRenderer
{
public:
	using Texture			= core::graphics::Texture;
	using Sprite			= core::graphics::Sprite;
	using GraphicsManager	= blue_sky::graphics::GraphicsManager;

private:
	GraphicsManager* gm_;

public:
	explicit UIRenderer( GraphicsManager* );

	/// 画面サイズ
	int get_screen_width() const;
	int get_screen_height() const;

	/// テキスト描画（矩形内に左寄せ）
	void draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color );

	/// テキスト描画（矩形内に中央寄せ）
	void draw_text_center( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color );

	/// スプライトバッチの開始・終了
	Sprite* begin_sprite();
	void end_sprite();

	/// GraphicsManager への直接アクセス（必要な場合のみ）
	GraphicsManager* get_graphics_manager() { return gm_; }
	const GraphicsManager* get_graphics_manager() const { return gm_; }

}; // class UIRenderer

} // namespace blue_sky::ui

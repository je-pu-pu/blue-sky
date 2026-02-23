#pragma once

#include <blue_sky/type.h>
#include <core/graphics/TextStyle.h>

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
 * 2D 描画 API（仮想座標系）
 *
 * GraphicsManager の Sprite / DirectWrite を薄くラップし、
 * HUD やメニュー画面から共通で使えるシンプルなインターフェースを提供する。
 *
 * すべての座標は仮想座標系 (DESIGN_WIDTH x DESIGN_HEIGHT) で指定する。
 * 内部で物理ピクセルへのスケーリングを自動的に行うため、
 * UI コードは実際の画面解像度に依存しない。
 *
 * テキスト描画は 1 回の呼び出しで完結する（内部で 2D/3D コンテキストの切り替えを行う）。
 * スプライト描画は begin_sprite() / end_sprite() で囲んでバッチ処理する。
 */
class Renderer
{
public:
	using Texture			= core::graphics::Texture;
	using Sprite			= core::graphics::Sprite;
	using GraphicsManager	= blue_sky::graphics::GraphicsManager;

	/// 仮想座標系の基準解像度 (Full HD)
	static constexpr float_t DESIGN_WIDTH  = 1920.f;
	static constexpr float_t DESIGN_HEIGHT = 1080.f;

private:
	GraphicsManager* gm_;

	/// 仮想座標 → 物理ピクセルへのスケール係数
	float_t get_scale_x() const;
	float_t get_scale_y() const;

public:
	explicit Renderer( GraphicsManager* );

	/// 仮想画面サイズ（常に基準解像度を返す）
	float_t get_screen_width() const { return DESIGN_WIDTH; }
	float_t get_screen_height() const { return DESIGN_HEIGHT; }

	/// 物理画面サイズ
	int get_physical_width() const;
	int get_physical_height() const;

	/// 物理ピクセル座標 → 仮想座標への変換（マウス座標用）
	float_t physical_to_virtual_x( float_t px ) const;
	float_t physical_to_virtual_y( float_t py ) const;

	/// テキスト描画（矩形内に左寄せ）— 仮想座標
	void draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color );
	void draw_text( float_t x, float_t y, float_t w, float_t h, const char_t* text, const core::graphics::TextStyle& style );

	/// テキスト描画（矩形内に中央寄せ）— 仮想座標
	void draw_text_center( float_t x, float_t y, float_t w, float_t h, const char_t* text, const Color& color );
	void draw_text_center( float_t x, float_t y, float_t w, float_t h, const char_t* text, const core::graphics::TextStyle& style );

	/// 塗りつぶし矩形の描画 — 仮想座標
	void draw_rect( float_t x, float_t y, float_t w, float_t h, const Color& color );

	/// スプライトバッチの開始・終了
	Sprite* begin_sprite();
	void end_sprite();

	/// GraphicsManager への直接アクセス（必要な場合のみ）
	GraphicsManager* get_graphics_manager() { return gm_; }
	const GraphicsManager* get_graphics_manager() const { return gm_; }

}; // class Renderer

} // namespace blue_sky::ui

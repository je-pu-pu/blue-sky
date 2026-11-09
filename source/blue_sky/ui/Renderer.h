#pragma once

#include <blue_sky/type.h>
#include <core/ui/Renderer.h>
#include <core/ui/VirtualScreen.h>
#include <win/Rect.h>

#include <unordered_map>

namespace blue_sky::graphics
{
	class GraphicsManager;
}

namespace core::graphics
{
	class Texture;
}

namespace blue_sky::ui
{

/**
 * core::ui::Renderer の具象実装（GraphicsManager の Sprite / MSDF テキストを使う）
 *
 * 仮想座標系は短い辺を 1920x1080 に合わせ、長い辺は画面に合わせて伸ばす（VirtualScreen）。
 * 物理画面へは**等方スケール**（縦横同じ倍率）で写すので、16:9 以外の解像度でも UI が歪まず、
 * 21:9 でも画面の端まで使える（以前は 16:9 の箱を中央に置き、余りを余白にしていた）。
 */
class Renderer : public core::ui::Renderer
{
public:
	using GraphicsManager = blue_sky::graphics::GraphicsManager;
	using Rect = core::ui::Rect;

	/// 仮想座標系の基準解像度
	static constexpr float_t DESIGN_WIDTH  = 1920.f;
	static constexpr float_t DESIGN_HEIGHT = 1080.f;

	/// 画像名の基準ディレクトリ（画像名 "ui/panel.png" → "media/texture/ui/panel.png"）
	static constexpr const char_t* IMAGE_DIRECTORY = "media/texture/";

private:
	GraphicsManager* gm_;

	/// 今まとめて描いている種類（同じ種類が続く間は準備と後始末を省く。順番は呼ばれたとおり）
	enum class BatchMode { NONE, SPRITE, TEXT };
	BatchMode mode_ = BatchMode::NONE;

	/// Sprite を描く前に呼ぶ（文字を積んでいれば先に描く）
	void begin_sprite_mode();

	/// 文字を描く前に呼ぶ（Sprite を描いていれば後始末する）
	void begin_text_mode();

	/// 画像名 → テクスチャ（読み込みに失敗した画像は nullptr を覚えておく）。テクスチャ自体は GraphicsManager が持つ
	mutable std::unordered_map< string_t, const core::graphics::Texture* > images_;

	/// 今の画面の大きさに対する仮想画面
	core::ui::VirtualScreen get_virtual_screen() const;

	float_t get_scale() const;

	/// 描画のときの写し方（仮想座標 → 今の描画先のピクセル）。描画先が画面なら get_scale と同じでずれは 0
	core::ui::TargetMapping get_draw_mapping() const;

	Rect to_physical( const Rect& ) const;

	/// 仮想座標の矩形 → 物理ピクセルの矩形（各辺を四捨五入）
	win::Rect to_pixel_rect( const Rect& ) const;

	/// 画像名からテクスチャを引く（初回に読み込む）。読めなければ nullptr
	const core::graphics::Texture* find_image( const string_t& image ) const;

public:
	explicit Renderer( GraphicsManager* );

	float_t get_screen_width() const override { return get_virtual_screen().width; }
	float_t get_screen_height() const override { return get_virtual_screen().height; }

	/// 物理ピクセル座標 → 仮想座標（マウス座標用。範囲外はクランプしない）
	float_t physical_to_virtual_x( float_t px ) const;
	float_t physical_to_virtual_y( float_t py ) const;

	void draw_text( const Rect& rect, const char_t* text, const core::graphics::TextStyle& style ) override;
	void draw_rect( const Rect& rect, const Color& color ) override;
	void fill_screen( const Color& color ) override;

	float_t get_pixel_scale() const override;

	void flush() override;
	Rect snap_to_pixels( const Rect& rect ) const override;

	/// 画像名の正規化（\ → /、先頭の ./ を取る）。同じ画像を別名で二重に読まないように
	static string_t normalize_image_name( const string_t& image );

	bool get_image_size( const string_t& image, float_t& width, float_t& height ) const override;
	void draw_image( const Rect& rect, const string_t& image, const Rect& source, const Color& tint ) override;

	float_t measure_text_width( const char_t* text, float_t font_size ) const override;
	float_t measure_line_height( float_t font_size ) const override;

}; // class Renderer

} // namespace blue_sky::ui

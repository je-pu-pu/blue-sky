#pragma once

#include <core/type.h>
#include <core/graphics/Texture.h>

#include <win/Rect.h>

#include <common/chase_value.h>

#include <algorithm>


namespace core
{

/**
 * 背景レイヤー
 *
 * このクラスは、ひとつの背景テクスチャを保持し、移動・回転・拡大縮小の簡易アニメーション機能を備えている。
 */
class BgSpriteLayer
{
public:
	using Texture = core::graphics::Texture;
	using TextureHandle = core::graphics::TextureHandle;

private:
	const string_t name_;
	TextureHandle texture_;		///< 背景の画像（ハンドルで持ち、描く側が引く。以前は生ポインタで、解放の後に古いものを指し得た）

	win::Rect src_rect_;
	win::Rect dst_rect_;

	common::chase_value< Vector3 > translation_;
	common::chase_value< float_t > rotation_;
	common::chase_value< float_t > scale_;
	common::chase_value< Color > color_;

	BgSpriteLayer& operator = ( const BgSpriteLayer& ) { }

public:
	/// 背景を作った基準の画面の大きさ。この大きさの画面では、画像を画素の大きさのまま描く（13 年間の見た目）
	static constexpr float_t BASE_SCREEN_WIDTH = 1280.f;
	static constexpr float_t BASE_SCREEN_HEIGHT = 720.f;

	/**
	 * 画面を覆うための拡大（画像の画素の大きさに掛ける。ファイルに書いた拡大はその上に掛かる）
	 *
	 * 基準の画面（1280x720）で 1。縦横比が違う画面では、幅の比と高さの比の大きい方に合わせる（はみ出す方は切れ、足りない方は出ない）。
	 * 基準の画面で画面を覆っていた背景は、どの画面でも覆う
	 */
	static float_t get_screen_cover_scale( float_t screen_width, float_t screen_height )
	{
		return ( std::max )( screen_width / BASE_SCREEN_WIDTH, screen_height / BASE_SCREEN_HEIGHT );
	}

	/// texture は大きさを読むだけで持たない。持つのは handle（同じテクスチャのハンドル）
	BgSpriteLayer( const char_t* name, const Texture* texture, TextureHandle handle );
	~BgSpriteLayer();

	const string_t& get_name() const { return name_; }
	TextureHandle get_texture_handle() const { return texture_; }

	win::Rect& get_src_rect() { return src_rect_; }
	win::Rect& get_dst_rect() { return dst_rect_; }

	common::chase_value< Vector3 >& get_translation() { return translation_; }
	common::chase_value< float_t >& get_rotation() { return rotation_; }
	common::chase_value< float_t >& get_scale() { return scale_; }
	common::chase_value< Color >& get_color() { return color_; }

	void update();

}; // class BgSpriteLayer

} // namespace core

#pragma once

#include <core/type.h>
#include <core/graphics/Texture.h>

#include <win/Rect.h>

#include <common/chase_value.h>


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

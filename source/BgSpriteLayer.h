#ifndef BG_SPRITE_LAYTER_H
#define BG_SPRITE_LAYTER_H

#include "Direct3D11Texture.h"
#include "Direct3D11Color.h"
#include "vector3.h"
#include "type.h"
#include <win/Rect.h>
#include <common/chase_value.h>
#include <d3d11.h>

/**
 * 背景レイヤー
 *
 * このクラスは、ひとつの背景テクスチャを保持し、移動・回転・拡大縮小の簡易アニメーション機能を備えている。
 */
class BgSpriteLayer
{
public:
	typedef Direct3D11Texture Texture;
	typedef Direct3D11Color Color;

private:
	const string_t name_;
	const Texture* texture_;

	win::Rect src_rect_;
	win::Rect dst_rect_;

	common::chase_value< vector3 > translation_;
	common::chase_value< float_t > rotation_;
	common::chase_value< float_t > scale_;
	common::chase_value< Color > color_;

public:
	BgSpriteLayer( const char_t*, const Texture* );
	~BgSpriteLayer();

	const string_t& get_name() const { return name_; }
	const Texture* get_texture() const { return texture_; }

	win::Rect& get_src_rect() { return src_rect_; }
	win::Rect& get_dst_rect() { return dst_rect_; }

	common::chase_value< vector3 >& get_translation() { return translation_; }
	common::chase_value< float_t >& get_rotation() { return rotation_; }
	common::chase_value< float_t >& get_scale() { return scale_; }
	common::chase_value< Color >& get_color() { return color_; }

	void update();


}; // class BgSpriteLayer

#endif // BG_SPRITE_LAYTER_H
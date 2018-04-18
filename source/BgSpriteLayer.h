#ifndef BG_SPRITE_LAYTER_H
#define BG_SPRITE_LAYTER_H

#include <core/Vector3.h>

#include <core/graphics/Direct3D11/Direct3D11Color.h>

#include <win/Rect.h>

#include <common/chase_value.h>

#include <type/type.h>

namespace game
{

class Texture;

}

/**
 * 背景レイヤー
 *
 * このクラスは、ひとつの背景テクスチャを保持し、移動・回転・拡大縮小の簡易アニメーション機能を備えている。
 */
class BgSpriteLayer
{
public:
	typedef game::Texture Texture;
	typedef Direct3D11Color Color;

private:
	const string_t name_;
	const Texture* texture_;

	win::Rect src_rect_;
	win::Rect dst_rect_;

	common::chase_value< Vector3 > translation_;
	common::chase_value< float_t > rotation_;
	common::chase_value< float_t > scale_;
	common::chase_value< Color > color_;

	BgSpriteLayer& operator = ( const BgSpriteLayer& ) { }

public:
	BgSpriteLayer( const char_t*, const Texture* );
	~BgSpriteLayer();

	const string_t& get_name() const { return name_; }
	const Texture* get_texture() const { return texture_; }

	win::Rect& get_src_rect() { return src_rect_; }
	win::Rect& get_dst_rect() { return dst_rect_; }

	common::chase_value< Vector3 >& get_translation() { return translation_; }
	common::chase_value< float_t >& get_rotation() { return rotation_; }
	common::chase_value< float_t >& get_scale() { return scale_; }
	common::chase_value< Color >& get_color() { return color_; }

	void update();

}; // class BgSpriteLayer

#endif // BG_SPRITE_LAYTER_H
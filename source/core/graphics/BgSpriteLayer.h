#pragma once

#include <core/type.h>

#include <win/Rect.h>

#include <common/chase_value.h>


namespace game
{

class Texture;

}

namespace core
{

/**
 * �w�i���C���[
 *
 * ���̃N���X�́A�ЂƂ̔w�i�e�N�X�`����ێ����A�ړ��E��]�E�g��k���̊ȈՃA�j���[�V�����@�\������Ă���B
 */
class BgSpriteLayer
{
public:
	using Texture = game::Texture;

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

} // namespace core

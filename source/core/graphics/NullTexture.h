#pragma once

#include "Texture.h"

namespace core::graphics
{

/**
 * �������Ȃ��e�N�X�`��
 *
 */
class NullTexture : public Texture
{
public:
	NullTexture() { }
	virtual ~NullTexture() { }

	virtual uint_t get_width() const { return 0; };
	virtual uint_t get_height() const { return 0; };

	virtual uint_t get_multi_sample_count() const { return 0; };
	virtual uint_t get_multi_sample_quality() const { return 0; };

	void bind_to_ds( uint_t ) const { };
	void bind_to_ps( uint_t ) const { };

}; // NullTexture

} // namespace core::graphics

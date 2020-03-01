#pragma once

#include "ShaderResource.h"

namespace core::graphics
{

/**
 * ���ۃe�N�X�`���N���X
 *
 * ���ۂ̃e�N�X�`�����������m�ۂ��A�V�F�[�_�[���\�[�X�Ƃ��Ďg�p�ł���
 */
class Texture : public ShaderResource
{
public:
	Texture() { }
	virtual ~Texture() { }

	virtual uint_t get_width() const = 0;
	virtual uint_t get_height() const = 0;

	virtual uint_t get_multi_sample_count() const = 0;
	virtual uint_t get_multi_sample_quality() const = 0;

}; // Texture

} // namespace core::graphics

#pragma once

#include <type/type.h>

namespace core::graphics
{

/**
 * �V�F�[�_�[���痘�p�\�ȃe�N�X�`���̃C���^�[�t�F�C�X
 *
 */
class ShaderResource
{
public:
    virtual void bind_to_ds( uint_t slot ) const = 0;
	virtual void bind_to_ps( uint_t slot ) const = 0;

}; // ShaderResource

} // namespace core::graphics

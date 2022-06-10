#pragma once

#include <core/type.h>

namespace core::graphics
{

/**
 * �����_�����O���ʂ��������ގ����ł���I�u�W�F�N�g�̃C���^�[�t�F�C�X
 *
 */
class RenderTarget
{
public:
    RenderTarget() { }
    virtual ~RenderTarget() { }

    virtual void activate() = 0;
    virtual void clear( const Color& = Color::Black ) = 0;

}; // RenderTarget

} // namespace core::graphics

#pragma once

#include <type/type.h>

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

}; // RenderTarget

} // namespace core::graphics

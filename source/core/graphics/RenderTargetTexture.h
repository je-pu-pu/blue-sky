#pragma once

#include "RenderTarget.h"
#include "ShaderResource.h"

namespace core::graphics
{

/**
 * �����_�����O���ʂ��������ނ��Ƃ��ł��A���A�V�F�[�_�[���\�[�X�Ƃ��Ďg�p�ł���e�N�X�`�� 
 *
 */
class RenderTargetTexture : public RenderTarget, public ShaderResource
{
public:
	RenderTargetTexture() { }
	virtual ~RenderTargetTexture() { }

}; // RenderTargetTexture

} // namespace core::graphics

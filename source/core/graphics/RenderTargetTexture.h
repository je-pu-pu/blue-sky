#pragma once

#include "Texture.h"
#include "RenderTarget.h"

namespace core::graphics
{

/**
 * �����_�����O���ʂ��������ނ��Ƃ��ł��A���A�V�F�[�_�[���\�[�X�Ƃ��Ďg�p�ł���e�N�X�`�� 
 *
 */
class RenderTargetTexture : public Texture, public RenderTarget
{
public:
	RenderTargetTexture() { }
	virtual ~RenderTargetTexture() { }

}; // RenderTargetTexture

} // namespace core::graphics

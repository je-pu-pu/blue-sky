#pragma once

#include "Texture.h"
#include "PixelFormat.h"

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 �����_�����O���ʂ��������ގ����ł��� Texture
 *
 */
class RenderTargetTexture : public Texture
{
public:
	RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height );

}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11

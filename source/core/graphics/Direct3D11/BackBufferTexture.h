#pragma once

#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{
	/**
	 * Direct3D11 �̃o�b�N�o�b�t�@���Q�Ƃ���e�N�X�`��
	 *
	 */
	class BackBufferTexture : public RenderTargetTexture
	{
	public:
		BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain );

	};

} // namespace core::graphics::direct_3d_11

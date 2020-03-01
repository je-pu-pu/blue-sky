#pragma once

#include "Direct3D11Common.h"
#include <core/graphics/BackBufferTexture.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

	/**
	 * Direct3D11 �̃o�b�N�o�b�t�@���Q�Ƃ���e�N�X�`��
	 *
	 */
	class BackBufferTexture : public core::graphics::BackBufferTexture
	{
	private:
		// @todo Texture �Ƌ��ʉ�����
		Direct3D11*							direct_3d_;
		com_ptr< ID3D11Texture2D >			texture_2d_;				///< ���ۂ� 2D �e�N�X�`���f�[�^
		com_ptr< ID3D11RenderTargetView >	render_target_view_;		///< �V�F�[�_�[���痘�p���邽�߂� View
		D3D11_TEXTURE2D_DESC				texture_2d_desc_{};			///< 2D �e�N�X�`���ɂ��Ă̐���

	public:
		BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain );

		ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }

		uint_t get_multi_sample_count() const { return texture_2d_desc_.SampleDesc.Count; }
		uint_t get_multi_sample_quality() const { return texture_2d_desc_.SampleDesc.Quality; }

		void activate() override;
	};

} // namespace core::graphics::direct_3d_11

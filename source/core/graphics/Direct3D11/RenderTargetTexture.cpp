#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{
	/**
	 * �����_�����O���ʂ��������ގ����ł���e�N�X�`�����쐬����
	 *
	 * @todo MipLevels, SampleDesc ���ςɂ���
	 *
	 * @param d3d Direct3D11
	 * @param Format �쐬����e�N�X�`���̃t�H�[�}�b�g
	 * @param width �쐬����e�N�X�`���̕�
	 * @param height �쐬����e�N�X�`���̍���
	 */
	RenderTargetTexture::RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height )
		: direct_3d_( d3d )
		, texture_( d3d, format, width, height, true )
	{
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_.get_texture_2d(), nullptr, & render_target_view_ ) );
	}

	void RenderTargetTexture::activate()
	{
		direct_3d_->set_render_target( this );
	}

} // namespace core::graphics::direct_3d_11

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
		: Texture( d3d )
	{
		create_texture_2d( format, width, height, true );
		create_shader_resource_view( format );
		create_render_target_view();
	}	

	/**
	 * �����_�����O���ʂ��������ނ��߂̃r���[���쐬����
	 *
	 */
	void RenderTargetTexture::create_render_target_view()
	{
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_2d_.get(), nullptr, & render_target_view_ ) );
	}

	/*
	void RenderTargetTexture::activate()
	{
		direct_3d_->set_render_target( this );
	}
	
	void RenderTargetTexture::clear( const game::Color& color )
	{
		direct_3d_->clear_back_buffer_view( )
	}
	*/

} // namespace core::graphics::direct_3d_11

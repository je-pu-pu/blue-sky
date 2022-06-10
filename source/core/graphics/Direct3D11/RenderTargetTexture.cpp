#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{

/**
* �����_�����O���ʂ��������ގ����ł���e�N�X�`�����쐬����
* �쐬�����e�N�X�`���͉�ʂ̕� * ��ʂ̍����̃T�C�Y�ɂȂ�
* 
* 
* @todo MipLevels, SampleDesc ���ςɂ���
*
* @param Format �쐬����e�N�X�`���̃t�H�[�}�b�g
*/
RenderTargetTexture::RenderTargetTexture( PixelFormat format )
	: texture_(
		Direct3D11::get_instance(),
		format,
		Direct3D11::get_instance()->get_width(),
		Direct3D11::get_instance()->get_height(),
		true
	)
{
	DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateRenderTargetView( texture_.get_texture_2d(), nullptr, & render_target_view_ ) );
}

/**
	* �����_�����O���ʂ��������ގ����ł���e�N�X�`�����쐬����
	*
	* @todo MipLevels, SampleDesc ���ςɂ���
	*
	* @param Format �쐬����e�N�X�`���̃t�H�[�}�b�g
	* @param width �쐬����e�N�X�`���̕�
	* @param height �쐬����e�N�X�`���̍���
	*/
RenderTargetTexture::RenderTargetTexture( PixelFormat format, int width, int height )
	: texture_( Direct3D11::get_instance(), format, width, height, true )
{
	DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateRenderTargetView( texture_.get_texture_2d(), nullptr, & render_target_view_ ) );
}

void RenderTargetTexture::activate()
{
	Direct3D11::get_instance()->set_render_target( this );
}

void RenderTargetTexture::clear( const Color& color )
{
	Direct3D11::get_instance()->clear_render_target_view( get_render_target_view(), color );
}

} // namespace core::graphics::direct_3d_11

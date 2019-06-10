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
	{
		direct_3d_ = d3d;

		{
			texture_2d_desc_.Width = width;
			texture_2d_desc_.Height = height;
			texture_2d_desc_.MipLevels = 1;
			texture_2d_desc_.ArraySize = 1;
			texture_2d_desc_.Format = static_cast< DXGI_FORMAT >(format);
			texture_2d_desc_.SampleDesc.Count = 1;
			texture_2d_desc_.SampleDesc.Quality = 0;
			texture_2d_desc_.Usage = D3D11_USAGE_DEFAULT;
			texture_2d_desc_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			texture_2d_desc_.CPUAccessFlags = 0;
			texture_2d_desc_.MiscFlags = 0;

			DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateTexture2D( & texture_2d_desc_, 0, & texture_2d_ ) );
		}

		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};

			view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			view_desc.ViewDimension = texture_2d_desc_.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipLevels = 1;
			view_desc.Texture2D.MostDetailedMip = 0;

			DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( texture_2d_.get(), & view_desc, & shader_resource_view_ ) );
		}
	}	

} // namespace core::graphics::direct_3d_11

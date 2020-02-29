#include "Texture.h"
#include "BackBufferTexture.h"

namespace core::graphics::direct_3d_11
{
	void Texture::create_texture_2d( PixelFormat format, int width, int height, bool multi_sample )
	{
		texture_2d_desc_.Width = width;
		texture_2d_desc_.Height = height;
		texture_2d_desc_.MipLevels = 1;
		texture_2d_desc_.ArraySize = 1;
		texture_2d_desc_.Format = static_cast< DXGI_FORMAT >( format );

		if ( multi_sample )
		{
			texture_2d_desc_.SampleDesc.Count = direct_3d_->get_back_buffer_texture()->get_multi_sample_count();
			texture_2d_desc_.SampleDesc.Quality = direct_3d_->get_back_buffer_texture()->get_multi_sample_quality();
		}
		else
		{
			texture_2d_desc_.SampleDesc.Count = 1;
			texture_2d_desc_.SampleDesc.Quality = 0;
		}
		
		texture_2d_desc_.Usage = D3D11_USAGE_DEFAULT;
		texture_2d_desc_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texture_2d_desc_.CPUAccessFlags = 0;
		texture_2d_desc_.MiscFlags = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateTexture2D( & texture_2d_desc_, 0, & texture_2d_ ) );
	}

	void Texture::create_shader_resource_view( PixelFormat format )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};

		view_desc.Format = static_cast< DXGI_FORMAT >( format );
		view_desc.ViewDimension = texture_2d_desc_.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = 1;
		view_desc.Texture2D.MostDetailedMip = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( texture_2d_.get(), & view_desc, & shader_resource_view_ ) );
	}
	
} // namespace core::graphics::direct_3d_11

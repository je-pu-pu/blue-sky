#include "Texture.h"
#include "BackBufferTexture.h"

namespace core::graphics::direct_3d_11
{
	void Texture::create_texture_2d( PixelFormat format, int width, int height, bool multi_sample )
	{
		if ( multi_sample )
		{
			create_texture_2d( format, width, height, direct_3d_->get_back_buffer_texture()->get_multi_sample_count(), direct_3d_->get_back_buffer_texture()->get_multi_sample_quality(), 1 );
		}
		else
		{
			create_texture_2d( format, width, height, 1, 0, 1 );
		}
	}

	void Texture::create_texture_2d( PixelFormat format, int width, int height, UINT sample_count, UINT sample_quality, UINT mip_levels )
	{
		texture_2d_desc_.Width = width;
		texture_2d_desc_.Height = height;
		texture_2d_desc_.MipLevels = mip_levels;
		texture_2d_desc_.ArraySize = 1;
		texture_2d_desc_.Format = to_dxgi_format( format );

		texture_2d_desc_.SampleDesc.Count = sample_count;
		texture_2d_desc_.SampleDesc.Quality = sample_quality;

		texture_2d_desc_.Usage = D3D11_USAGE_DEFAULT;
		texture_2d_desc_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texture_2d_desc_.CPUAccessFlags = 0;
		texture_2d_desc_.MiscFlags = mip_levels > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;	// ミップを GenerateMips で作れるようにする

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateTexture2D( & texture_2d_desc_, 0, & texture_2d_ ) );
	}

	void Texture::create_shader_resource_view( PixelFormat format )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};

		view_desc.Format = to_dxgi_format( format );
		view_desc.ViewDimension = texture_2d_desc_.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = texture_2d_desc_.MipLevels;	// 作ったミップをすべて読めるようにする（以前は常に 1 段）
		view_desc.Texture2D.MostDetailedMip = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( texture_2d_.get(), & view_desc, & shader_resource_view_ ) );
	}
	
} // namespace core::graphics::direct_3d_11

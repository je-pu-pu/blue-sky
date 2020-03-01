#include "BackBufferTexture.h"
#include "Direct3D11.h"

namespace core::graphics::direct_3d_11
{
	BackBufferTexture::BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain )
	{
		DIRECT_X_FAIL_CHECK( swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & texture_2d_ ) ) );
		texture_2d_->GetDesc( & texture_2d_desc_ );

		DIRECT_X_FAIL_CHECK( d3d->getDevice()->CreateRenderTargetView( texture_2d_.get(), nullptr, & render_target_view_ ) );
	}

} // namespace core::graphics::direct_3d_11

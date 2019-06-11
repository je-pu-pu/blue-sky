#include "BackBufferTexture.h"

namespace core::graphics::direct_3d_11
{
	BackBufferTexture::BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain )
		: RenderTargetTexture( d3d )
	{
		DIRECT_X_FAIL_CHECK( swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & texture_2d_ ) ) );
		texture_2d_->GetDesc( & texture_2d_desc_ );

		create_render_target_view();
	}

} // namespace core::graphics::direct_3d_11

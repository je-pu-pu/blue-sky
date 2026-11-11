#include "BackBufferTexture.h"
#include "Direct3D11.h"

namespace core::graphics::direct_3d_11
{
	BackBufferTexture::BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain, const DXGI_SAMPLE_DESC& sample )
		: direct_3d_( d3d )
	{
		DIRECT_X_FAIL_CHECK( swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & swap_chain_buffer_ ) ) );

		if ( sample.Count > 1 )
		{
			// MSAA の描画先を自前で作る（以前はスワップチェーン自体を MSAA にしていて、フリップモデルを使えなかった）
			D3D11_TEXTURE2D_DESC desc;
			swap_chain_buffer_->GetDesc( & desc );
			desc.SampleDesc = sample;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			desc.MiscFlags = 0;

			DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateTexture2D( & desc, nullptr, & texture_2d_ ) );
		}
		else
		{
			swap_chain_buffer_->AddRef();		// reset は参照を受け取る（AddRef しない）ので、2 つ目の持ち主の分を足す
			texture_2d_.reset( swap_chain_buffer_.get() );
		}

		texture_2d_->GetDesc( & texture_2d_desc_ );

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_2d_.get(), nullptr, & render_target_view_ ) );
	}

	void BackBufferTexture::resolve( ID3D11DeviceContext* context )
	{
		if ( texture_2d_.get() != swap_chain_buffer_.get() )
		{
			context->ResolveSubresource( swap_chain_buffer_.get(), 0, texture_2d_.get(), 0, texture_2d_desc_.Format );
		}
	}

	void BackBufferTexture::activate()
	{
		direct_3d_->set_render_target( this );
	}

	void BackBufferTexture::clear( const Color& color )
	{
		direct_3d_->clear_render_target_view( get_render_target_view(), color );
	}

} // namespace core::graphics::direct_3d_11

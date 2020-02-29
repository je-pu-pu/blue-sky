#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{
	/**
	 * レンダリング結果を書き込む事ができるテクスチャを作成する
	 *
	 * @todo MipLevels, SampleDesc を可変にする
	 *
	 * @param d3d Direct3D11
	 * @param Format 作成するテクスチャのフォーマット
	 * @param width 作成するテクスチャの幅
	 * @param height 作成するテクスチャの高さ
	 */
	RenderTargetTexture::RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height )
		: Texture( d3d )
	{
		create_texture_2d( format, width, height, true );
		create_shader_resource_view( format );
		create_render_target_view();
	}	

	/**
	 * レンダリング結果を書き込むためのビューを作成する
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

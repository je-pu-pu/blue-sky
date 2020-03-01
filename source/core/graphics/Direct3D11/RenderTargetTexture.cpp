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

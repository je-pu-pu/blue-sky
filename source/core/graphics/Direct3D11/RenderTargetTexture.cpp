#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{

/**
* レンダリング結果を書き込む事ができるテクスチャを作成する
* 作成されるテクスチャは画面の幅 * 画面の高さのサイズになる
* 
* 
* @todo MipLevels, SampleDesc を可変にする
*
* @param Format 作成するテクスチャのフォーマット
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
	* レンダリング結果を書き込む事ができるテクスチャを作成する
	*
	* @todo MipLevels, SampleDesc を可変にする
	*
	* @param Format 作成するテクスチャのフォーマット
	* @param width 作成するテクスチャの幅
	* @param height 作成するテクスチャの高さ
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

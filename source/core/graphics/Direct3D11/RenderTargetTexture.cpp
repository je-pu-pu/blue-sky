#include "RenderTargetTexture.h"

namespace core::graphics::direct_3d_11
{

using D3D11Texture = core::graphics::direct_3d_11::Texture;

/**
* レンダリング結果を書き込む事ができるテクスチャを作成する
* 作成されるテクスチャは画面の幅 * 画面の高さのサイズになる
*
*
* @todo MipLevels, SampleDesc を可変にする
*
* @param format 作成するテクスチャのフォーマット
*/
RenderTargetTexture::RenderTargetTexture( PixelFormat format )
	: format_( format )
	, is_screen_sized_( true )
	, texture_( std::make_unique< D3D11Texture >(
		Direct3D11::get_instance(),
		format,
		Direct3D11::get_instance()->get_width(),
		Direct3D11::get_instance()->get_height(),
		true
	) )
{
	DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

/**
	* レンダリング結果を書き込む事ができるテクスチャを作成する
	*
	* @todo MipLevels, SampleDesc を可変にする
	*
	* @param format 作成するテクスチャのフォーマット
	* @param width 作成するテクスチャの幅
	* @param height 作成するテクスチャの高さ
	*/
RenderTargetTexture::RenderTargetTexture( PixelFormat format, int width, int height )
	: format_( format )
	, is_screen_sized_( false )
	, texture_( std::make_unique< D3D11Texture >( Direct3D11::get_instance(), format, width, height, true ) )
{
	DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

void RenderTargetTexture::recreate_if_needed()
{
	if ( ! is_screen_sized_ ) return;

	auto* d3d = Direct3D11::get_instance();

	if ( texture_->get_width() == static_cast< uint_t >( d3d->get_width() )
	  && texture_->get_height() == static_cast< uint_t >( d3d->get_height() ) ) return;

	texture_ = std::make_unique< D3D11Texture >( d3d, format_, d3d->get_width(), d3d->get_height(), true );

	render_target_view_.reset();
	DIRECT_X_FAIL_CHECK( d3d->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

void RenderTargetTexture::activate()
{
	recreate_if_needed();
	Direct3D11::get_instance()->set_render_target( this );
}

void RenderTargetTexture::clear( const Color& color )
{
	recreate_if_needed();
	Direct3D11::get_instance()->clear_render_target_view( get_render_target_view(), color );
}

} // namespace core::graphics::direct_3d_11

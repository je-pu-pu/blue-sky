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
* @param direct_3d テクスチャを作る先
* @param format 作成するテクスチャのフォーマット
*/
RenderTargetTexture::RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format )
	: direct_3d_( direct_3d )
	, format_( format )
	, is_screen_sized_( true )
	, texture_( std::make_unique< D3D11Texture >(
		direct_3d,
		format,
		direct_3d->get_width(),
		direct_3d->get_height(),
		true
	) )
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

/**
	* レンダリング結果を書き込む事ができるテクスチャを作成する
	*
	* @todo MipLevels, SampleDesc を可変にする
	*
	* @param direct_3d テクスチャを作る先
	* @param format 作成するテクスチャのフォーマット
	* @param width 作成するテクスチャの幅
	* @param height 作成するテクスチャの高さ
	*/
RenderTargetTexture::RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format, int width, int height )
	: direct_3d_( direct_3d )
	, format_( format )
	, is_screen_sized_( false )
	, texture_( std::make_unique< D3D11Texture >( direct_3d, format, width, height, true ) )
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

void RenderTargetTexture::recreate_if_needed()
{
	if ( ! is_screen_sized_ ) return;

	auto* d3d = direct_3d_;

	if ( texture_->get_width() == static_cast< uint_t >( d3d->get_width() )
	  && texture_->get_height() == static_cast< uint_t >( d3d->get_height() ) ) return;

	texture_ = std::make_unique< D3D11Texture >( d3d, format_, d3d->get_width(), d3d->get_height(), true );

	render_target_view_.reset();
	DIRECT_X_FAIL_CHECK( d3d->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

core::graphics::Texture* RenderTargetTexture::resolve_for_sampling()
{
	if ( texture_->get_multi_sample_count() <= 1 )
	{
		return this;
	}

	auto* d3d = direct_3d_;

	if ( ! resolved_ || resolved_->get_width() != texture_->get_width() || resolved_->get_height() != texture_->get_height() )
	{
		resolved_ = std::make_unique< D3D11Texture >( d3d, format_, texture_->get_width(), texture_->get_height(), false );
	}

	d3d->getImmediateContext()->ResolveSubresource( resolved_->get_texture_2d(), 0, texture_->get_texture_2d(), 0, to_dxgi_format( format_ ) );

	return resolved_.get();
}

void RenderTargetTexture::activate()
{
	recreate_if_needed();
	direct_3d_->set_render_target( this );
}

void RenderTargetTexture::clear( const Color& color )
{
	recreate_if_needed();
	direct_3d_->clear_render_target_view( get_render_target_view(), color );
}

} // namespace core::graphics::direct_3d_11

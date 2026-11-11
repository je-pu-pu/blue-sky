#include "RenderTargetTexture.h"
#include "BackBufferTexture.h"

namespace core::graphics::direct_3d_11
{

using D3D11Texture = core::graphics::direct_3d_11::Texture;

/**
* レンダリング結果を書き込む事ができるテクスチャを作成する
* 作成されるテクスチャは画面の幅 * 画面の高さのサイズになる（ミップ 1 段・画面と同じ MSAA）
*
* @param direct_3d テクスチャを作る先
* @param format 作成するテクスチャのフォーマット
*/
RenderTargetTexture::RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format )
	: RenderTargetTexture( direct_3d, format, Options() )
{

}

/**
* ミップの段数・MSAA を指定して、レンダリング結果を書き込む事ができるテクスチャを作成する
* 作成されるテクスチャは画面の幅 * 画面の高さのサイズになる
*
* @param direct_3d テクスチャを作る先
* @param format 作成するテクスチャのフォーマット
* @param options ミップの段数・MSAA（作れない組み合わせは直す。RenderTargetTextureOptions::normalized）
*/
RenderTargetTexture::RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format, const Options& options )
	: direct_3d_( direct_3d )
	, format_( format )
	, options_( options.normalized( direct_3d->get_back_buffer_texture()->get_multi_sample_count() ) )
	, is_screen_sized_( true )
{
	create_texture( direct_3d_->get_width(), direct_3d_->get_height() );
}

/**
	* レンダリング結果を書き込む事ができるテクスチャを作成する（ミップ 1 段・画面と同じ MSAA）
	*
	* @param direct_3d テクスチャを作る先
	* @param format 作成するテクスチャのフォーマット
	* @param width 作成するテクスチャの幅
	* @param height 作成するテクスチャの高さ
	*/
RenderTargetTexture::RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format, int width, int height )
	: direct_3d_( direct_3d )
	, format_( format )
	, options_( Options().normalized( direct_3d->get_back_buffer_texture()->get_multi_sample_count() ) )
	, is_screen_sized_( false )
{
	create_texture( width, height );
}

/**
 * 指定の大きさでテクスチャと描画先のビューを作る（作り直しにも使う）
 */
void RenderTargetTexture::create_texture( int width, int height )
{
	// 画面と同じ MSAA（既定）は、バックバッファのサンプル数・品質を使う（指定できるようにする前と同じ）
	const bool same_as_screen = options_.sample_count == Options::SAMPLE_COUNT_SAME_AS_SCREEN;
	const UINT sample_count = same_as_screen ? direct_3d_->get_back_buffer_texture()->get_multi_sample_count() : options_.sample_count;
	const UINT sample_quality = same_as_screen ? direct_3d_->get_back_buffer_texture()->get_multi_sample_quality() : options_.sample_quality;

	texture_ = std::make_unique< D3D11Texture >( direct_3d_, format_, width, height, sample_count, sample_quality, options_.mip_levels );

	render_target_view_.reset();
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_->get_texture_2d(), nullptr, & render_target_view_ ) );
}

void RenderTargetTexture::recreate_if_needed()
{
	if ( ! is_screen_sized_ ) return;

	auto* d3d = direct_3d_;

	if ( texture_->get_width() == static_cast< uint_t >( d3d->get_width() )
	  && texture_->get_height() == static_cast< uint_t >( d3d->get_height() ) ) return;

	create_texture( d3d->get_width(), d3d->get_height() );
}

core::graphics::Texture* RenderTargetTexture::resolve_for_sampling()
{
	if ( texture_->get_multi_sample_count() <= 1 )
	{
		// ミップがあるなら、描いた結果からミップを作ってから読む
		texture_->generate_mips();

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

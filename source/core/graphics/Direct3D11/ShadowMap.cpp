#include "ShadowMap.h"
#include "Direct3D11.h"

#include <core/graphics/Direct3D11/ShaderResource.h>

#include <common/exception.h>

namespace core::graphics::direct_3d_11
{

ShadowMap::Vector				ShadowMap::light_position_;
ShadowMap::Matrix				ShadowMap::view_matrix_;
ShadowMap::MatrixList			ShadowMap::projection_matrix_list_;

static const float light_y = 500.f;

ShadowMap::ShadowMap( Direct3D11* direct_3d, uint_t cascade_levels, size_t size )
	: direct_3d_( direct_3d )
	, shader_resource_( new ShaderResource( direct_3d ) )
	, cascade_levels_( cascade_levels )
	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )
{
	assert( cascade_levels_ <= MaxCascadeLevels );

	{
		D3D11_TEXTURE2D_DESC texture_desc = { 0 };

		texture_desc.Width = size * cascade_levels_;
		texture_desc.Height = size;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R32_TYPELESS;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.SampleDesc.Quality = 0;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateTexture2D( & texture_desc, 0, & depth_stencil_texture_ ) );
	}

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = { DXGI_FORMAT_D32_FLOAT };
	
		depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateDepthStencilView( depth_stencil_texture_, & depth_stencil_view_desc, & depth_stencil_view_ ) );
	}

	ID3D11ShaderResourceView* view = 0;

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { DXGI_FORMAT_R32_FLOAT };

		view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = static_cast< UINT >( -1 );
		view_desc.Texture2D.MostDetailedMip = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( depth_stencil_texture_, & view_desc, & view ) );
	}

	{
		texture_.reset( new Direct3D11Texture( direct_3d_, view ) );
	}

	viewport_list_.resize( cascade_levels_ );

	int n = 0;

	for ( auto i = viewport_list_.begin(); i != viewport_list_.end(); ++i )
	{
		i->TopLeftX = static_cast< float >( size * n );
		i->TopLeftY = 0.f;
		i->Width = static_cast< float >( size );
		i->Height = static_cast< float >( size );
		i->MinDepth = 0.f;
		i->MaxDepth = 1.f;

		n++;
	}

	set_light_position( Vector( 50.f, 100.f, -25.f, 0.f ) );
}

ShadowMap::~ShadowMap()
{
	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );
}

/**
 * 光源の座標を設定する
 *
 * @param pos 光源の座標 ( カメラの座標からの相対座標 )
 */
void ShadowMap::set_light_position( const Vector& pos )
{
	light_position_ = pos;
	light_position_ *= 100.f / light_position_.y(); /// 高さが 100.f になるように調整

	/// @todo 動的に変更できるようにする
	float_t width[ MaxCascadeLevels ] = { 10.f, 50.f, 150.f, 9999.f };
	// float_t depth[ MaxCascadeLevels + 1 ] = { 0.1f, 5.f, 50.f, 150.f, 300.f };

	for ( int n = 0; n < MaxCascadeLevels; n++ )
	{
		/// @todo 地面より手前に far clip がこないようにする
		projection_matrix_list_[ n ].set_orthographic( width[ n ], width[ n ], 1.f, 500.f );
		shader_resource_->data().view_depth_per_cascade_level[ n ] = width[ n ] * 0.3f;
	}
}

/**
 * カメラの座標を設定する
 *
 * @param pos カメラの座標
 */
void ShadowMap::set_eye_position( const Vector& eye )
{
	Vector at( eye.x(), eye.y(), eye.z(), 1.f );
	Vector up( 0.f, 0.f, 1.f, 0.f );

	view_matrix_.set_look_at( light_position_, at, up );
}

/**
 * シャドウマップを描画するための準備をする
 *
 */
void ShadowMap::ready_to_render_shadow_map()
{
	ID3D11ShaderResourceView* shader_resource_view[] = { 0 };
	direct_3d_->getImmediateContext()->PSSetShaderResources( shader_resource_view_slot_, 1, shader_resource_view );

	direct_3d_->getImmediateContext()->ClearDepthStencilView( depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.f, 0 );

	ID3D11RenderTargetView* render_target_view[] = { 0 };
	direct_3d_->getImmediateContext()->OMSetRenderTargets( 1, render_target_view, depth_stencil_view_ );
}

/**
 * 指定したカスケードレベルのシャドウマップを描画するための準備をする
 *
 * @param level カスケードレベル
 */
void ShadowMap::ready_to_render_shadow_map_with_cascade_level( uint_t level )
{
	shader_resource_->data().shadow_view_projection[ level ] = get_view_projection_matrix( level ).transpose();
	shader_resource_->data().shadow_view_projection[ 0     ] = shader_resource_->data().shadow_view_projection[ level ];

	shader_resource_->update();
	shader_resource_->bind_to_vs();

	direct_3d_->getImmediateContext()->RSSetViewports( 1, & viewport_list_[ level ] );
}

/**
 * シーンを描画するための準備をする
 *
 */
void ShadowMap::ready_to_render_scene()
{
	texture_->bind_to_ps( shader_resource_view_slot_ );

	for ( int n = 0; n < cascade_levels_; n++ )
	{
		shader_resource_->data().shadow_view_projection[ n ] = get_view_projection_matrix( n ).transpose();
	}

	shader_resource_->update();
	shader_resource_->bind_to_vs();
	shader_resource_->bind_to_ps();
}

} // namespace core::graphics::direct_3d_11

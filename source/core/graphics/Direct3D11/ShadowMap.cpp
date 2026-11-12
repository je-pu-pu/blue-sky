#include "ShadowMap.h"
#include "Direct3D11.h"

#include <core/graphics/Direct3D11/DynamicSlotConstantBuffer.h>

#include <common/exception.h>

namespace core::graphics::direct_3d_11
{

static const float light_y = 500.f;

ShadowMap::ShadowMap( Direct3D11* direct_3d, uint_t cascade_levels, size_t size )
	: direct_3d_( direct_3d )
	, shader_resource_( create_typed_constant_buffer< ConstantBuffer >( direct_3d ) )
	, cascade_levels_( cascade_levels )
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

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateDepthStencilView( depth_stencil_texture_.get(), & depth_stencil_view_desc, & depth_stencil_view_ ) );
	}

	ID3D11ShaderResourceView* view = 0;

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { DXGI_FORMAT_R32_FLOAT };

		view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = static_cast< UINT >( -1 );
		view_desc.Texture2D.MostDetailedMip = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( depth_stencil_texture_.get(), & view_desc, & view ) );
	}

	{
		texture_.reset( new direct_3d_11::Texture( direct_3d_, view ) );
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
	// 深度ステンシルは com_ptr が解放する（以前は生ポインタで、コンストラクタの途中で例外が出るとリークしていた）
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

	update_projection();
}

/**
 * 光から見た投影行列を、今の設定（near / far clip・カスケードの幅）で作り直す
 *
 * 設定は抽象 ShadowMap の set_clip / set_cascade_width で変えられる（以前はここに固定値で書かれていた）
 */
void ShadowMap::update_projection()
{
	for ( int n = 0; n < MaxCascadeLevels; n++ )
	{
		const float_t width = get_cascade_width( n );

		projection_matrix_list_[ n ].set_orthographic( width, width, get_near_clip(), get_far_clip() );
		shader_resource_->data().view_depth_per_cascade_level[ n ] = get_view_depth_for_cascade( n );
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

	direct_3d_->getImmediateContext()->ClearDepthStencilView( depth_stencil_view_.get(), D3D11_CLEAR_DEPTH, 1.f, 0 );

	ID3D11RenderTargetView* render_target_view[] = { 0 };
	direct_3d_->getImmediateContext()->OMSetRenderTargets( 1, render_target_view, depth_stencil_view_.get() );
}

/**
 * 指定したカスケードレベルのシャドウマップを描画するための準備をする
 *
 * @param level カスケードレベル
 */
void ShadowMap::ready_to_render_shadow_map_with_cascade_level( uint_t level )
{
	shader_resource_->data().shadow_view_projection[ level ] = get_view_projection_matrix( level );
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
	for ( int n = 0; n < cascade_levels_; n++ )
	{
		shader_resource_->data().shadow_view_projection[ n ] = get_view_projection_matrix( n );
	}

	shader_resource_->update();
}

void ShadowMap::bind_to_render_scene() const
{
	texture_->bind_to_ps( shader_resource_view_slot_ );

	shader_resource_->bind_to_vs();
	shader_resource_->bind_to_ps();
}

} // namespace core::graphics::direct_3d_11

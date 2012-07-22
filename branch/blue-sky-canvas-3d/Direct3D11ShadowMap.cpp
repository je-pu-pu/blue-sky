#include "Direct3D11ShadowMap.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>

#include <xnamath.h>

XMVECTOR Direct3D11ShadowMap::light_position_;
XMMATRIX Direct3D11ShadowMap::view_matrix_;
XMMATRIX Direct3D11ShadowMap::projection_matrix_;

static const float light_y = 500.f;

Direct3D11ShadowMap::Direct3D11ShadowMap( Direct3D11* direct_3d, size_t size )
	: direct_3d_( direct_3d )
	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )
	, shader_resource_view_( 0 )
{
	{
		D3D11_TEXTURE2D_DESC texture_desc = { 0 };

		texture_desc.Width = size;
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

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { DXGI_FORMAT_R32_FLOAT };

		view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = -1;
		view_desc.Texture2D.MostDetailedMip = 0;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( depth_stencil_texture_, & view_desc, & shader_resource_view_ ) );
	}

	{
		viewport_.TopLeftX = 0.f;
		viewport_.TopLeftY = 0.f;
		viewport_.Width = static_cast< float >( size );
		viewport_.Height = static_cast< float >( size );
		viewport_.MinDepth = 0.f;
		viewport_.MaxDepth = 1.f;
	}

	light_position_ = XMVectorSet( 50.f, 350.f, -25.f, 0.f );
}

Direct3D11ShadowMap::~Direct3D11ShadowMap()
{
	DIRECT_X_RELEASE( shader_resource_view_ );
	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );
}

void Direct3D11ShadowMap::setEyePosition( const XMVECTOR& eye )
{
	XMVECTOR eye_fix = XMVectorSet( XMVectorGetX( eye ) + XMVectorGetX( light_position_ ), XMVectorGetY( light_position_ ), XMVectorGetZ( eye ) + XMVectorGetZ( light_position_ ), 0.f );
	XMVECTOR at = XMVectorSet( XMVectorGetX( eye ), XMVectorGetY( eye ), XMVectorGetZ( eye ), 0.f );
	XMVECTOR up = XMVectorSet( 0.f, 0.f, 1.f, 0.f );

	view_matrix_ = XMMatrixLookAtLH( eye_fix, at, up );
	projection_matrix_ = XMMatrixOrthographicLH( 10.f, 10.f, 50.f, 400.f );
}

void Direct3D11ShadowMap::render()
{
	direct_3d_->getImmediateContext()->ClearDepthStencilView( depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.f, 0 );
	
	ID3D11RenderTargetView* render_target_view[] = { 0 };
	direct_3d_->getImmediateContext()->OMSetRenderTargets( 1, render_target_view, depth_stencil_view_ );
	direct_3d_->getImmediateContext()->RSSetViewports( 1, & viewport_ );
}
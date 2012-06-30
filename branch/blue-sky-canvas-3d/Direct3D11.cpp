#include "Direct3D11.h"
#include "DirectX.h"

#include "include/d3dx11effect.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <xnamath.h>

#include <string>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "effects11.lib" )

#ifdef _DEBUG
#define PREF_HUD
#pragma comment( lib, "d3dx11d.lib" )
#else
#pragma comment( lib, "d3dx11.lib" )
#endif

struct SimpleVertex
{
    XMFLOAT3 Pos;
};

Direct3D11::Direct3D11( HWND hwnd, int w, int h, bool full_screen, const char* adapter_format, const char* depth_stencil_format, int multi_sample_type, int multi_sample_quality )
	: device_( 0 )
	, immediate_context_( 0 )
	, swap_chain_( 0 )
	
	, render_target_view_( 0 )

	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )

	, effect_( 0 )
	, vertex_layout_( 0 )
	
	, constant_buffer_( 0 )
{
	common::log( "log/d3d11.log", "", false );

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL feature_level = feature_levels[ 0 ];

	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 };
	swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = w;
    swap_chain_desc.BufferDesc.Height = h;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.Windowed = ! full_screen;

	DIRECT_X_FAIL_CHECK( D3D11CreateDeviceAndSwapChain( 0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, feature_levels, sizeof( feature_levels ) / sizeof( D3D_FEATURE_LEVEL ), D3D11_SDK_VERSION, & swap_chain_desc, & swap_chain_, & device_, & feature_level, & immediate_context_ ) );

    ID3D11Texture2D* back_buffer = 0;

	DIRECT_X_FAIL_CHECK( swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & back_buffer ) ) );
	DIRECT_X_FAIL_CHECK( device_->CreateRenderTargetView( back_buffer, 0, & render_target_view_ ) );

	DIRECT_X_RELEASE( back_buffer );

	immediate_context_->OMSetRenderTargets( 1, & render_target_view_, 0 );


	D3D11_TEXTURE2D_DESC texture_desc = { 0 };
    
	texture_desc.Width = w;
    texture_desc.Height = h;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0;

	DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( & texture_desc, 0, & depth_stencil_texture_ ) );


	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = { texture_desc.Format };
	
    depth_stencil_view_desc.Format = texture_desc.Format;
    depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_desc.Texture2D.MipSlice = 0;

	DIRECT_X_FAIL_CHECK( device_->CreateDepthStencilView( depth_stencil_texture_, & depth_stencil_view_desc, & depth_stencil_view_ ) );

    immediate_context_->OMSetRenderTargets( 1, & render_target_view_, depth_stencil_view_ );

 
    D3D11_VIEWPORT viewport;
    
	viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
	viewport.Width = static_cast< float >( w );
    viewport.Height = static_cast< float >( h );
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    immediate_context_->RSSetViewports( 1, & viewport );

	// ?
    immediate_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

Direct3D11::~Direct3D11()
{
	DIRECT_X_RELEASE( constant_buffer_ );

	DIRECT_X_RELEASE( vertex_layout_ );
	
	DIRECT_X_RELEASE( effect_ );

	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );

	DIRECT_X_RELEASE( render_target_view_ );
	DIRECT_X_RELEASE( swap_chain_ );
	DIRECT_X_RELEASE( immediate_context_ );
	DIRECT_X_RELEASE( device_ );
}

void Direct3D11::load_effect_file( const char* file_path )
{
	ID3D10Blob* shader = 0;
	ID3D10Blob* error_messages = 0;

	HRESULT hr = D3DX11CompileFromFile( file_path, 0, 0, 0, "fx_5_0", 0, 0, 0, & shader, & error_messages, 0  );

	if ( FAILED( hr ) )
	{
		if ( error_messages )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_messages->GetBufferPointer() ) );
		}
		else
		{
			DIRECT_X_FAIL_CHECK( hr );
		}
	}

	int n = shader->GetBufferSize();

	DIRECT_X_FAIL_CHECK( D3DX11CreateEffectFromMemory( shader->GetBufferPointer(), shader->GetBufferSize(), 0, device_, & effect_ ) );

	
	// ?
	D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	UINT numElements = ARRAYSIZE( layout );


	ID3DX11EffectPass* pass = effect_->GetTechniqueByName( "|main" )->GetPassByIndex( 0 ); 

	D3DX11_PASS_DESC pass_desc;
	pass->GetDesc( & pass_desc );

	DIRECT_X_FAIL_CHECK( device_->CreateInputLayout( layout, numElements, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, & vertex_layout_ ) );

	// ?
	immediate_context_->IASetInputLayout( vertex_layout_ );
}

void Direct3D11::apply_effect()
{
	ID3DX11EffectTechnique* technique = effect_->GetTechniqueByName( "|main" );

	ID3DX11EffectPass* pass = technique->GetPassByIndex( 0 ); 
	DIRECT_X_FAIL_CHECK( pass->Apply( 0, immediate_context_ ) );
}

void Direct3D11::clear()
{
	float clear_color[ 4 ] = { 0.f, 0.125f, 0.3f, 1.0f };

	immediate_context_->ClearRenderTargetView( render_target_view_, clear_color );
	immediate_context_->ClearDepthStencilView( depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}
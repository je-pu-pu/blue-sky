#include "Direct3D11.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11TextureManager.h"
#include "DirectX.h"

#include "include/d3dx11effect.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <xnamath.h>

#include <dxgi.h>
#include <d3d10_1.h>

#include <string>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment (lib, "d3d10_1.lib")
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "effects11.lib" )


#ifdef _DEBUG
#pragma comment( lib, "d3dx11d.lib" )
#pragma comment( lib, "d3dx10d.lib" )
#else
#pragma comment( lib, "d3dx11.lib" )
// #pragma comment( lib, "d3dx10.lib" )
#endif

Direct3D11::Direct3D11( HWND hwnd, int w, int h, bool full_screen, const char* adapter_format, const char* depth_stencil_format, int multi_sample_type, int multi_sample_quality )
	: device_( 0 )
	, immediate_context_( 0 )
	, swap_chain_( 0 )
	
	, back_buffer_texture_( 0 )
	, back_buffer_view_( 0 )

	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )

	, effect_( 0 )
	, vertex_layout_( 0 )
	
	, device_10_( 0 )
	, back_buffer_surface_( 0 )
	, text_texture_( 0 )
	, text_view_( 0 )
	, text_surface_( 0 )
	, text_texture_mutex_11_( 0 )
	, text_texture_mutex_10_( 0 )
{
	common::log( "log/d3d11.log", "", false );

	IDXGIAdapter1* dxgi_adapter = 0;

	// get_adapter()
	{
		IDXGIFactory1* dxgi_factory = 0;

		DIRECT_X_FAIL_CHECK( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), reinterpret_cast< void** >( & dxgi_factory ) ) );

		for ( int n = 0; dxgi_factory->EnumAdapters1( n, & dxgi_adapter ) != DXGI_ERROR_NOT_FOUND; n++ )
		{
			DXGI_ADAPTER_DESC1 adapter_desc;
			DIRECT_X_FAIL_CHECK( dxgi_adapter->GetDesc1( & adapter_desc ) );

			log_adapter_desc( n, adapter_desc );

			DIRECT_X_RELEASE( dxgi_adapter );
		}

		DIRECT_X_FAIL_CHECK( dxgi_factory->EnumAdapters1( 0, & dxgi_adapter ) );
		DIRECT_X_RELEASE( dxgi_factory );

		// MessageBoxW( 0, adapter_desc.Description, L"HOGE", MB_OK );
	}
	

	D3D_FEATURE_LEVEL feature_levels[] = {
//		D3D_FEATURE_LEVEL_11_0,
//		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL feature_level = feature_levels[ 0 ];

	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 };
	swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = w;
    swap_chain_desc.BufferDesc.Height = h;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.Windowed = ! full_screen;

	// !!!
	DIRECT_X_FAIL_CHECK( D3D11CreateDeviceAndSwapChain( dxgi_adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT, feature_levels, sizeof( feature_levels ) / sizeof( D3D_FEATURE_LEVEL ), D3D11_SDK_VERSION, & swap_chain_desc, & swap_chain_, & device_, & feature_level, & immediate_context_ ) );

	// Direct3D 10.1
	{
		// D3D10_CREATE_DEVICE_DEBUG
		DIRECT_X_FAIL_CHECK( D3D10CreateDevice1( dxgi_adapter, D3D10_DRIVER_TYPE_HARDWARE, 0, D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, & device_10_ ) );
	}

	DIRECT_X_RELEASE( dxgi_adapter );

	// create_back_buffer_view()
	{
		DIRECT_X_FAIL_CHECK( swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & back_buffer_texture_ ) ) );
		DIRECT_X_FAIL_CHECK( device_->CreateRenderTargetView( back_buffer_texture_, 0, & back_buffer_view_ ) );
	}

	// create_back_buffer_surface()
	{
		DIRECT_X_FAIL_CHECK( back_buffer_texture_->QueryInterface( __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & back_buffer_surface_ ) ) );
	}

	// create_text_texture()
	{
		D3D11_TEXTURE2D_DESC texture_desc = { 0 };

		texture_desc.Width = w;
		texture_desc.Height = h;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

		DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( & texture_desc, 0, & text_texture_ ) );

		// create_text_view()
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { texture_desc.Format };

			view_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipLevels = texture_desc.MipLevels;
			view_desc.Texture2D.MostDetailedMip = 0;

			DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( text_texture_, & view_desc, & text_view_ ) );
		}
	}

	// create_text_texture_mutex()
	{
		HANDLE shared_handle = 0;
		IDXGIResource* text_texture_resource_ = 0;

		DIRECT_X_FAIL_CHECK( text_texture_->QueryInterface( __uuidof( IDXGIKeyedMutex ), reinterpret_cast< void** >( & text_texture_mutex_11_ ) ) );
		DIRECT_X_FAIL_CHECK( text_texture_->QueryInterface( __uuidof( IDXGIResource ), reinterpret_cast< void** >( & text_texture_resource_ ) ) );
		DIRECT_X_FAIL_CHECK( text_texture_resource_->GetSharedHandle( & shared_handle ) );

		DIRECT_X_FAIL_CHECK( device_10_->OpenSharedResource( shared_handle, __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & text_surface_ ) ) );
		DIRECT_X_FAIL_CHECK( text_surface_->QueryInterface( __uuidof( IDXGIKeyedMutex ), reinterpret_cast< void** >( & text_texture_mutex_10_ ) ) );

		DIRECT_X_RELEASE( text_texture_resource_ );
	}

	// create_depth_stencil_view()
	{
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
	}

	// setup_viewport()
	{
		viewport_.TopLeftX = 0;
		viewport_.TopLeftY = 0;
		viewport_.Width = static_cast< float >( w );
		viewport_.Height = static_cast< float >( h );
		viewport_.MinDepth = 0.f;
		viewport_.MaxDepth = 1.f;
	}
}

Direct3D11::~Direct3D11()
{
	DIRECT_X_RELEASE( text_texture_mutex_10_ );
	DIRECT_X_RELEASE( text_texture_mutex_11_ );
	DIRECT_X_RELEASE( text_surface_ );

	DIRECT_X_RELEASE( text_view_ );
	DIRECT_X_RELEASE( text_texture_ );

	DIRECT_X_RELEASE( back_buffer_surface_ );

	DIRECT_X_RELEASE( device_10_ );

	DIRECT_X_RELEASE( vertex_layout_ );
	
	DIRECT_X_RELEASE( effect_ );

	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );

	DIRECT_X_RELEASE( back_buffer_view_ );
	DIRECT_X_RELEASE( back_buffer_texture_ );

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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	UINT numElements = ARRAYSIZE( layout );


	ID3DX11EffectPass* pass = effect_->GetTechniqueByName( "|drawing_line" )->GetPassByIndex( 0 ); 

	D3DX11_PASS_DESC pass_desc;
	pass->GetDesc( & pass_desc );

	DIRECT_X_FAIL_CHECK( device_->CreateInputLayout( layout, numElements, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, & vertex_layout_ ) );

	// ?
	immediate_context_->IASetInputLayout( vertex_layout_ );


	// Mesh Manager
	mesh_manager_ = new Direct3D11MeshManager( this );

	// Texture Manager
	texture_manager_ = new Direct3D11TextureManager( this );
}

void Direct3D11::apply_effect()
{
	ID3DX11EffectTechnique* technique = effect_->GetTechniqueByName( "|main" );

	ID3DX11EffectPass* pass = technique->GetPassByIndex( 0 ); 
	DIRECT_X_FAIL_CHECK( pass->Apply( 0, immediate_context_ ) );
}

void Direct3D11::clear()
{
	float clear_color[ 4 ] = { 0.6f, 0.8f, 1.f, 1.f };

	immediate_context_->ClearRenderTargetView( back_buffer_view_, clear_color );
	immediate_context_->ClearDepthStencilView( depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.f, 0 );
	immediate_context_->OMSetRenderTargets( 1, & back_buffer_view_, depth_stencil_view_ );
	immediate_context_->RSSetViewports( 1, & viewport_ );
}

void Direct3D11::begin2D()
{
	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->AcquireSync( 0, INFINITE ) );
}

void Direct3D11::end2D()
{
	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->ReleaseSync( 1 ) );
}

void Direct3D11::begin3D()
{
	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->AcquireSync( 1, INFINITE ) );
}

void Direct3D11::end3D()
{
	// immediate_context_->CopyResource( back_buffer_texture_, text_texture_ );

	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->ReleaseSync( 0 ) );
}

void Direct3D11::end()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->Present( 0, 0 ) );
}

void Direct3D11::renderText()
{
	ID3DX11EffectTechnique* technique = effect_->GetTechniqueByName( "|text" );

	D3DX11_TECHNIQUE_DESC technique_desc;
	technique->GetDesc( & technique_desc );

	for ( UINT n = 0; n < technique_desc.Passes; n++ )
	{
		ID3DX11EffectPass* pass = technique->GetPassByIndex( n ); 
		DIRECT_X_FAIL_CHECK( pass->Apply( 0, immediate_context_ ) );

		immediate_context_->PSSetShaderResources( 0, 1, & text_view_ );
		immediate_context_->Draw( 3, 0 ); // !!!
	}
}

void Direct3D11::setDebugViewport()
{
	D3D11_TEXTURE2D_DESC texture_desc;

	back_buffer_texture_->GetDesc( & texture_desc );
	
	static_cast< float >( texture_desc.Width );

	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = static_cast< float >( texture_desc.Height ) / 4.f * 3.f;
	viewport.Width = static_cast< float >( texture_desc.Width ) / 4.f;
	viewport.Height = static_cast< float >( texture_desc.Height ) / 4.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	immediate_context_->RSSetViewports( 1, & viewport );
}

void Direct3D11::log_adapter_desc( int index, const DXGI_ADAPTER_DESC1& adapter_desc )
{
	#define adapter_desc_string( name ) std::wstring( L"\t" ) + L# name + L" : " + common::w_serialize( adapter_desc.name ) + L"\n"

	std::wstring text = common::w_serialize( index ) + L" : " + adapter_desc.Description + L"\n" +
		adapter_desc_string( VendorId ) +
		adapter_desc_string( DeviceId ) +
		adapter_desc_string( SubSysId ) +
		adapter_desc_string( Revision ) +
		adapter_desc_string( DedicatedVideoMemory ) +
		adapter_desc_string( DedicatedSystemMemory ) +
		adapter_desc_string( SharedSystemMemory ) +
		adapter_desc_string( AdapterLuid.LowPart ) + 
		adapter_desc_string( AdapterLuid.HighPart ) + 
		adapter_desc_string( Flags );

	common::log( "log/adapter_info.log", text );
}
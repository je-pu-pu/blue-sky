#include "Direct3D11.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Effect.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11Color.h"
#include "DirectWrite.h"
#include "DirectX.h"

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <xnamath.h>

#include <dxgi.h>
#include <d3d10_1.h>

#include <string>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment (lib, "d3d10_1.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#ifdef _DEBUG
#pragma comment( lib, "d3dx11d.lib" )
// #pragma comment( lib, "d3dx10d.lib" )
#else
#pragma comment( lib, "d3dx11.lib" )
// #pragma comment( lib, "d3dx10.lib" )
#endif

// #define 
#define ENABLE_DIRECT_WRITE

Direct3D11::Direct3D11( HWND hwnd, int w, int h, bool full_screen, const char* adapter_format, const char* depth_stencil_format, int multi_sample_type, int multi_sample_quality )
	: device_( 0 )
	, immediate_context_( 0 )
	, swap_chain_( 0 )
	
	, back_buffer_texture_( 0 )
	, back_buffer_view_( 0 )

	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )

	, device_10_( 0 )
	, back_buffer_surface_( 0 )
	, text_texture_( 0 )
	, text_view_( 0 )
	, text_surface_( 0 )
	, text_texture_mutex_11_( 0 )
	, text_texture_mutex_10_( 0 )
{
	common::log( "log/d3d11.log", "init", false );

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
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL feature_level = feature_levels[ 0 ];

	ZeroMemory( & swap_chain_desc_, sizeof( swap_chain_desc_ ) );
	swap_chain_desc_.BufferCount = 1;
    swap_chain_desc_.BufferDesc.Width = w;
    swap_chain_desc_.BufferDesc.Height = h;
    swap_chain_desc_.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
//	swap_chain_desc_.BufferDesc.RefreshRate.Numerator = 60;
//	swap_chain_desc_.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc_.OutputWindow = hwnd;
    swap_chain_desc_.SampleDesc.Count = 1;
    swap_chain_desc_.SampleDesc.Quality = 0;
    swap_chain_desc_.Windowed = ! full_screen;
	swap_chain_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef _DEBUG
    UINT d3d11_create_device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT d3d11_create_device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	// !!!
	DIRECT_X_FAIL_CHECK( D3D11CreateDeviceAndSwapChain( dxgi_adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, d3d11_create_device_flags, feature_levels, ARRAYSIZE( feature_levels ), D3D11_SDK_VERSION, & swap_chain_desc_, & swap_chain_, & device_, & feature_level, & immediate_context_ ) );

	// log_feature_level_11()
	{
		std::map< D3D_FEATURE_LEVEL, std::string > feature_level_map;

		feature_level_map[ D3D_FEATURE_LEVEL_9_1 ] = "9.1";
		feature_level_map[ D3D_FEATURE_LEVEL_9_2 ] = "9.2";
		feature_level_map[ D3D_FEATURE_LEVEL_9_3 ] = "9.3";
		feature_level_map[ D3D_FEATURE_LEVEL_10_0 ] = "10.0";
		feature_level_map[ D3D_FEATURE_LEVEL_10_1 ] = "10.1";
		feature_level_map[ D3D_FEATURE_LEVEL_11_0 ] = "11.0";

		common::log( "log/d3d11.log", std::string( "created d3d11 device ( feature_level : " ) + feature_level_map[ device_->GetFeatureLevel() ] + " )" );
	}

	// multi_sample()
	/// @todo 実装する
	{
		UINT multi_sample_count = 4;
		UINT multi_sample_quality = 0;

		if ( device_->CheckMultisampleQualityLevels( swap_chain_desc_.BufferDesc.Format, multi_sample_count, & multi_sample_quality ) )
		{
			int x = 0;
		}
	}

#ifdef ENABLE_DIRECT_WRITE
	// Direct3D 10.1
	{
#ifdef _DEBUG
		UINT d3d10_create_device_flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT | D3D10_CREATE_DEVICE_DEBUG;
#else
		UINT d3d10_create_device_flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
#endif

		D3D10_FEATURE_LEVEL1 d3d10_feature_levels[] =
		{
			D3D10_FEATURE_LEVEL_9_3,
			D3D10_FEATURE_LEVEL_9_2,
			D3D10_FEATURE_LEVEL_9_1,

			// D3D10_FEATURE_LEVEL_10_1,
			// D3D10_FEATURE_LEVEL_10_0,
		};

		const char* d3d10_feature_level_names[] =
		{
			"9.3",
			"9.2",
			"9.1",

			// "10.1",
			// "10.0",
		};

		for ( int n = 0; n < ARRAYSIZE( d3d10_feature_levels ); n++ )
		{
			if ( SUCCEEDED( D3D10CreateDevice1( dxgi_adapter, D3D10_DRIVER_TYPE_HARDWARE, 0, d3d10_create_device_flags, d3d10_feature_levels[ n ], D3D10_1_SDK_VERSION, & device_10_ ) ) )
			{
				common::log( "log/d3d11.log", std::string( "created d3d10 device ( feature_level : " ) + d3d10_feature_level_names[ n ] + " )" );
				break;
			}
		}

		if ( ! device_10_ )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "D3D10CreateDevice1() failed." );
		}

		// D2D のデバッグレイヤーメッセージを抑制
		device_10_->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}
#endif

	DIRECT_X_RELEASE( dxgi_adapter );

	create_back_buffer_view();
	create_back_buffer_surface();

#ifdef ENABLE_DIRECT_WRITE
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

		DIRECT_X_RELEASE( text_texture_resource_ );

		DIRECT_X_FAIL_CHECK( device_10_->OpenSharedResource( shared_handle, __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & text_surface_ ) ) );
		DIRECT_X_FAIL_CHECK( text_surface_->QueryInterface( __uuidof( IDXGIKeyedMutex ), reinterpret_cast< void** >( & text_texture_mutex_10_ ) ) );
	}
#endif

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

#ifdef ENABLE_DIRECT_WRITE
	// Font
	font_ = new Font( text_surface_ );
#endif

	DIRECT_X_RELEASE( text_surface_ );

	sprite_ = new Sprite( this );
	fader_ = new Fader( this );
	effect_ = new Effect( this );
	
	mesh_manager_ = new Direct3D11MeshManager( this );
	texture_manager_ = new Direct3D11TextureManager( this );
}

/// @todo D3D11_CREATE_DEVICE_DEBUG を有効にし、メモリリークを調べる
Direct3D11::~Direct3D11()
{
	set_full_screen( false );

	texture_manager_.release();
	mesh_manager_.release();
	effect_.release();
	fader_.release();
	sprite_.release();
	font_.release();

	DIRECT_X_RELEASE( text_texture_mutex_10_ );
	DIRECT_X_RELEASE( text_texture_mutex_11_ );
	DIRECT_X_RELEASE( text_surface_ );

	DIRECT_X_RELEASE( text_view_ );
	DIRECT_X_RELEASE( text_texture_ );

	DIRECT_X_RELEASE( back_buffer_surface_ );

	if ( device_10_ )
	{
		device_10_->ClearState();
		device_10_->Flush();
	}

	if ( false && device_10_ )
	{
		ID3D10Debug* debug_ = 0;

		DIRECT_X_FAIL_CHECK( device_10_->QueryInterface( __uuidof( ID3D10Debug ), reinterpret_cast< void** >( & debug_ ) ) );
		DIRECT_X_FAIL_CHECK( debug_->Validate() );

		DIRECT_X_RELEASE( debug_ );
	}

	DIRECT_X_RELEASE( device_10_ );

	for ( InputLayoutList::iterator i = vertex_layout_list_.begin(); i != vertex_layout_list_.end(); ++i )
	{
		DIRECT_X_RELEASE( i->second );
	}

	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );

	DIRECT_X_RELEASE( back_buffer_view_ );
	DIRECT_X_RELEASE( back_buffer_texture_ );

	DIRECT_X_RELEASE( swap_chain_ );

	if ( immediate_context_ )
	{
		immediate_context_->ClearState();
		immediate_context_->Flush();
	}

	if ( false )
	{
		ID3D11Debug* debug_ = 0;

		DIRECT_X_FAIL_CHECK( device_->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast< void** >( & debug_ ) ) );
		DIRECT_X_FAIL_CHECK( debug_->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL ) );

		DIRECT_X_RELEASE( debug_ );
	}

	DIRECT_X_RELEASE( immediate_context_ );
	DIRECT_X_RELEASE( device_ );
}

void Direct3D11::create_back_buffer_view()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & back_buffer_texture_ ) ) );
	DIRECT_X_FAIL_CHECK( device_->CreateRenderTargetView( back_buffer_texture_, 0, & back_buffer_view_ ) );
}

void Direct3D11::create_back_buffer_surface()
{
	DIRECT_X_FAIL_CHECK( back_buffer_texture_->QueryInterface( __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & back_buffer_surface_ ) ) );
}

// ???
void Direct3D11::create_default_input_layout()
{
	// main
	D3D11_INPUT_ELEMENT_DESC layout_main[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	vertex_layout_list_[ "main" ] = ( * effect_->getTechnique( "|main" )->getPassList().begin() )->createVertexLayout( layout_main, ARRAYSIZE( layout_main ) );

	// skin
	D3D11_INPUT_ELEMENT_DESC layout_skin[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE",     0, DXGI_FORMAT_R8G8B8A8_UINT,      1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,     1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	vertex_layout_list_[ "skin" ] = ( * effect_->getTechnique( "|skin" )->getPassList().begin() )->createVertexLayout( layout_skin, ARRAYSIZE( layout_skin ) );

	// line
	D3D11_INPUT_ELEMENT_DESC layout_line[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	vertex_layout_list_[ "line" ] = ( * effect_->getTechnique( "|drawing_line" )->getPassList().begin() )->createVertexLayout( layout_line, ARRAYSIZE( layout_line ) );

	// sprite 
	D3D11_INPUT_ELEMENT_DESC layout_sprite[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	vertex_layout_list_[ "sprite" ] = ( * effect_->getTechnique( "|sprite" )->getPassList().begin() )->createVertexLayout( layout_sprite, ARRAYSIZE( layout_sprite ) );
}

void Direct3D11::set_full_screen( bool full_screen )
{
	DIRECT_X_FAIL_CHECK( swap_chain_->SetFullscreenState( full_screen, 0 ) );
}

void Direct3D11::set_size( int w, int h )
{
	swap_chain_desc_.BufferDesc.Width = w;
	swap_chain_desc_.BufferDesc.Height = h;

	DIRECT_X_FAIL_CHECK( swap_chain_->ResizeBuffers(
		swap_chain_desc_.BufferCount,
		swap_chain_desc_.BufferDesc.Width,
		swap_chain_desc_.BufferDesc.Height,
		swap_chain_desc_.BufferDesc.Format,
		swap_chain_desc_.Flags
	) );
}

bool Direct3D11::is_full_screen() const
{
	BOOL full_screen;

	DIRECT_X_FAIL_CHECK( swap_chain_->GetFullscreenState( & full_screen, 0 ) );

	return full_screen == TRUE;
}

void Direct3D11::switch_full_screen()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->ResizeTarget( & swap_chain_desc_.BufferDesc ) );
	DIRECT_X_FAIL_CHECK( swap_chain_->SetFullscreenState( ! is_full_screen(), 0 ) );
}

void Direct3D11::on_resize( int w, int h )
{
	immediate_context_->OMSetRenderTargets( 0, 0, 0 );

	DIRECT_X_RELEASE( back_buffer_surface_ );
	DIRECT_X_RELEASE( back_buffer_view_ );
	DIRECT_X_RELEASE( back_buffer_texture_ );

	swap_chain_desc_.BufferDesc.Width = w;
	swap_chain_desc_.BufferDesc.Height = h;

	DIRECT_X_FAIL_CHECK( swap_chain_->ResizeBuffers(
		swap_chain_desc_.BufferCount,
		swap_chain_desc_.BufferDesc.Width,
		swap_chain_desc_.BufferDesc.Height,
		swap_chain_desc_.BufferDesc.Format,
		swap_chain_desc_.Flags
	) );

	create_back_buffer_view();
	create_back_buffer_surface();
}

void Direct3D11::clear()
{
	clear( Color( 1, 1, 1, 1 ) );
}

void Direct3D11::clear( const Color& color )
{
	immediate_context_->ClearRenderTargetView( back_buffer_view_, static_cast< const float* >( & color.r() ) );
	immediate_context_->ClearDepthStencilView( depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.f, 0 );
	immediate_context_->OMSetRenderTargets( 1, & back_buffer_view_, depth_stencil_view_ );
	immediate_context_->RSSetViewports( 1, & viewport_ );
}

void Direct3D11::setInputLayout( const char* name )
{
	immediate_context_->IASetInputLayout( vertex_layout_list_[ name ] );
}

void Direct3D11::begin2D()
{
	if ( ! text_texture_mutex_10_ )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->AcquireSync( 0, INFINITE ) );
}

void Direct3D11::end2D()
{
	if ( ! text_texture_mutex_10_ )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->ReleaseSync( 1 ) );
}

void Direct3D11::begin3D()
{
	if ( ! text_texture_mutex_11_ )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->AcquireSync( 1, INFINITE ) );
}

void Direct3D11::end3D()
{
	if ( ! text_texture_mutex_11_ )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->ReleaseSync( 0 ) );
}

void Direct3D11::end()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->Present( 0, 0 ) );
}

void Direct3D11::renderText()
{
	getSprite()->begin();

	EffectTechnique* technique = effect_->getTechnique( "|sprite" );

	for ( EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		Sprite::Rect dst_rect( 0, 0, swap_chain_desc_.BufferDesc.Width, swap_chain_desc_.BufferDesc.Height );
		getSprite()->draw( dst_rect, text_view_ );
	}

	getSprite()->end();
}

void Direct3D11::setDebugViewport( float x, float y, float w, float h )
{
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = w;
	viewport.Height = h;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	immediate_context_->RSSetViewports( 1, & viewport );
}

void Direct3D11::getTexture2dDescByTexture( Texture* texture, D3D11_TEXTURE2D_DESC* texture_2d_desc )
{
	com_ptr< ID3D11Resource > texture_2d_resource;

	texture->GetResource( & texture_2d_resource );

	ID3D11Texture2D* texture_2d = static_cast< ID3D11Texture2D* >( texture_2d_resource.get() );
	texture_2d->GetDesc( texture_2d_desc );
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
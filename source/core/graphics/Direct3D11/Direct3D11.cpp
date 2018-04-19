#include "Direct3D11.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Effect.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11Texture.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11Color.h"
#include <core/graphics/DirectWrite/DirectWrite.h>

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <dxgi.h>
#include <d3d10_1.h>

#include <string>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3d10_1.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

const Direct3D11::Color Direct3D11::DEFAULT_CLEAR_COLOR = Direct3D11::Color::Black;

/**
 * コンストラクタ
 *
 * @param hwnd					ウィンドウハンドル
 * @param w						バックバッファの幅
 * @param h						バックバッファの高さ
 * @param full_screen			フルスクリーンフラグ
 * @param adapter_format		未使用
 * @param depth_stencil_format	未使用
 * @param multi_sample_count	マルチサンプリングのサンプル数
 * @param multi_sample_quality	マルチサンプリングのクオリティ
 */
Direct3D11::Direct3D11( HWND hwnd, int w, int h, bool full_screen, const char* /* adapter_format */, const char* /* depth_stencil_format */, int multi_sample_count, int multi_sample_quality )
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

	IDXGIFactory1* dxgi_factory = 0;
	DIRECT_X_FAIL_CHECK( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), reinterpret_cast< void** >( & dxgi_factory ) ) );

	log_all_adapter_desc( dxgi_factory );

	DIRECT_X_FAIL_CHECK( dxgi_factory->EnumAdapters1( 0, & dxgi_adapter_ ) );

	create_device();

	log_feature_level();

	create_swap_chain( dxgi_factory, hwnd, w, h, full_screen, multi_sample_count, multi_sample_quality );

	DIRECT_X_RELEASE( dxgi_factory );

	create_back_buffer_view();
	create_back_buffer_surface();

	create_depth_stencil_view();
	
	setup_viewport();

	sprite_ = new Sprite( this );
	fader_ = new Fader( this );
	effect_ = new Effect( this );
	
	mesh_manager_ = new Direct3D11MeshManager( this );
	texture_manager_ = new Direct3D11TextureManager( this );
}

/**
 * デストラクタ
 *
 */
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

	text_view_.release();
	DIRECT_X_RELEASE( text_texture_ );

	DIRECT_X_RELEASE( back_buffer_surface_ );

	if ( immediate_context_ )
	{
		immediate_context_->ClearState();
		immediate_context_->Flush();
	}

	if ( device_10_ )
	{
		device_10_->ClearState();
		device_10_->Flush();
	}

	if ( false )
	// if ( device_10_ )
	{
		ID3D10Debug* debug_ = 0;

		device_10_->QueryInterface( __uuidof( ID3D10Debug ), reinterpret_cast< void** >( &debug_ ) );
		debug_->Validate();

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

	if ( false )
	{
		ID3D11Debug* debug_ = 0;

		device_->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast< void** >( & debug_ ) );
		debug_->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

		DIRECT_X_RELEASE( debug_ );
	}

	DIRECT_X_RELEASE( immediate_context_ );
	DIRECT_X_RELEASE( device_ );
}

void Direct3D11::create_device()
{
	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL feature_level = feature_levels[ 0 ];

#ifdef _DEBUG
    UINT d3d11_create_device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT d3d11_create_device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	DIRECT_X_FAIL_CHECK( D3D11CreateDevice( dxgi_adapter_.get(), D3D_DRIVER_TYPE_UNKNOWN, 0, d3d11_create_device_flags, feature_levels, ARRAYSIZE( feature_levels ), D3D11_SDK_VERSION, & device_, & feature_level, & immediate_context_ ) );
}

void Direct3D11::create_swap_chain( IDXGIFactory1* dxgi_factory, HWND hwnd, uint_t w, uint_t h, bool full_screen, int multi_sample_count, int multi_sample_quality )
{
	ZeroMemory( & swap_chain_desc_, sizeof( swap_chain_desc_ ) );
	swap_chain_desc_.BufferCount = 1;
	swap_chain_desc_.BufferDesc.Width = w;
	swap_chain_desc_.BufferDesc.Height = h;
	swap_chain_desc_.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
//	swap_chain_desc_.BufferDesc.RefreshRate.Numerator = 60;
//	swap_chain_desc_.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc_.OutputWindow = hwnd;
	swap_chain_desc_.SampleDesc.Count = multi_sample_count;
	swap_chain_desc_.SampleDesc.Quality = multi_sample_quality;
	swap_chain_desc_.Windowed = ! full_screen;
	swap_chain_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// check_multi_sample()
	{
		if ( multi_sample_count < 1 )
		{
			multi_sample_count = 1;
		}
		if ( multi_sample_quality < 0 )
		{
			multi_sample_quality = 0;
		}

		UINT available_quality_levels = 0;

		DIRECT_X_FAIL_CHECK( device_->CheckMultisampleQualityLevels( swap_chain_desc_.BufferDesc.Format, multi_sample_count, & available_quality_levels ) );

		if ( available_quality_levels == 0 )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "multi sample quality is not supported" );
		}

		if ( static_cast< UINT >( multi_sample_quality ) >= available_quality_levels )
		{
			multi_sample_quality = available_quality_levels - 1;
		}

		swap_chain_desc_.SampleDesc.Count = multi_sample_count;
		swap_chain_desc_.SampleDesc.Quality = multi_sample_quality;
	}
		
	DIRECT_X_FAIL_CHECK( dxgi_factory->CreateSwapChain( device_, & swap_chain_desc_, & swap_chain_ ) );
}

void Direct3D11::create_back_buffer_view()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( & back_buffer_texture_ ) ) );
	back_buffer_view_ = create_render_target_view( back_buffer_texture_ );
}

void Direct3D11::create_back_buffer_surface()
{
	DIRECT_X_FAIL_CHECK( back_buffer_texture_->QueryInterface( __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & back_buffer_surface_ ) ) );
}

ID3D11RenderTargetView* Direct3D11::create_render_target_view( ID3D11Texture2D* texture )
{
	ID3D11RenderTargetView* view = 0;
	DIRECT_X_FAIL_CHECK( device_->CreateRenderTargetView( texture, nullptr, & view ) );
	return view;
}

void Direct3D11::create_depth_stencil_view()
{
	D3D11_TEXTURE2D_DESC texture_desc = { 0 };
    
	texture_desc.Width = swap_chain_desc_.BufferDesc.Width;
	texture_desc.Height = swap_chain_desc_.BufferDesc.Height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DEPTH_STENCIL_FORMAT;
	texture_desc.SampleDesc = swap_chain_desc_.SampleDesc;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( & texture_desc, 0, & depth_stencil_texture_ ) );

	depth_stencil_view_ = create_depth_stencil_view( depth_stencil_texture_ );
}

ID3D11DepthStencilView* Direct3D11::create_depth_stencil_view( ID3D11Texture2D* texture )
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = { 0 };
	
	depth_stencil_view_desc.Format = DEPTH_STENCIL_FORMAT;
	depth_stencil_view_desc.ViewDimension = swap_chain_desc_.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView* view = 0;
	DIRECT_X_FAIL_CHECK( device_->CreateDepthStencilView( texture, & depth_stencil_view_desc, & view ) );

	return view;
}

void Direct3D11::setup_viewport()
{
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.Width = static_cast< float >( swap_chain_desc_.BufferDesc.Width );
	viewport_.Height = static_cast< float >( swap_chain_desc_.BufferDesc.Height );
	viewport_.MinDepth = 0.f;
	viewport_.MaxDepth = 1.f;
}

/**
 * フォントをセットアップする
 *
 */
void Direct3D11::setup_font()
{
	if ( font_ )
	{
		return;
	}

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
			if ( SUCCEEDED( D3D10CreateDevice1( dxgi_adapter_.get(), D3D10_DRIVER_TYPE_HARDWARE, 0, d3d10_create_device_flags, d3d10_feature_levels[ n ], D3D10_1_SDK_VERSION, & device_10_ ) ) )
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

	// create_text_texture()
	{
		D3D11_TEXTURE2D_DESC texture_desc = { 0 };

		texture_desc.Width = swap_chain_desc_.BufferDesc.Width;
		texture_desc.Height = swap_chain_desc_.BufferDesc.Height;
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

			ID3D11ShaderResourceView* text_view = 0;
			DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( text_texture_, & view_desc, & text_view ) );

			text_view_ = new Texture( this, text_view );
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

	// Font
	font_ = new Font( text_surface_ );

	DIRECT_X_RELEASE( text_surface_ );
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

	create_input_layout( "main", "|main", layout_main, ARRAYSIZE( layout_main ) );

	// skin
	D3D11_INPUT_ELEMENT_DESC layout_skin[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE",     0, DXGI_FORMAT_R8G8B8A8_UINT,      1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHT",   0, DXGI_FORMAT_R8G8B8A8_UNORM,     1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "skin", "|skin", layout_skin, ARRAYSIZE( layout_skin ) );

	// line
	D3D11_INPUT_ELEMENT_DESC layout_line[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "line", "|drawing_line", layout_line, ARRAYSIZE( layout_line ) );

	// 
	D3D11_INPUT_ELEMENT_DESC layout_drawing_point[] =
	{
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PRESSURE",    0, DXGI_FORMAT_R32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	create_input_layout( "drawing_point", "|drawing_point", layout_drawing_point, ARRAYSIZE( layout_drawing_point ) );

	// sprite 
	D3D11_INPUT_ELEMENT_DESC layout_sprite[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "sprite", "|sprite", layout_sprite, ARRAYSIZE( layout_sprite ) );
}

void Direct3D11::create_input_layout( char_t* input_layout_name, char_t* teqhnique_name, D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	vertex_layout_list_[ input_layout_name ] = effect_->getTechnique( teqhnique_name )->getPassList().front()->create_input_layout( layout, layout_array_size );
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
	unset_render_target();

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

/**
 * 指定した色で画面をクリアする
 *
 * @param color クリアに使用する色
 */
void Direct3D11::clear_default_view( const Color& color )
{
	clear_back_buffer_view( color );
	clear_depth_stencil_view();

	// set_default_render_target();
	// set_default_viewport();
}

/**
 * バックバッファのみをクリアする
 *
 * @param color クリアに使用する色
 */
void Direct3D11::clear_back_buffer_view( const Color& color )
{
	clear_render_target_view( back_buffer_view_, color );
}

/**
 * デプスバッファとステンシルバッファのみをクリアする
 *
 */
void Direct3D11::clear_depth_stencil_view()
{
	clear_depth_stencil_view( depth_stencil_view_ );
}

/**
 * 指定したレンダリングターゲットをクリアする
 *
 * @param view レンダリングターゲット
 * @param color クリアに使用する色
 */
void Direct3D11::clear_render_target_view( ID3D11RenderTargetView* view, const Color& color )
{
	immediate_context_->ClearRenderTargetView( view, static_cast< const float* >( & color.r() ) );
}

/**
 * 指定したデプスステンシルをクリアする
 *
 * @param view デプスステンシル
 */
void Direct3D11::clear_depth_stencil_view( ID3D11DepthStencilView* view )
{
	immediate_context_->ClearDepthStencilView( view, D3D11_CLEAR_DEPTH, 1.f, 0 );
}

/**
 * 通常のレンダーターゲットを設定する
 *
 */
void Direct3D11::set_default_render_target()
{
	immediate_context_->OMSetRenderTargets( 1, & back_buffer_view_, depth_stencil_view_ );
}

/**
 * 通常のビューポートを設定する
 *
 */
void Direct3D11::set_default_viewport()
{
	immediate_context_->RSSetViewports( 1, & viewport_ );
}

/**
 * 立体視用にレンダーターゲットを設定する
 *
 * @param render_target_view レンダーターゲット
 * @param depth_stencil_view ビュー
 */
void Direct3D11::set_render_target_for_vr( ID3D11RenderTargetView* render_target_view, ID3D11DepthStencilView* depth_stencil_view )
{
	const float clear_color[ 4 ] = { 0.f, 0.f, 0.f, 1.f };

	// immediate_context_->ClearRenderTargetView( render_target_view, clear_color );
	// clear_depth_stencil_view();

	immediate_context_->OMSetRenderTargets( 1, & render_target_view, depth_stencil_view );
}

/**
 * レンダーターゲットを解除する
 *
 */
void Direct3D11::unset_render_target()
{
	immediate_context_->OMSetRenderTargets( 0, 0, 0 );
}

/**
 * 立体視用に左目用のビューポートを設定する
 *
 */
void Direct3D11::set_viewport_for_left_eye()
{
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast< float >( swap_chain_desc_.BufferDesc.Width / 2 );
	viewport.Height = static_cast< float >( swap_chain_desc_.BufferDesc.Height );
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	immediate_context_->RSSetViewports( 1, & viewport );
}

/**
 * 立体視用に右目用のビューポートを設定する
 *
 */
void Direct3D11::set_viewport_for_right_eye()
{
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = static_cast< float >( swap_chain_desc_.BufferDesc.Width / 2 );
	viewport.TopLeftY = 0;
	viewport.Width = static_cast< float >( swap_chain_desc_.BufferDesc.Width / 2 );
	viewport.Height = static_cast< float >( swap_chain_desc_.BufferDesc.Height );
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	immediate_context_->RSSetViewports( 1, & viewport );
}

void Direct3D11::setInputLayout( const char* name )
{
	immediate_context_->IASetInputLayout( vertex_layout_list_[ name ] );
}

/**
 * テクスチャをピクセルシェーダーに関連付ける
 *
 * @param slot スロット
 * @param texture テクスチャ
 */
void Direct3D11::bind_texture_to_ps( uint_t slot, const Texture* texture )
{
	ID3D11ShaderResourceView* view[] = { texture ? texture->get_shader_resource_view() : nullptr };
	immediate_context_->PSSetShaderResources( slot, 1, view );
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
	if ( ! font_ )
	{
		return;
	}

	getSprite()->begin();

	EffectTechnique* technique = effect_->getTechnique( "|sprite" );

	for ( EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		Sprite::Rect dst_rect( 0, 0, swap_chain_desc_.BufferDesc.Width, swap_chain_desc_.BufferDesc.Height );
		getSprite()->draw( dst_rect, text_view_.get() );
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

void Direct3D11::getTexture2dDescByTexture( const Texture* texture, D3D11_TEXTURE2D_DESC* texture_2d_desc )
{
	com_ptr< ID3D11Resource > texture_2d_resource;

	texture->get_shader_resource_view()->GetResource( & texture_2d_resource );

	ID3D11Texture2D* texture_2d = static_cast< ID3D11Texture2D* >( texture_2d_resource.get() );
	texture_2d->GetDesc( texture_2d_desc );
}

void Direct3D11::log_all_adapter_desc( IDXGIFactory1* dxgi_factory )
{
	IDXGIAdapter1* dxgi_adapter;

	for ( int n = 0; dxgi_factory->EnumAdapters1( n, & dxgi_adapter ) != DXGI_ERROR_NOT_FOUND; n++ )
	{
		DXGI_ADAPTER_DESC1 adapter_desc;
		DIRECT_X_FAIL_CHECK( dxgi_adapter->GetDesc1( & adapter_desc ) );

		log_adapter_desc( n, adapter_desc );

		DIRECT_X_RELEASE( dxgi_adapter );
	}
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

void Direct3D11::log_feature_level()
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
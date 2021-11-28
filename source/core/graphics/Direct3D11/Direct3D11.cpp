#include "Direct3D11.h"
#include "Sprite.h"
#include "Texture.h"
#include "RenderTargetTexture.h"
#include "BackBufferTexture.h"

#include "InputLayout.h"
#include "Effect.h"
#include "EffectTechnique.h"
#include "EffectPass.h"

#include "WICTextureLoader.h"

#include <core/graphics/DirectWrite/DirectWrite.h>

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/string.h>
#include <common/log.h>

#include <dxgi.h>
#include <d3d10_1.h>

#include <string>

#include <iostream>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3d10_1.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

namespace core::graphics::direct_3d_11
{

const Direct3D11::Color Direct3D11::DEFAULT_CLEAR_COLOR = Direct3D11::Color::Black;

Direct3D11* Direct3D11::instance_ = nullptr;

/**
 * コンストラクタ
 *
 * @param hwnd					ウィンドウハンドル
 * @param w						バックバッファの幅
 * @param h						バックバッファの高さ
 * @param full_screen			フルスクリーンフラグ
 * @param multi_sample_count	マルチサンプリングのサンプル数
 * @param multi_sample_quality	マルチサンプリングのクオリティ
 */
Direct3D11::Direct3D11( HWND hwnd, int w, int h, bool full_screen, int multi_sample_count, int multi_sample_quality )
	: device_( 0 )
	, immediate_context_( 0 )
	, swap_chain_( 0 )

	, depth_stencil_texture_( 0 )
	, depth_stencil_view_( 0 )

	, device_10_( 0 )
	, text_texture_( 0 )

	, text_texture_mutex_11_( 0 )
	, text_texture_mutex_10_( 0 )
{
	instance_ = this;

	common::log( "log/d3d11.log", "init", false );

	IDXGIFactory1* dxgi_factory = 0;
	DIRECT_X_FAIL_CHECK( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), reinterpret_cast< void** >( & dxgi_factory ) ) );

	log_all_adapter_desc( dxgi_factory );

	DIRECT_X_FAIL_CHECK( dxgi_factory->EnumAdapters1( 0, & dxgi_adapter_ ) );

	create_device();

	log_feature_level();

	create_swap_chain( dxgi_factory, hwnd, w, h, full_screen, multi_sample_count, multi_sample_quality );

	DIRECT_X_RELEASE( dxgi_factory );

	create_back_buffer_texture();

	create_depth_stencil_view();
	create_depth_texture();
	
	setup_default_viewport();

	effect_.reset( new Effect( this ) );
}

/**
 * デストラクタ
 *
 */
Direct3D11::~Direct3D11()
{
	set_full_screen( false );

	effect_.reset();
	sprite_.reset();
	font_.reset();

	DIRECT_X_RELEASE( text_texture_mutex_10_ );
	DIRECT_X_RELEASE( text_texture_mutex_11_ );

	text_view_.reset();
	DIRECT_X_RELEASE( text_texture_ );

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

	for ( auto& input_layout : input_layout_list_ )
	{
		delete input_layout.second;
	}

	depth_texture_.reset();

	DIRECT_X_RELEASE( depth_stencil_view_ );
	DIRECT_X_RELEASE( depth_stencil_texture_ );

	back_buffer_texture_.reset();

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

/**
 * Direct3D 11 デバイスを作成する
 *
 */
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
	swap_chain_desc_.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; /// Direct2D と共存するため BGRA にする？ // DXGI_FORMAT_R8G8B8A8_UNORM;
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

		/*
		for( int n = 1; n < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; n++ )
		{
			DIRECT_X_FAIL_CHECK( device_->CheckMultisampleQualityLevels( swap_chain_desc_.BufferDesc.Format, n, & available_quality_levels ) );

			std::cout << n << ":" << available_quality_levels << '\n';
		}
		*/

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

/**
 * スワップチェインからバックバッファへの描画用テクスチャを作成する
 *
 */
void Direct3D11::create_back_buffer_texture()
{
	back_buffer_texture_.reset( new BackBufferTexture( this, swap_chain_ ) );
}

/**
 * デプスステンシルテクスチャを作成する
 *
 */
void Direct3D11::create_depth_stencil_view()
{
	D3D11_TEXTURE2D_DESC texture_desc = { 0 };
    
	texture_desc.Width = swap_chain_desc_.BufferDesc.Width;
	texture_desc.Height = swap_chain_desc_.BufferDesc.Height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture_desc.SampleDesc = swap_chain_desc_.SampleDesc;
//	texture_desc.SampleDesc.Count = 1;
//	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( & texture_desc, 0, & depth_stencil_texture_ ) );

	depth_stencil_view_ = create_depth_stencil_view( depth_stencil_texture_ );
}

/**
 * 既存の Texture2D からレンダリング結果を書き込むためのビューを作成する
 *
 */
ID3D11RenderTargetView* Direct3D11::create_render_target_view( ID3D11Texture2D* texture )
{
	ID3D11RenderTargetView* view = 0;
	DIRECT_X_FAIL_CHECK( device_->CreateRenderTargetView( texture, nullptr, & view ) );
	return view;
}

/**
 * 既存の Texture2D からデプスステンシルビューを作成する
 *
 */
ID3D11DepthStencilView* Direct3D11::create_depth_stencil_view( ID3D11Texture2D* texture )
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = { static_cast< DXGI_FORMAT >( 0 ) };
	
	depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension = swap_chain_desc_.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView* view = 0;
	DIRECT_X_FAIL_CHECK( device_->CreateDepthStencilView( texture, & depth_stencil_view_desc, & view ) );

	return view;
}

/**
 * シェーダーリソースに使用できるデプス参照テクスチャを作成する
 *
 */
void Direct3D11::create_depth_texture()
{
	// マルチサンプリングを無効にしたテクスチャ・シェーダーリソースビューを作成する
	if constexpr ( false )
	{
		D3D11_TEXTURE2D_DESC texture_desc = { 0 };
		texture_desc.Width = swap_chain_desc_.BufferDesc.Width;
		texture_desc.Height = swap_chain_desc_.BufferDesc.Height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R32_FLOAT;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.SampleDesc.Quality = 0;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags =  D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags = 0;
		texture_desc.MiscFlags = 0;

		ID3D11Texture2D* texture = 0;
		DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( & texture_desc, 0, & texture ) );

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { texture_desc.Format };

		view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = texture_desc.MipLevels;
		view_desc.Texture2D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* view = 0;
		DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( texture, & view_desc, & view ) );

		depth_texture_.reset( new Texture( this, view ) );
	}

	// デプスステンシルバッファを参照するシェーダーリソースビューを作成する
	if constexpr ( true )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = { DXGI_FORMAT_R32_FLOAT };

		view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		view_desc.ViewDimension = swap_chain_desc_.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = 1;
		view_desc.Texture2D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* view = 0;
		DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( depth_stencil_texture_, & view_desc, & view ) );

		depth_texture_.reset( new Texture( this, view ) );
	}
}


void Direct3D11::setup_default_viewport()
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

			text_view_.reset( new Texture( this, text_view ) );
		}
	}

	com_ptr< IDXGISurface1 > text_surface;

	// create_text_texture_mutex()
	{
		HANDLE shared_handle = 0;
		IDXGIResource* text_texture_resource_ = 0;

		DIRECT_X_FAIL_CHECK( text_texture_->QueryInterface( __uuidof( IDXGIKeyedMutex ), reinterpret_cast< void** >( & text_texture_mutex_11_ ) ) );
		DIRECT_X_FAIL_CHECK( text_texture_->QueryInterface( __uuidof( IDXGIResource ), reinterpret_cast< void** >( & text_texture_resource_ ) ) );
		DIRECT_X_FAIL_CHECK( text_texture_resource_->GetSharedHandle( & shared_handle ) );

		DIRECT_X_RELEASE( text_texture_resource_ );

		DIRECT_X_FAIL_CHECK( device_10_->OpenSharedResource( shared_handle, __uuidof( IDXGISurface1 ), reinterpret_cast< void** >( & text_surface ) ) );
		DIRECT_X_FAIL_CHECK( text_surface->QueryInterface( __uuidof( IDXGIKeyedMutex ), reinterpret_cast< void** >( & text_texture_mutex_10_ ) ) );
	}

	// Font
	font_.reset( new Font( text_surface.get() ) );
}

/**
 * スプライトをセットアップする
 *
 * この関数は、インプットレイアウトを作成した後、エフェクトファイルを読み込んだ後に呼び出す必要がある
 */
void Direct3D11::setup_sprite()
{
	sprite_.reset( new core::graphics::direct_3d_11::Sprite( this ) );
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

	// ポイント 
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

	/// @todo 2D の描画に法線を使っているのは無駄なのでなんとかする
	/*
	// 2D
	D3D11_INPUT_ELEMENT_DESC layout_2d[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "main2d", "|main2d", layout_2d, ARRAYSIZE( layout_2d ) );
	*/
}

void Direct3D11::create_input_layout( char_t* input_layout_name, char_t* teqhnique_name, D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	auto* pass = static_cast< EffectPass* >( effect_->get_technique( teqhnique_name )->get_pass_list().front() );

	input_layout_list_[ input_layout_name ] = new InputLayout( pass->create_input_layout( layout, layout_array_size ) );
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

	swap_chain_desc_.BufferDesc.Width = w;
	swap_chain_desc_.BufferDesc.Height = h;

	DIRECT_X_FAIL_CHECK( swap_chain_->ResizeBuffers(
		swap_chain_desc_.BufferCount,
		swap_chain_desc_.BufferDesc.Width,
		swap_chain_desc_.BufferDesc.Height,
		swap_chain_desc_.BufferDesc.Format,
		swap_chain_desc_.Flags
	) );

	create_back_buffer_texture();

	/// @todo 全ての RenderTargetTexture を再作成する？ ( 必要があるか要調査 )
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
	clear_render_target_view( back_buffer_texture_->get_render_target_view(), color );
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
 * @param bool デプスステンシルビューを同時に設定する
 */
void Direct3D11::set_default_render_target( bool with_depth_stencil )
{
	ID3D11RenderTargetView* views[] = { back_buffer_texture_->get_render_target_view() };
	immediate_context_->OMSetRenderTargets( 1, views, with_depth_stencil ? depth_stencil_view_ : nullptr );
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
 * レンダリング結果を書き込むテクスチャを設定する
 *
 * @param texture レンダリング結果を書き込むテクスチャ
 */
void Direct3D11::set_render_target( RenderTargetTexture* texture )
{
	ID3D11RenderTargetView* views[] = { texture->get_render_target_view() };
	immediate_context_->OMSetRenderTargets( 1, views, depth_stencil_view_ );
}

void Direct3D11::set_render_target( BackBufferTexture* texture )
{
	ID3D11RenderTargetView* views[] = { texture->get_render_target_view() };
	immediate_context_->OMSetRenderTargets( 1, views, depth_stencil_view_ );
}

/**
 * 立体視用にレンダーターゲットを設定する
 *
 * @param render_target_view レンダーターゲット
 * @param depth_stencil_view ビュー
 */
void Direct3D11::set_render_target_for_vr( ID3D11RenderTargetView* render_target_view, ID3D11DepthStencilView* depth_stencil_view )
{
	// const float clear_color[ 4 ] = { 0.f, 0.f, 0.f, 1.f };
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
 * ビューポートを設定する
 *
 */
void Direct3D11::set_viewport( float_t x, float_t y, float_t w, float_t h, float_t d_min, float_t d_max )
{
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = w;
	viewport.Height = h;
	viewport.MinDepth = d_min;
	viewport.MaxDepth = d_max;

	immediate_context_->RSSetViewports( 1, & viewport );
}

void Direct3D11::set_input_layout( const char* name )
{
	set_input_layout( input_layout_list_[ name ] );
}

const Direct3D11::InputLayout* Direct3D11::get_input_layout( const char* name ) const
{
	auto i = input_layout_list_.find( name );

	if ( i == input_layout_list_.end() )
	{
		return nullptr;
	}

	return i->second;
}

void Direct3D11::set_input_layout( const InputLayout* input_layout )
{
	immediate_context_->IASetInputLayout( input_layout->get_input_layout() );
}

Direct3D11::Texture* Direct3D11::load_texture( const char* file_path ) 
{
	ID3D11ShaderResourceView* view = 0;

	std::wstring ws_file_name = common::convert_to_wstring( file_path );
	
	if ( FAILED( DirectX::CreateWICTextureFromFileEx( device_, immediate_context_, ws_file_name.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, nullptr, & view ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "file open failed. " ) + file_path );
	}

	// immediate_context_->GenerateMips( view );

	return new Texture( this, view );
}

/**
 * テクスチャをドメインシェーダーに関連付ける
 *
 * @param slot スロット
 * @param texture テクスチャ
 */
void Direct3D11::bind_texture_to_ds( uint_t slot, const Texture* texture )
{
	ID3D11ShaderResourceView* view[] = { texture ? texture->get_shader_resource_view() : nullptr };
	immediate_context_->DSSetShaderResources( slot, 1, view );
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

	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->AcquireSync( text_texture_sync_key_, INFINITE ) );
}

void Direct3D11::end2D()
{
	if ( ! text_texture_mutex_10_ )
	{
		return;
	}

	text_texture_sync_key_++;

	DIRECT_X_FAIL_CHECK( text_texture_mutex_10_->ReleaseSync( text_texture_sync_key_ ) );
}

void Direct3D11::begin3D()
{
	if ( ! text_texture_mutex_11_ )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->AcquireSync( text_texture_sync_key_, INFINITE ) );
}

void Direct3D11::end3D()
{
	if ( ! text_texture_mutex_11_ )
	{
		return;
	}

	text_texture_sync_key_++;

	DIRECT_X_FAIL_CHECK( text_texture_mutex_11_->ReleaseSync( text_texture_sync_key_ ) );
}

void Direct3D11::present()
{
	DIRECT_X_FAIL_CHECK( swap_chain_->Present( 0, 0 ) );
}

void Direct3D11::renderText()
{
	if ( ! font_ )
	{
		return;
	}

	get_sprite()->begin();

	EffectTechnique* technique = effect_->get_technique( "|sprite" );

	for ( const auto& pass : technique->get_pass_list() )
	{
		pass->apply();

		Sprite::Rect dst_rect( 0, 0, get_width(), get_height() );
		get_sprite()->draw( dst_rect, text_view_.get() );
	}

	get_sprite()->end();
}

/**
 * マルチサンプリングされたデプスバッファをマルチサンプリングされていないデプステクスチャにコピーする
 *
 */
void Direct3D11::resolve_depth_texture()
{
#if 0
	immediate_context_->ResolveSubresource( depth_texture_->get_texture_2d(), 0, depth_stencil_texture_, 0, DXGI_FORMAT_R32_FLOAT );
#endif
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
	std::unordered_map< D3D_FEATURE_LEVEL, std::string > feature_level_map;

	feature_level_map[ D3D_FEATURE_LEVEL_9_1 ] = "9.1";
	feature_level_map[ D3D_FEATURE_LEVEL_9_2 ] = "9.2";
	feature_level_map[ D3D_FEATURE_LEVEL_9_3 ] = "9.3";
	feature_level_map[ D3D_FEATURE_LEVEL_10_0 ] = "10.0";
	feature_level_map[ D3D_FEATURE_LEVEL_10_1 ] = "10.1";
	feature_level_map[ D3D_FEATURE_LEVEL_11_0 ] = "11.0";

	common::log( "log/d3d11.log", std::string( "created d3d11 device ( feature_level : " ) + feature_level_map[ device_->GetFeatureLevel() ] + " )" );
}

} // namespace core::graphics::direct_3d_11

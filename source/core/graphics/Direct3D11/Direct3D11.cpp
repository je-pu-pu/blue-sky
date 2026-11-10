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
#include "TextureReadback.h"
#include <core/graphics/DeviceLostException.h>

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/string.h>
#include <common/log.h>

#include <dxgi.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <string>

#include <iostream>

#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

namespace core::graphics::direct_3d_11
{

const Direct3D11::Color Direct3D11::DEFAULT_CLEAR_COLOR = Direct3D11::Color::Black;

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
{
	com_ptr< IDXGIFactory1 > dxgi_factory;
	DIRECT_X_FAIL_CHECK( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), reinterpret_cast< void** >( & dxgi_factory ) ) );

	log_all_adapter_desc( dxgi_factory.get() );

	DIRECT_X_FAIL_CHECK( dxgi_factory->EnumAdapters1( 0, & dxgi_adapter_ ) );

	create_device();

	log_feature_level();

	create_swap_chain( dxgi_factory.get(), hwnd, w, h, full_screen, multi_sample_count, multi_sample_quality );

	dxgi_factory.reset();

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

	if ( immediate_context_.get() )
	{
		immediate_context_->ClearState();
		immediate_context_->Flush();
	}

	input_layout_list_.clear();

	depth_texture_.reset();

	depth_stencil_view_.reset();
	depth_stencil_texture_.reset();

	back_buffer_texture_.reset();

	swap_chain_.reset();

	if ( false )
	{
		ID3D11Debug* debug_ = 0;

		device_->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast< void** >( & debug_ ) );
		debug_->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

		DIRECT_X_RELEASE( debug_ );
	}

	immediate_context_.reset();
	device_.reset();
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
		
	// スワップチェーン自体は MSAA なし・2 枚・FLIP_DISCARD にする（MSAA は BackBufferTexture が自前の描画先で行い、表示の前に解決する）
	// 以前はバッファ 1 枚・DISCARD・MSAA 付きの古い構成で、フリップモデル（ウィンドウでの遅延が少ない・合成のコピーがない）を使えなかった。
	// フリップモデルを作れない環境（古い Windows）では、以前の DISCARD に戻す
	DXGI_SWAP_CHAIN_DESC desc = swap_chain_desc_;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	if ( FAILED( dxgi_factory->CreateSwapChain( device_.get(), & desc, & swap_chain_ ) ) )
	{
		desc.BufferCount = 1;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		DIRECT_X_FAIL_CHECK( dxgi_factory->CreateSwapChain( device_.get(), & desc, & swap_chain_ ) );
	}

	swap_chain_buffer_count_ = desc.BufferCount;

	std::cout << "スワップチェーン : " << ( desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD ? "FLIP_DISCARD" : "DISCARD" )
		<< " バッファ " << desc.BufferCount << " 枚・MSAA " << swap_chain_desc_.SampleDesc.Count << '\n';

	// Alt+Enter は DXGI に任せず、アプリ側で処理する
	// （DXGI が勝手に切り替えると、ユーザーの操作かフォーカス喪失による解除かを区別できず、設定と実際の状態がずれる）
	DIRECT_X_FAIL_CHECK( dxgi_factory->MakeWindowAssociation( swap_chain_desc_.OutputWindow, DXGI_MWA_NO_ALT_ENTER ) );
}

/**
 * スワップチェインからバックバッファへの描画用テクスチャを作成する
 *
 */
void Direct3D11::create_back_buffer_texture()
{
	back_buffer_texture_.reset( new BackBufferTexture( this, swap_chain_.get(), swap_chain_desc_.SampleDesc ) );
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

	depth_stencil_view_.reset( create_depth_stencil_view( depth_stencil_texture_.get() ) );
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
		DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( depth_stencil_texture_.get(), & view_desc, & view ) );

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
 * スプライトをセットアップする
 *
 * この関数は、インプットレイアウトを作成した後、エフェクトファイルを読み込んだ後に呼び出す必要がある
 */
void Direct3D11::setup_sprite( const core::graphics::GraphicsManager* graphics_manager )
{
	sprite_.reset( new core::graphics::direct_3d_11::Sprite( this, graphics_manager ) );
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

	// position + normal
	D3D11_INPUT_ELEMENT_DESC layout_pos_norm[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "pos_norm", "|point_sprite", layout_pos_norm, ARRAYSIZE( layout_pos_norm ) );

	// line
	D3D11_INPUT_ELEMENT_DESC layout_pos_color[] =
    {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	create_input_layout( "pos_color", "|drawing_line", layout_pos_color, ARRAYSIZE( layout_pos_color ) );

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

	// 2D（画面全体の矩形・フェーダー・ポストエフェクト）: 位置と UV だけを読む（以前は "main" で法線も読んでいた）
	// 頂点バッファはメッシュと同じもの（位置・法線・UV）を使うので、UV はメッシュの頂点の中の位置から読み、法線は読み飛ばす
	D3D11_INPUT_ELEMENT_DESC layout_2d[] =
	{
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, mesh_vertex_tex_coord_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	create_input_layout( "2d", "|main2d", layout_2d, ARRAYSIZE( layout_2d ) );
}

void Direct3D11::create_input_layout( const char_t* input_layout_name, const char_t* teqhnique_name, D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	// すでに同じ名前でインプットレイアウトが作成済みであればスキップする
	const auto i = input_layout_list_.find( input_layout_name );
	
	if ( i != input_layout_list_.end() )
	{
		return;

		// delete i->second;
		// input_layout_list_.erase( i );
	}

	const auto* technique = effect_->get_technique( teqhnique_name );

	if ( ! technique || technique->get_pass_list().empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "technique not found for input layout : " ) + teqhnique_name );
	}

	auto* pass = static_cast< EffectPass* >( technique->get_pass_list().front().get() );

	input_layout_list_[ input_layout_name ] = std::make_unique< InputLayout >( pass->create_input_layout( layout, layout_array_size ) );
}

void Direct3D11::set_full_screen( bool full_screen )
{
	DIRECT_X_FAIL_CHECK( swap_chain_->SetFullscreenState( full_screen, 0 ) );
}

void Direct3D11::set_size( int w, int h )
{
	// サイズ変更の経路は on_resize の 1 本だけにする
	// （以前は ResizeBuffers だけを行う別の経路があり、バックバッファ・深度・ビューポートを作り直していなかった）
	on_resize( w, h );
}

bool Direct3D11::is_full_screen() const
{
	BOOL full_screen;

	DIRECT_X_FAIL_CHECK( swap_chain_.get()->GetFullscreenState( & full_screen, 0 ) );

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

	back_buffer_texture_.reset();

	swap_chain_desc_.BufferDesc.Width = w;
	swap_chain_desc_.BufferDesc.Height = h;

	DIRECT_X_FAIL_CHECK( swap_chain_->ResizeBuffers(
		swap_chain_buffer_count_,
		swap_chain_desc_.BufferDesc.Width,
		swap_chain_desc_.BufferDesc.Height,
		swap_chain_desc_.BufferDesc.Format,
		swap_chain_desc_.Flags
	) );

	create_back_buffer_texture();

	depth_stencil_view_.reset();
	depth_stencil_texture_.reset();

	create_depth_stencil_view();
	create_depth_texture();

	setup_default_viewport();

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
	clear_depth_stencil_view( depth_stencil_view_.get() );
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
	immediate_context_->OMSetRenderTargets( 1, views, with_depth_stencil ? depth_stencil_view_.get() : nullptr );
}

/**
 * 通常のビューポートを設定する
 *
 */
void Direct3D11::set_default_viewport()
{
	immediate_context_->RSSetViewports( 1, & viewport_ );
}

Pixel2DSpace Direct3D11::get_2d_space() const
{
	// 今の描画先の大きさ（ビューポート）。VR の目ごとの描画などで、画面と違う大きさの描画先に描くときに合わせる
	UINT count = 1;
	D3D11_VIEWPORT viewport = { };
	immediate_context_.get()->RSGetViewports( & count, & viewport );

	if ( count >= 1 && viewport.Width > 0.f && viewport.Height > 0.f )
	{
		return Pixel2DSpace{ viewport.Width, viewport.Height, offset_2d_x_ };
	}

	return Pixel2DSpace{ static_cast< float_t >( get_width() ), static_cast< float_t >( get_height() ), offset_2d_x_ };
}

bool Direct3D11::save_back_buffer_to_png( const wchar_t* file_name )
{
	// 描画先（MSAA なら解決してから）を保存する。フリップモデルのスワップチェーンのバッファは、表示の後は中身が決まらない
	if ( ! back_buffer_texture_ )
	{
		return false;
	}

	return save_texture_2d_to_png( device_.get(), immediate_context_.get(), back_buffer_texture_->get_texture_2d(), file_name );
}

void Direct3D11::reset_render_state()
{
	immediate_context_->OMSetBlendState( nullptr, nullptr, 0xFFFFFFFF );
	immediate_context_->OMSetDepthStencilState( nullptr, 0 );
	immediate_context_->RSSetState( nullptr );

	immediate_context_->GSSetShader( nullptr, nullptr, 0 );
	immediate_context_->HSSetShader( nullptr, nullptr, 0 );
	immediate_context_->DSSetShader( nullptr, nullptr, 0 );

	// 描画先にしたテクスチャが読み込み側に残っていると、描画先に結び付けられない（D3D11 が黙って外す）
	ID3D11ShaderResourceView* null_views[ D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT ] = { };
	immediate_context_->VSSetShaderResources( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, null_views );
	immediate_context_->PSSetShaderResources( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, null_views );

	set_default_render_target();
	set_default_viewport();
}

/**
 * レンダリング結果を書き込むテクスチャを設定する
 *
 * @param texture レンダリング結果を書き込むテクスチャ
 */
void Direct3D11::set_render_target( RenderTargetTexture* texture )
{
	ID3D11RenderTargetView* views[] = { texture->get_render_target_view() };
	immediate_context_->OMSetRenderTargets( 1, views, depth_stencil_view_.get() );
}

void Direct3D11::set_render_target( BackBufferTexture* texture )
{
	ID3D11RenderTargetView* views[] = { texture->get_render_target_view() };
	immediate_context_->OMSetRenderTargets( 1, views, depth_stencil_view_.get() );
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
	D3D11_VIEWPORT viewport = {
		.TopLeftX = x,
		.TopLeftY = y,
		.Width = w,
		.Height = h,
		.MinDepth = d_min,
		.MaxDepth = d_max
	};

	immediate_context_->RSSetViewports( 1, & viewport );
}

void Direct3D11::set_input_layout( const char* name )
{
	set_input_layout( input_layout_list_[ name ].get() );
}

const Direct3D11::InputLayout* Direct3D11::get_input_layout( const char* name ) const
{
	auto i = input_layout_list_.find( name );

	if ( i == input_layout_list_.end() )
	{
		return nullptr;
	}

	return i->second.get();
}

void Direct3D11::set_input_layout( const InputLayout* input_layout )
{
	immediate_context_->IASetInputLayout( input_layout->get_input_layout() );
}

Direct3D11::Texture* Direct3D11::load_texture( const char* file_path ) 
{
	ID3D11ShaderResourceView* view = 0;

	std::wstring ws_file_name = common::convert_to_wstring( file_path );
	
	if ( FAILED( DirectX::CreateWICTextureFromFileEx( device_.get(), immediate_context_.get(), ws_file_name.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, nullptr, & view ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "file open failed. " ) + file_path );
	}

	// immediate_context_->GenerateMips( view );

	return new Texture( this, view );
}

Direct3D11::Texture* Direct3D11::load_texture_premultiplied( const char* file_path )
{
	return create_texture_from_image( decode_image_premultiplied( file_path ), file_path );
}

Direct3D11::DecodedImage Direct3D11::decode_image_premultiplied( const char* file_path )
{
	const std::wstring ws_file_name = common::convert_to_wstring( file_path );

	const auto fail = [ file_path ] ( const char* step, HRESULT hr )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "UI texture load failed ( " ) + step + " : " + common::serialize( static_cast< unsigned long >( hr ) ) + " ). " + file_path );
	};

	// WIC のファクトリーは呼び出しごとに作る（作業スレッドからも呼べるように。以前はメインスレッド前提で使い回していた）
	Microsoft::WRL::ComPtr< IWICImagingFactory > factory;
	HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( factory.GetAddressOf() ) );
	if ( FAILED( hr ) ) { fail( "factory", hr ); }

	Microsoft::WRL::ComPtr< IWICBitmapDecoder > decoder;
	hr = factory->CreateDecoderFromFilename( ws_file_name.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf() );
	if ( FAILED( hr ) ) { fail( "open", hr ); }

	Microsoft::WRL::ComPtr< IWICBitmapFrameDecode > frame;
	hr = decoder->GetFrame( 0, frame.GetAddressOf() );
	if ( FAILED( hr ) ) { fail( "frame", hr ); }

	UINT width = 0;
	UINT height = 0;
	hr = frame->GetSize( & width, & height );
	if ( FAILED( hr ) ) { fail( "size", hr ); }

	// 機能レベル 10.x の上限（8192）に合わせる（UI の画像でそれより大きいものは想定しない）
	constexpr UINT MAX_UI_TEXTURE_SIZE = 8192;

	if ( width == 0 || height == 0 || width > MAX_UI_TEXTURE_SIZE || height > MAX_UI_TEXTURE_SIZE )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "UI texture load failed ( unsupported size " ) + std::to_string( width ) + "x" + std::to_string( height ) + ", max " + std::to_string( MAX_UI_TEXTURE_SIZE ) + " ). " + file_path );
	}

	// どの形式の画像でも 32bit RGBA（ストレート）に変換して取り出し、自前で乗算済みにする
	// （WIC の 32bppPRGBA への変換は丸め方が実装依存なので使わない）
	Microsoft::WRL::ComPtr< IWICFormatConverter > converter;
	hr = factory->CreateFormatConverter( converter.GetAddressOf() );
	if ( FAILED( hr ) ) { fail( "converter", hr ); }

	hr = converter->Initialize( frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom );
	if ( FAILED( hr ) ) { fail( "convert", hr ); }

	DecodedImage image;
	image.width = width;
	image.height = height;

	const UINT row_pitch = width * 4;
	image.pixels.resize( static_cast< size_t >( row_pitch ) * height );

	hr = converter->CopyPixels( nullptr, row_pitch, static_cast< UINT >( image.pixels.size() ), image.pixels.data() );
	if ( FAILED( hr ) ) { fail( "copy", hr ); }

	for ( size_t i = 0; i < image.pixels.size(); i += 4 )
	{
		const unsigned a = image.pixels[ i + 3 ];

		for ( size_t c = 0; c < 3; c++ )
		{
			image.pixels[ i + c ] = static_cast< uint8_t >( ( image.pixels[ i + c ] * a + 127 ) / 255 );
		}
	}

	return image;
}

Direct3D11::Texture* Direct3D11::create_texture_from_image( const DecodedImage& image, const char* name_for_error )
{
	const auto fail = [ name_for_error ] ( const char* step, HRESULT hr )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "UI texture create failed ( " ) + step + " : " + common::serialize( static_cast< unsigned long >( hr ) ) + " ). " + ( name_for_error ? name_for_error : "" ) );
	};

	D3D11_TEXTURE2D_DESC desc = { };
	desc.Width = image.width;
	desc.Height = image.height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data = { };
	data.pSysMem = image.pixels.data();
	data.SysMemPitch = image.width * 4;

	Microsoft::WRL::ComPtr< ID3D11Texture2D > texture;
	HRESULT hr = device_->CreateTexture2D( & desc, & data, texture.GetAddressOf() );
	if ( FAILED( hr ) ) { fail( "texture", hr ); }

	ID3D11ShaderResourceView* view = nullptr;
	hr = device_->CreateShaderResourceView( texture.Get(), nullptr, & view );
	if ( FAILED( hr ) ) { fail( "view", hr ); }

	Texture* result = new Texture( this, view );
	result->set_premultiplied( true );		// 展開のときに乗算済みにしている
	return result;
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


void Direct3D11::present()
{
	if ( back_buffer_texture_ )
	{
		back_buffer_texture_->resolve( immediate_context_.get() );
	}

	// フリップモデルでは表示のあと描画先が外れる。次のフレームの頭で reset_render_state が結び直す
	const HRESULT hr = swap_chain_->Present( vsync_ ? 1 : 0, 0 );

	// デバイスロスト（ドライバの更新・GPU のタイムアウト・GPU の切り替え）は専用の例外で知らせる。
	// WinMain が受け取り、ゲームを起動し直して復旧する（GPU のリソースは各所が持っていて、その場では作り直せない）
	if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
	{
		const HRESULT reason = device_->GetDeviceRemovedReason();

		char text[ 16 ];
		sprintf_s( text, "0x%08lX", static_cast< unsigned long >( reason ) );

		throw DeviceLostException( __FILE__, __LINE__, std::string( "GPU のデバイスが失われました（ドライバの更新、GPU のタイムアウト、GPU の切り替えなど）。理由: " ) + text );
	}

	// DXGI_STATUS_OCCLUDED（最小化中など）は成功の扱い
	DIRECT_X_FAIL_CHECK( hr );
}

/**
 * マルチサンプリングされたデプスバッファをマルチサンプリングされていないデプステクスチャにコピーする
 *
 */
void Direct3D11::resolve_depth_texture()
{
#if 0
	immediate_context_->ResolveSubresource( depth_texture_->get_texture_2d(), 0, depth_stencil_texture_.get(), 0, DXGI_FORMAT_R32_FLOAT );
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

	// ログの出口をそろえる（以前は log/adapter_info.log に別に書いていた）。std::cout は GameMain が debug_log へ流す
	std::cout << "GPU の情報 :" << std::endl << common::convert_to_string( text ) << std::endl;
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

	std::cout << "Direct3D 11 のデバイスを作りました（機能レベル : " << feature_level_map[ device_->GetFeatureLevel() ] << "）" << std::endl;
}

std::vector< Direct3D11::DisplayMode > Direct3D11::get_available_display_modes() const
{
	std::vector< DisplayMode > result;

	IDXGIOutput* output = nullptr;

	if ( FAILED( const_cast< IDXGIAdapter1* >( dxgi_adapter_.get() )->EnumOutputs( 0, & output ) ) || ! output )
	{
		return result;
	}

	UINT mode_count = 0;
	output->GetDisplayModeList( swap_chain_desc_.BufferDesc.Format, 0, & mode_count, nullptr );

	if ( mode_count > 0 )
	{
		std::vector< DXGI_MODE_DESC > modes( mode_count );
		output->GetDisplayModeList( swap_chain_desc_.BufferDesc.Format, 0, & mode_count, modes.data() );

		for ( const auto& mode : modes )
		{
			DisplayMode dm = { static_cast< int >( mode.Width ), static_cast< int >( mode.Height ) };

			// 重複除去（同じ解像度でリフレッシュレート違いが複数返る）
			bool is_duplicate = false;

			for ( const auto& existing : result )
			{
				if ( existing.width == dm.width && existing.height == dm.height )
				{
					is_duplicate = true;
					break;
				}
			}

			if ( ! is_duplicate )
			{
				result.push_back( dm );
			}
		}
	}

	DIRECT_X_RELEASE( output );

	return result;
}

} // namespace core::graphics::direct_3d_11

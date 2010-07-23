#include "DirectShow.h"
#include "DirectX.h"
#include <d3dx9.h>
#include <vmr9.h>

#pragma comment(lib, "strmiids.lib")

DirectShow::DirectShow( HWND hwnd )
	: graph_builder_( 0 )
	, capture_graph_builder_( 0 )
	, video_mixing_renderer_( 0 )
	, media_control_( 0 )
{
	DIRECT_X_FAIL_CHECK( CoInitializeEx( 0, COINIT_APARTMENTTHREADED ) );

	DIRECT_X_FAIL_CHECK( CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast< void** >( & graph_builder_ ) ) );
	DIRECT_X_FAIL_CHECK( CoCreateInstance( CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast< void** >( & video_mixing_renderer_ ) ) );
	DIRECT_X_FAIL_CHECK( graph_builder_->AddFilter( video_mixing_renderer_, L"video_mixing_renderer" ) );

	wchar_t* file_name = L"media/movie/sins_op.mpa";	

	IVMRFilterConfig* vmr_config = 0;
	DIRECT_X_FAIL_CHECK( video_mixing_renderer_->QueryInterface( IID_IVMRFilterConfig9, reinterpret_cast< void** >( & vmr_config ) ) );
	DIRECT_X_FAIL_CHECK( vmr_config->SetRenderingMode( VMRMode_Windowless ) );
	vmr_config->Release();

	IVMRWindowlessControl9* windowless_conterol = 0;
	DIRECT_X_FAIL_CHECK( video_mixing_renderer_->QueryInterface( IID_IVMRWindowlessControl9, reinterpret_cast< void** >( & windowless_conterol ) ) );
    DIRECT_X_FAIL_CHECK( windowless_conterol->SetVideoClippingWindow( hwnd ) );

	DIRECT_X_FAIL_CHECK( graph_builder_->RenderFile( file_name, 0 ) );

	/*
	bool full_screen = false;

	// if ( ! full_screen )
	// {
		IVMRFilterConfig* vmr_config = 0;
		DIRECT_X_FAIL_CHECK( video_mixing_renderer_->QueryInterface( IID_IVMRFilterConfig9, reinterpret_cast< void** >( & vmr_config ) ) );
		DIRECT_X_FAIL_CHECK( vmr_config->SetRenderingMode( VMRMode_Windowless ) );
		vmr_config->Release();

		IVMRWindowlessControl9* windowless_conterol = 0;
		DIRECT_X_FAIL_CHECK( video_mixing_renderer_->QueryInterface( IID_IVMRWindowlessControl9, reinterpret_cast< void** >( & windowless_conterol ) ) );
        DIRECT_X_FAIL_CHECK( windowless_conterol->SetVideoClippingWindow( hwnd ) );
	// }

	

	IBaseFilter* source_filter = 0;
	DIRECT_X_FAIL_CHECK( graph_builder_->AddSourceFilter( file_name, file_name, & source_filter ) );
	
	DIRECT_X_FAIL_CHECK( CoCreateInstance( CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, reinterpret_cast< void** >( & capture_graph_builder_ ) ) );
	capture_graph_builder_->SetFiltergraph( graph_builder_ ); 

	DIRECT_X_FAIL_CHECK( capture_graph_builder_->RenderStream( 0, 0, source_filter, 0, video_mixing_renderer_ ) );
	DIRECT_X_FAIL_CHECK( capture_graph_builder_->RenderStream( 0, & MEDIATYPE_Audio, source_filter, 0, 0 ) );
	*/

	RECT src_rect = { 0 };
	DIRECT_X_FAIL_CHECK( windowless_conterol->GetNativeVideoSize( & src_rect.right, & src_rect.bottom, 0, 0 ) );
    
	RECT dst_rect = { 0 };
	GetClientRect( hwnd, & dst_rect );
	
	DIRECT_X_FAIL_CHECK( windowless_conterol->SetVideoPosition( & src_rect, & dst_rect ) );
	windowless_conterol->Release();


	DIRECT_X_FAIL_CHECK( graph_builder_->QueryInterface( IID_IMediaControl, reinterpret_cast< void** >( & media_control_ ) ) );

	media_control_->Run();
}

DirectShow::~DirectShow()
{
	if ( media_control_ ) media_control_->Release();
	if ( capture_graph_builder_ ) capture_graph_builder_->Release();
	if ( video_mixing_renderer_ ) video_mixing_renderer_->Release();
	if ( graph_builder_ ) graph_builder_->Release();

	CoUninitialize();
}

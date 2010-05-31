#include "Direct3D9.h"

#include <common/exception.h>

#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )

Direct3D9::Direct3D9( HWND hwnd )
	: direct_3d_( 0 )
	, device_( 0 )
{
	direct_3d_ = Direct3DCreate9( D3D_SDK_VERSION );

	if ( direct_3d_ == NULL )
	{
		throw "";
	}

	DWORD behavior_flag = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	D3DPRESENT_PARAMETERS present = { 0 };
    present.Windowed = TRUE;
    present.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present.BackBufferFormat = D3DFMT_UNKNOWN;
	present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	present.EnableAutoDepthStencil = TRUE;
	present.AutoDepthStencilFormat = D3DFMT_D16;

	if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, behavior_flag, & present, & device_ ) ) )
	{
		direct_3d_->Release();

		COMMON_THROW_EXCEPTION;
	}
}

Direct3D9::~Direct3D9()
{
	device_->Release();
	direct_3d_->Release();
}
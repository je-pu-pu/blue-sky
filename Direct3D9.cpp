#include "Direct3D9.h"

#include <common/exception.h>

#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )

#define PREF_HUD

Direct3D9::Direct3D9( HWND hwnd )
	: direct_3d_( 0 )
	, device_( 0 )
{
	direct_3d_ = Direct3DCreate9( D3D_SDK_VERSION );

	if ( direct_3d_ == NULL )
	{
		COMMON_THROW_EXCEPTION;
	}

//	DWORD behavior_flag = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	D3DPRESENT_PARAMETERS present = { 0 };
    present.Windowed = TRUE;
    present.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present.BackBufferFormat = D3DFMT_UNKNOWN;
	present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	present.EnableAutoDepthStencil = TRUE;
	present.AutoDepthStencilFormat = D3DFMT_D16;
//	present.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;

#ifdef PREF_HUD
	for ( int n = 0; n < direct_3d_->GetAdapterCount(); n++ )
	{
		D3DADAPTER_IDENTIFIER9 adapter_identifier;

		direct_3d_->GetAdapterIdentifier( n, 0, & adapter_identifier );
		
		if ( strstr( adapter_identifier.Description, "PerfHUD" ) != 0 )
		{
			if ( FAILED( direct_3d_->CreateDevice( n, D3DDEVTYPE_REF, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, & present, & device_ ) ) )
			{
				direct_3d_->Release();

				COMMON_THROW_EXCEPTION;
			}

			break;
		}
	}
#endif // PREF_HUD
	
	if ( ! device_ )
	{
		if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, & present, & device_ ) ) )
		{
			if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, & present, & device_ ) ) )
			{
				if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, & present, & device_ ) ) )
				{
					direct_3d_->Release();

					COMMON_THROW_EXCEPTION;
				}
			}
		}
	}

	device_->GetDeviceCaps( & device_caps_ );

	int w = device_caps_.MaxTextureWidth;
	int h = device_caps_.MaxTextureHeight;

	if ( device_caps_.FVFCaps & D3DFVFCAPS_PSIZE )
	{
		//
		int x = 0;
	}


}

Direct3D9::~Direct3D9()
{
	device_->Release();
	direct_3d_->Release();
}
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

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

	text_out_adapter_info( "d3d_adapter_info.txt" );

//	DWORD behavior_flag = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	ZeroMemory( & present_, sizeof( present_ ) );

	present_.SwapEffect = D3DSWAPEFFECT_FLIP;
	present_.BackBufferFormat = D3DFMT_X8R8G8B8;
	present_.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	present_.BackBufferWidth = 720;
	present_.BackBufferHeight = 480;
	
	present_.Windowed = TRUE;
//	present_.SwapEffect = D3DSWAPEFFECT_DISCARD;
//	present_.BackBufferFormat = D3DFMT_UNKNOWN;
//	present_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	present_.EnableAutoDepthStencil = TRUE;
	present_.AutoDepthStencilFormat = D3DFMT_D16;
//	present_.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;

	DWORD multi_sample_quality = 0;
	if ( SUCCEEDED( direct_3d_->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A32B32G32R32F, true, D3DMULTISAMPLE_2_SAMPLES, & multi_sample_quality ) ) )
	{
		// present_.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
		// present_.MultiSampleQuality = multi_sample_quality - 1;
	}

#ifdef PREF_HUD
	for ( UINT n = 0; n < direct_3d_->GetAdapterCount(); n++ )
	{
		D3DADAPTER_IDENTIFIER9 adapter_identifier;

		direct_3d_->GetAdapterIdentifier( n, 0, & adapter_identifier );
		
		if ( strstr( adapter_identifier.Description, "PerfHUD" ) != 0 )
		{
			if ( FAILED( direct_3d_->CreateDevice( n, D3DDEVTYPE_REF, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, & present_, & device_ ) ) )
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
		HRESULT hr = 0;

		if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, & present_, & device_ ) ) )
		{
			if ( FAILED( direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, & present_, & device_ ) ) )
			{
				if ( FAILED( hr = direct_3d_->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, & present_, & device_ ) ) )
				{
					direct_3d_->Release();

					DIRECT_X_FAIL_CHECK( hr );
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

	device_->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
}

Direct3D9::~Direct3D9()
{
	device_->Release();
	direct_3d_->Release();
}

void Direct3D9::toggle_full_screen()
{
	present_.Windowed = ! present_.Windowed;

	device_->Reset( & present_ );
}

void Direct3D9::text_out_adapter_info( const char* file_name, bool append )
{
	std::string info_text;

	for ( UINT n = 0; n < direct_3d_->GetAdapterCount(); n++ )
	{
		D3DADAPTER_IDENTIFIER9 adapter_identifier;

		if ( SUCCEEDED( direct_3d_->GetAdapterIdentifier( n, 0, & adapter_identifier ) ) )
		{
			info_text += "adapter : " + common::serialize( n ) + "\n" +
				"\tDriver : " + adapter_identifier.Driver + "\n" +
				"\tDescription : " + adapter_identifier.Description + "\n" +
				"\tDeviceName : " + adapter_identifier.DeviceName + "\n" +
				"\tDriverVersion : " + common::serialize( adapter_identifier.DriverVersion.QuadPart ) + "\n" +
				"\tDriverVersionLowPart : " + common::serialize( adapter_identifier.DriverVersion.LowPart ) + "\n" +
				"\tDriverVersionHighPart : " + common::serialize( adapter_identifier.DriverVersion.HighPart ) + "\n" +
				"\tVendorId : " + common::serialize( adapter_identifier.VendorId ) + "\n" +
				"\tDeviceId : " + common::serialize( adapter_identifier.DeviceId ) + "\n" +
				"\tSubSysId : " + common::serialize( adapter_identifier.SubSysId ) + "\n" +
				"\tRevision : " + common::serialize( adapter_identifier.Revision ) + "\n" +
//				"\tDeviceIdentifier : " + common::serialize( adapter_identifier.DeviceIdentifier ) + "\n" +
				"\tWHQLLevel : " + common::serialize( adapter_identifier.WHQLLevel ) + "\n" +
				"-----\n";
		}
	}

	common::log( file_name, info_text, append );
}
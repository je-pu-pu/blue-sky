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

	ZeroMemory( & present_, sizeof( present_ ) );

	present_.SwapEffect = D3DSWAPEFFECT_FLIP;
	present_.BackBufferFormat = D3DFMT_X8R8G8B8;
//	present_.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	present_.BackBufferWidth = 720;
	present_.BackBufferHeight = 480;
	
	present_.Windowed = TRUE;
	present_.SwapEffect = D3DSWAPEFFECT_DISCARD;
//	present_.BackBufferFormat = D3DFMT_UNKNOWN;
//	present_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	present_.EnableAutoDepthStencil = TRUE;
	present_.AutoDepthStencilFormat = D3DFMT_D16;
//	present_.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;

	DWORD multi_sample_quality = 0;
	if ( SUCCEEDED( direct_3d_->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_8_SAMPLES, & multi_sample_quality ) ) )
	{
		present_.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
		present_.MultiSampleQuality = multi_sample_quality - 1;
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

	text_out_device_caps( "d3d_device_caps.txt" );

	device_->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
}

Direct3D9::~Direct3D9()
{
	device_->Release();
	direct_3d_->Release();
}

void Direct3D9::set_full_screen( bool full_scrren )
{
	present_.Windowed = ! full_scrren;

	DIRECT_X_FAIL_CHECK( device_->Reset( & present_ ) );
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

void Direct3D9::text_out_device_caps( const char* file_name, bool append )
{
#define device_caps_string( name ) std::string( "\t" ) + # name + " : " + common::serialize( device_caps_.name ) + "\n"

	std::string info_text = "device caps :\n" +
		device_caps_string( DeviceType ) +
		device_caps_string( AdapterOrdinal ) +

		device_caps_string( Caps ) +
		device_caps_string( Caps2 ) +
		device_caps_string( Caps3 ) +
		device_caps_string( PresentationIntervals ) +

		device_caps_string( CursorCaps ) +

	    device_caps_string( DevCaps ) +

		device_caps_string( PrimitiveMiscCaps ) +
		device_caps_string( RasterCaps ) +
		device_caps_string( ZCmpCaps ) +
		device_caps_string( SrcBlendCaps ) +
		device_caps_string( DestBlendCaps ) +
		device_caps_string( AlphaCmpCaps ) +
		device_caps_string( ShadeCaps ) +
		device_caps_string( TextureCaps ) +
		device_caps_string( TextureFilterCaps ) +
		device_caps_string( CubeTextureFilterCaps ) +
		device_caps_string( VolumeTextureFilterCaps ) +
		device_caps_string( TextureAddressCaps ) +
		device_caps_string( VolumeTextureAddressCaps ) +

		device_caps_string( LineCaps ) +

		device_caps_string( MaxTextureWidth ) +
		device_caps_string( MaxTextureHeight ) +
		device_caps_string( MaxVolumeExtent ) +

		device_caps_string( MaxTextureRepeat ) +
		device_caps_string( MaxTextureAspectRatio ) +
		device_caps_string( MaxAnisotropy ) +
		device_caps_string( MaxVertexW ) +

		device_caps_string( GuardBandLeft ) +
		device_caps_string( GuardBandTop ) +
		device_caps_string( GuardBandRight ) +
		device_caps_string( GuardBandBottom ) +

		device_caps_string( ExtentsAdjust ) +
		device_caps_string( StencilCaps ) +

		device_caps_string( FVFCaps ) +
		device_caps_string( TextureOpCaps ) +
		device_caps_string( MaxTextureBlendStages ) +
		device_caps_string( MaxSimultaneousTextures ) +

		device_caps_string( VertexProcessingCaps ) +
		device_caps_string( MaxActiveLights ) +
		device_caps_string( MaxUserClipPlanes ) +
		device_caps_string( MaxVertexBlendMatrices ) +
		device_caps_string( MaxVertexBlendMatrixIndex ) +

		device_caps_string( MaxPointSize ) +

		device_caps_string( MaxPrimitiveCount ) +
		device_caps_string( MaxVertexIndex ) +
		device_caps_string( MaxStreams ) +
		device_caps_string( MaxStreamStride ) +

		device_caps_string( VertexShaderVersion ) +
		device_caps_string( MaxVertexShaderConst ) +

		device_caps_string( PixelShaderVersion ) +
		device_caps_string( PixelShader1xMaxValue ) +

		device_caps_string( DevCaps2 ) +

		device_caps_string( MaxNpatchTessellationLevel ) +
		device_caps_string( Reserved5 ) +

		device_caps_string( MasterAdapterOrdinal ) +
		device_caps_string( AdapterOrdinalInGroup ) +
		device_caps_string( NumberOfAdaptersInGroup ) +
		device_caps_string( DeclTypes ) +
		device_caps_string( NumSimultaneousRTs ) +
		device_caps_string( StretchRectFilterCaps ) +
//		device_caps_string( VS20Caps ) +
//		device_caps_string( PS20Caps ) +
		device_caps_string( VertexTextureFilterCaps ) +
		device_caps_string( MaxVShaderInstructionsExecuted ) +
		device_caps_string( MaxPShaderInstructionsExecuted ) +
		device_caps_string( MaxVertexShader30InstructionSlots ) +
		device_caps_string( MaxPixelShader30InstructionSlots );

	common::log( file_name, info_text, append );
}
#include "Direct3D9.h"
#include "Direct3D9Resource.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

#pragma comment( lib, "d3d9.lib" )

#ifdef _DEBUG
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#define PREF_HUD

Direct3D9::Direct3D9( HWND hwnd, int w, int h, bool full_screen, int multi_sample_type, int multi_sample_quality )
	: direct_3d_( 0 )
	, device_( 0 )
	, effect_( 0 )
	, sprite_( 0 )
	, texture_manager_( 0 )
{
	direct_3d_ = Direct3DCreate9( D3D_SDK_VERSION );

	if ( direct_3d_ == NULL )
	{
		COMMON_THROW_EXCEPTION;
	}

	text_out_adapter_info( "d3d_adapter_info.txt" );

	ZeroMemory( & present_, sizeof( present_ ) );

//	present_.hDeviceWindow = hwnd;
	present_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	present_.BackBufferFormat = D3DFMT_X8R8G8B8;
	present_.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	present_.BackBufferWidth = w;
	present_.BackBufferHeight = h;
	
	present_.Windowed = ! full_screen;
//	present_.BackBufferFormat = D3DFMT_UNKNOWN;
//	present_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	present_.EnableAutoDepthStencil = TRUE;
	present_.AutoDepthStencilFormat = D3DFMT_D16;
//	present_.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;

	DWORD max_multi_sample_quality = 0;
	if ( SUCCEEDED( direct_3d_->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, present_.Windowed, static_cast< D3DMULTISAMPLE_TYPE >( multi_sample_type ), & max_multi_sample_quality ) ) )
	{
		if ( multi_sample_quality >= static_cast< int >( max_multi_sample_quality ) )
		{
			multi_sample_quality = max_multi_sample_quality - 1;
		}

		present_.MultiSampleType = static_cast< D3DMULTISAMPLE_TYPE >( multi_sample_type );
		present_.MultiSampleQuality = multi_sample_quality;
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

	// device_->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );

	DIRECT_X_FAIL_CHECK( D3DXCreateSprite( device_, & sprite_ ) );

	texture_manager_ = new Direct3D9TextureManager( this );
}

Direct3D9::~Direct3D9()
{
	if ( texture_manager_ ) delete texture_manager_;

	if ( sprite_ ) sprite_->Release();
	if ( effect_ ) effect_->Release();
	if ( device_ ) device_->Release();
	if ( direct_3d_ ) direct_3d_->Release();
}

void Direct3D9::load_effect_file( const char* file_name )
{
	LPD3DXBUFFER error_message_buffer = 0;

	HRESULT hr = D3DXCreateEffectFromFile( getDevice(), file_name, 0, 0, 0, 0, & effect_, & error_message_buffer );

	if ( FAILED( hr ) )
	{
		if ( error_message_buffer )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_message_buffer->GetBufferPointer() ) );
		}
		else
		{
			DIRECT_X_FAIL_CHECK( hr );
		}
	}
}

void Direct3D9::reset()
{
	HRESULT hr = device_->TestCooperativeLevel();

	if ( hr == D3DERR_DEVICENOTRESET )
	{
		DIRECT_X_FAIL_CHECK( effect_->OnLostDevice() );
		DIRECT_X_FAIL_CHECK( sprite_->OnLostDevice() );

		for ( ResourceList::iterator i = resource_list_.begin(); i != resource_list_.end(); ++i )
		{
			(*i)->on_lost_device();
		}

		if ( SUCCEEDED( device_->Reset( & present_ ) ) )
		{
			DIRECT_X_FAIL_CHECK( effect_->OnResetDevice() );
			DIRECT_X_FAIL_CHECK( sprite_->OnResetDevice() );
			
			for ( ResourceList::iterator i = resource_list_.begin(); i != resource_list_.end(); ++i )
			{
				(*i)->on_reset_device();
			}

			// COMMON_THROW_EXCEPTION_MESSAGE( "reset OK !!!" );
		}
	}
}

void Direct3D9::set_full_screen( bool full_scrren )
{
	present_.Windowed = ! full_scrren;

	HRESULT hr = device_->TestCooperativeLevel();

	if ( hr == D3DERR_DEVICELOST )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( effect_->OnLostDevice() );
	DIRECT_X_FAIL_CHECK( sprite_->OnLostDevice() );

	for ( ResourceList::iterator i = resource_list_.begin(); i != resource_list_.end(); ++i )
	{
		(*i)->on_lost_device();
	}

	DIRECT_X_FAIL_CHECK( device_->Reset( & present_ ) );

	DIRECT_X_FAIL_CHECK( effect_->OnResetDevice() );
	DIRECT_X_FAIL_CHECK( sprite_->OnResetDevice() );

	for ( ResourceList::iterator i = resource_list_.begin(); i != resource_list_.end(); ++i )
	{
		(*i)->on_reset_device();
	}
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

	info_text += std::string( "\n" ) + "shader :\n" +
		"\t" + D3DXGetVertexShaderProfile( getDevice() ) + "\n" +
		"\t" + D3DXGetPixelShaderProfile( getDevice() ) + "\n";

	common::log( file_name, info_text, append );
}
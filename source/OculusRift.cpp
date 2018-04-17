#include "OculusRift.h"
#include "Direct3D11Vector.h"
#include "Direct3D11Matrix.h"

#include <common/exception.h>
#include <Extras/OVR_Math.h>
#include <algorithm>
#include <vector>

#include <iostream>

#include "memory.h"

#pragma comment( lib, "LibOVR.lib" )

#define OCULUS_VR_FAIL_CHECK( x ) { if ( OVR_FAILURE( x ) ) { ovrErrorInfo errorInfo; ovr_GetLastErrorInfo( & errorInfo ); COMMON_THROW_EXCEPTION_MESSAGE( std::string( #x ) + " failed. : " + errorInfo.ErrorString ); } }

const Direct3D11::Matrix OculusRift::COORDINATE_SYSTEM_CONVERT_MATRIX = [] () { Direct3D11::Matrix m; m.set_scaling( 1, 1, -1 ); return m; }();

OculusRift::OculusRift( Direct3D11* d3d )
	: direct_3d_( d3d )
	, yaw_( 0.f )
	, pitch_( 0.f )
	, roll_( 0.f )
	, last_yaw_( 0.f )
	, last_pitch_( 0.f )
	, last_roll_( 0.f )
{
	OCULUS_VR_FAIL_CHECK( ovr_Initialize( nullptr ) );
	OCULUS_VR_FAIL_CHECK( ovr_Create( &ovr_session_, & ovr_luid_ ) );

	hmd_desc_ = ovr_GetHmdDesc( ovr_session_ );


	ovrSizei size_l = ovr_GetFovTextureSize( ovr_session_, ovrEye_Left,  hmd_desc_.DefaultEyeFov[ 0 ], 1.f );
	ovrSizei size_r = ovr_GetFovTextureSize( ovr_session_, ovrEye_Right, hmd_desc_.DefaultEyeFov[ 1 ], 1.f );
	
	ovrSizei size;
	size.w = size_l.w + size_r.w;
	size.h = std::max( size_l.h, size_r.h );

	create_render_target_texture_swap_chain( size );
	create_depth_stencil_texture_swap_chain( size );

	create_render_target_view();
	create_depth_stencil_view();

	create_layer( size_l, size_r );
}

OculusRift::~OculusRift()
{
	ovr_DestroyTextureSwapChain( ovr_session_, render_target_texture_swap_chain_ );
	ovr_DestroyTextureSwapChain( ovr_session_, depth_stencil_texture_swap_chain_ );
	ovr_Destroy( ovr_session_ );
	ovr_Shutdown();
}

void OculusRift::create_render_target_texture_swap_chain( const ovrSizei& size )
{
	ovrTextureSwapChainDesc desc = { };
	desc.Type			= ovrTexture_2D;
	desc.Format			= OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ArraySize		= 1;
	desc.Width			= size.w;
	desc.Height			= size.h;
	desc.MipLevels		= 1;
	desc.SampleCount	= 1;
	desc.StaticImage	= ovrFalse;
	desc.MiscFlags		= ovrTextureMisc_None;
	desc.BindFlags		= ovrTextureBind_DX_RenderTarget;

	OCULUS_VR_FAIL_CHECK( ovr_CreateTextureSwapChainDX( ovr_session_, direct_3d_->getDevice(), & desc, & render_target_texture_swap_chain_ ) );
}

void OculusRift::create_depth_stencil_texture_swap_chain( const ovrSizei& size )
{
	ovrTextureSwapChainDesc desc = { };
	desc.Type			= ovrTexture_2D;
	desc.Format			= OVR_FORMAT_D32_FLOAT;
	desc.ArraySize		= 1;
	desc.Width			= size.w;
	desc.Height			= size.h;
	desc.MipLevels		= 1;
	desc.SampleCount	= 1;
	desc.StaticImage	= ovrFalse;
	desc.MiscFlags		= ovrTextureMisc_None;
	desc.BindFlags		= ovrTextureBind_DX_DepthStencil;

	OCULUS_VR_FAIL_CHECK( ovr_CreateTextureSwapChainDX( ovr_session_, direct_3d_->getDevice(), & desc, & depth_stencil_texture_swap_chain_ ) );
}

void OculusRift::create_layer( const ovrSizei& size_l, const ovrSizei& size_r )
{
	eye_render_desc_[ 0 ] = ovr_GetRenderDesc( ovr_session_, ovrEye_Left, hmd_desc_.DefaultEyeFov[ 0 ] );
	eye_render_desc_[ 1 ] = ovr_GetRenderDesc( ovr_session_, ovrEye_Right, hmd_desc_.DefaultEyeFov[ 1 ] );

	hmd_to_eye_pose_[ 0 ] = eye_render_desc_[ 0 ].HmdToEyePose;
	hmd_to_eye_pose_[ 1 ] = eye_render_desc_[ 1 ].HmdToEyePose;

	layer_.Header.Type = ovrLayerType_EyeFov;
	layer_.Header.Flags = 0;
	layer_.ColorTexture[ 0 ] = render_target_texture_swap_chain_;
	layer_.ColorTexture[ 1 ] = render_target_texture_swap_chain_;
	layer_.DepthTexture[ 0 ] = depth_stencil_texture_swap_chain_;
	layer_.DepthTexture[ 1 ] = depth_stencil_texture_swap_chain_;
	layer_.Fov[ 0 ] = eye_render_desc_[ 0 ].Fov;
	layer_.Fov[ 1 ] = eye_render_desc_[ 1 ].Fov;
	layer_.Viewport[ 0 ] = OVR::Recti( 0, 0, size_l.w, size_l.h );
	layer_.Viewport[ 1 ] = OVR::Recti( size_l.w, 0, size_r.w, size_r.h );
}

void OculusRift::create_render_target_view()
{
	int count = 0;
	ovr_GetTextureSwapChainLength( ovr_session_, render_target_texture_swap_chain_, &count );

	render_target_view_list_.resize( count );

	for ( int n = 0; n < count; n++ )
	{
		ID3D11Texture2D* texture = nullptr;
		ovr_GetTextureSwapChainBufferDX( ovr_session_, render_target_texture_swap_chain_, n, IID_PPV_ARGS( & texture ) );
		render_target_view_list_[ n ] = direct_3d_->create_render_target_view( texture );
		texture->Release();
	}
}

void OculusRift::create_depth_stencil_view()
{
	int count = 0;
	ovr_GetTextureSwapChainLength( ovr_session_, depth_stencil_texture_swap_chain_, &count );

	depth_stencil_view_list_.resize( count );

	for ( int n = 0; n < count; n++ )
	{
		ID3D11Texture2D* texture = nullptr;
		ovr_GetTextureSwapChainBufferDX( ovr_session_, depth_stencil_texture_swap_chain_, n, IID_PPV_ARGS( & texture ) );
		depth_stencil_view_list_[ n ] = direct_3d_->create_depth_stencil_view( texture );
		texture->Release();
	}
}

void OculusRift::update()
{
	{
		last_yaw_   = yaw_;
		last_pitch_ = pitch_;
		last_roll_  = roll_;
	}

	ovrTrackingState tracking_state = ovr_GetTrackingState( ovr_session_, ovr_GetTimeInSeconds(), ovrFalse );
	// ovr_CalcEyePoses();

	OVR::Quat< float > q( tracking_state.HeadPose.ThePose.Orientation );
	OVR::Vector3< float > v = q.ToRotationVector();

	yaw_   = -v.y;
	pitch_ = -v.x;
	roll_  =  v.z;
}

/**
 * フレーム毎に行う描画の準備
 *
 */
void OculusRift::setup_rendering()
{
	int render_target_view_index = 0;
	int depth_stencil_view_index = 0;

	OCULUS_VR_FAIL_CHECK( ovr_GetTextureSwapChainCurrentIndex( ovr_session_, render_target_texture_swap_chain_, & render_target_view_index ) );
	OCULUS_VR_FAIL_CHECK( ovr_GetTextureSwapChainCurrentIndex( ovr_session_, depth_stencil_texture_swap_chain_, & depth_stencil_view_index ) );

	ovr_WaitToBeginFrame( ovr_session_, 0 );

	auto render_target_view = render_target_view_list_[ render_target_view_index ];
	auto depth_stencil_view = depth_stencil_view_list_[ depth_stencil_view_index ];

	direct_3d_->set_render_target_for_vr( render_target_view, depth_stencil_view );
	direct_3d_->clear_render_target_view( render_target_view );
	direct_3d_->clear_depth_stencil_view( depth_stencil_view );

	double predicated_display_time = ovr_GetPredictedDisplayTime( ovr_session_, 0 );
	ovrTrackingState traking_state = ovr_GetTrackingState( ovr_session_, predicated_display_time, ovrTrue );
	ovr_CalcEyePoses( traking_state.HeadPose.ThePose, hmd_to_eye_pose_, layer_.RenderPose );

	OCULUS_VR_FAIL_CHECK( ovr_BeginFrame( ovr_session_, 0 ) );
}

/**
 * フレーム毎に行う左目用描画の準備
 *
 */
void OculusRift::setup_rendering_for_left_eye()
{
	setup_viewport( ovrEye_Left );
}

/**
 * フレーム毎に行う右目用描画の準備
 *
 */
void OculusRift::setup_rendering_for_right_eye()
{
	setup_viewport( ovrEye_Right );
}

/**
 * ビューポートの設定を行う
 *
 * @param eye_index ( ovrEye_Left or ovrEye_Right )
 */
void OculusRift::setup_viewport( int eye_index )
{
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = static_cast< float  >( layer_.Viewport[ eye_index ].Pos.x );
	viewport.TopLeftY = static_cast< float  >( layer_.Viewport[ eye_index ].Pos.y );
	viewport.Width    = static_cast< float  >( layer_.Viewport[ eye_index ].Size.w );
	viewport.Height   = static_cast< float  >( layer_.Viewport[ eye_index ].Size.h );
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	direct_3d_->getImmediateContext()->RSSetViewports( 1, & viewport );

	/*
	std::cout << "eye : " << eye_index << " / pos : "
		<< layer_.RenderPose[ eye_index ].Position.x << ", "
		<< layer_.RenderPose[ eye_index ].Position.y << ", "
		<< layer_.RenderPose[ eye_index ].Position.z << std::endl;
	*/
}

/**
 * フレーム事に行う描画終了処理
 *
 */
void OculusRift::finish_rendering()
{
	OCULUS_VR_FAIL_CHECK( ovr_CommitTextureSwapChain( ovr_session_, render_target_texture_swap_chain_ ) );
	OCULUS_VR_FAIL_CHECK( ovr_CommitTextureSwapChain( ovr_session_, depth_stencil_texture_swap_chain_ ) );

	ovrLayerHeader* layers = & layer_.Header;
	OCULUS_VR_FAIL_CHECK( ovr_EndFrame( ovr_session_, 0, nullptr, & layers, 1 ) );
}

/**
 *
 *
 * @param eye_index ( ovrEye_Left or ovrEye_Right )
 */
Direct3D11::Vector OculusRift::get_eye_position( int eye_index ) const
{
	return Direct3D11::Vector(
		layer_.RenderPose[ eye_index ].Position.x,
		layer_.RenderPose[ eye_index ].Position.y,
		layer_.RenderPose[ eye_index ].Position.z ) * COORDINATE_SYSTEM_CONVERT_MATRIX;
}

/**
 *
 * 
 * @param eye_index ( ovrEye_Left or ovrEye_Right )
 */
Direct3D11::Matrix OculusRift::get_eye_rotation( int eye_index ) const
{
	Direct3D11::Vector q(
		-layer_.RenderPose[ eye_index ].Orientation.x,
		-layer_.RenderPose[ eye_index ].Orientation.y,
		layer_.RenderPose[ eye_index ].Orientation.z,
		layer_.RenderPose[ eye_index ].Orientation.w
	);

	Direct3D11::Matrix m;
	m.set_rotation_quaternion( q );

	return m;
}

/**
 * 
 * 
 * @param eye_index ( ovrEye_Left or ovrEye_Right )
 */
Direct3D11::Matrix OculusRift::get_projection_matrix( int eye_index, float near_plane, float far_plane ) const
{
	OVR::Matrix4f projection = ovrMatrix4f_Projection( layer_.Fov[ eye_index ], near_plane, far_plane, ovrProjection_None );
	
	Direct3D11::Matrix m;

	m.set(
		projection.M[ 0 ][ 0 ], projection.M[ 1 ][ 0 ], projection.M[ 2 ][ 0 ], projection.M[ 3 ][ 0 ],
		projection.M[ 0 ][ 1 ], projection.M[ 1 ][ 1 ], projection.M[ 2 ][ 1 ], projection.M[ 3 ][ 1 ],
		projection.M[ 0 ][ 2 ], projection.M[ 1 ][ 2 ], projection.M[ 2 ][ 2 ], projection.M[ 3 ][ 2 ],
		projection.M[ 0 ][ 3 ], projection.M[ 1 ][ 3 ], projection.M[ 2 ][ 3 ], projection.M[ 3 ][ 3 ]
	);

	return COORDINATE_SYSTEM_CONVERT_MATRIX * m;
}

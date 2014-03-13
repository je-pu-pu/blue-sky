#include "OculusRift.h"

#include "memory.h"

#ifdef _DEBUG
#pragma comment( lib, "libovrd.lib" )
#else
#pragma comment( lib, "libovr.lib" )
#endif

OculusRift::OculusRift()
	: yaw_( 0.f )
	, pitch_( 0.f )
	, roll_( 0.f )
	, last_yaw_( 0.f )
	, last_pitch_( 0.f )
	, last_roll_( 0.f )
{
	OVR::System::Init( OVR::Log::ConfigureDefaultLog( OVR::LogMask_All ) );

	device_manager_ = *OVR::DeviceManager::Create();
	hmd_device_ = *device_manager_->EnumerateDevices< OVR::HMDDevice >().CreateDevice();

	if ( ! hmd_device_ )
	{
		device_manager_.Clear();
		OVR::System::Destroy();

		throw "oculus device not found.";
	}

	hmd_device_->GetDeviceInfo( & hmd_info_ );
	
	sensor_device_ = *hmd_device_->GetSensor();
	sensor_fusion_.AttachToSensor( sensor_device_ );

	{
		float_t view_center = hmd_info_.HScreenSize * 0.25f;
		float_t eyeProjectionShift = view_center - hmd_info_.LensSeparationDistance * 0.5f;
		projection_offset_ = 4.f * eyeProjectionShift / hmd_info_.HScreenSize;
	}

	{
		const float_t Area2DFov = 85.f;

		float metersToPixels          = ( hmd_info_.HResolution / hmd_info_.HScreenSize );
		float lensDistanceScreenPixels= metersToPixels * hmd_info_.LensSeparationDistance;
		float eyeDistanceScreenPixels = metersToPixels * hmd_info_.InterpupillaryDistance;
		float offCenterShiftPixels    = ( hmd_info_.EyeToScreenDistance / 0.8f) * eyeDistanceScreenPixels;
		float leftPixelCenter         = ( hmd_info_.HResolution / 2) - lensDistanceScreenPixels * 0.5f;
		float rightPixelCenter        = lensDistanceScreenPixels * 0.5f;
		float pixelDifference         = leftPixelCenter - rightPixelCenter;
    
		// This computes the number of pixels that fit within specified 2D FOV (assuming
		// distortion scaling will be done).
		float percievedHalfScreenDistance = tan(Area2DFov * 0.5f) * hmd_info_.EyeToScreenDistance;
		float vfovSize = 2.0f * percievedHalfScreenDistance / 1.f; // Distortion.Scale;
		float_t FovPixels = hmd_info_.VResolution * vfovSize / hmd_info_.VScreenSize;
    
		// Create orthographic matrix.   
		/*
		OVR::Matrix4f m;
		m.SetIdentity();
		m.M[0][0] = FovPixels / (FullView.w * 0.5f);
		m.M[1][1] = -FovPixels / FullView.h;
		m.M[0][3] = 0;
		m.M[1][3] = 0;
		m.M[2][2] = 0;
		*/

		float orthoPixelOffset = (pixelDifference + offCenterShiftPixels/*/Distortion.Scale*/) * 0.5f;
		ortho_offset_ = orthoPixelOffset * 2.0f / FovPixels;

		ortho_offset_ = -ortho_offset_ * 0.5f;

		// ortho_offset_ = 0.f;

		// ortho_offset_ = 0.30333337;
	}
}

OculusRift::~OculusRift()
{
	sensor_device_.Clear();
	hmd_device_.Clear();
	device_manager_.Clear();

	OVR::System::Destroy();
}

void OculusRift::update()
{
	last_yaw_ = yaw_;
	last_pitch_ = pitch_;
	last_roll_ = roll_;

	OVR::Quatf q = sensor_fusion_.GetOrientation();

	q.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z >( & yaw_, & pitch_, & roll_ );
}

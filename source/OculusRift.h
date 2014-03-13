#ifndef OCULUS_RIFT_H
#define OCULUS_RIFT_H

#include "type.h"
#include <OVR.h>

/**
 * Oculus Rift
 *
 */
class OculusRift
{
public:

private:
	OVR::Ptr< OVR::DeviceManager >	device_manager_;
	OVR::Ptr< OVR::SensorDevice >	sensor_device_;
    OVR::Ptr< OVR::HMDDevice >		hmd_device_;

    OVR::SensorFusion				sensor_fusion_;
    OVR::HMDInfo					hmd_info_;

	float_t							yaw_;
	float_t							pitch_;
	float_t							roll_;
	
	float_t							last_yaw_;
	float_t							last_pitch_;
	float_t							last_roll_;

	float_t							projection_offset_;
	float_t							ortho_offset_;

public:
	OculusRift();
	~OculusRift();

	void update();

	float_t get_yaw() const { return yaw_; }
	float_t get_pitch() const { return pitch_; }
	float_t get_roll() const { return roll_; }

	float_t get_delta_yaw() const { return last_yaw_ - yaw_; }
	float_t get_delta_pitch() const { return last_pitch_ - pitch_; }
	float_t get_delta_roll() const { return last_roll_ - roll_; }

	float_t get_projection_offset() const { return projection_offset_; }
	float_t get_ortho_offset() const { return ortho_offset_; }


}; // class OculusRift

#endif // OCULUS_RIFT_H

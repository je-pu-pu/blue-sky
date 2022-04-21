#pragma once

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/type.h>

#include <OVR_CAPI_D3D.h>

#include <d3d11.h>

#include <vector>

/**
 * Oculus Rift
 *
 */
class OculusRift
{
public:
	using Direct3D = core::graphics::direct_3d_11::Direct3D11;
	using Vector = core::Vector;
	using Matrix = core::Matrix;

private:
	Direct3D*						direct_3d_;

	ovrSession						ovr_session_ = nullptr;
	ovrGraphicsLuid					ovr_luid_;
	ovrHmdDesc						hmd_desc_;
	ovrTextureSwapChain				render_target_texture_swap_chain_ = nullptr;
	ovrTextureSwapChain				depth_stencil_texture_swap_chain_ = nullptr;
	
	ovrEyeRenderDesc				eye_render_desc_[ 2 ] = { ovrEyeRenderDesc(), ovrEyeRenderDesc() };
	ovrPosef						hmd_to_eye_pose_[ 2 ] = { ovrPosef(), ovrPosef() };
	ovrLayerEyeFovDepth				layer_ = ovrLayerEyeFovDepth();

	std::vector< ID3D11RenderTargetView* > render_target_view_list_;
	std::vector< ID3D11DepthStencilView* > depth_stencil_view_list_;

	/// 各軸周りの回転 ( 左手座標系 / radian )
	float_t							yaw_;
	float_t							pitch_;
	float_t							roll_;
	
	// 前回のフレームの各軸周りの回転 ( 左手座標系 / radian )
	float_t							last_yaw_;
	float_t							last_pitch_;
	float_t							last_roll_;

	static const Direct3D::Matrix COORDINATE_SYSTEM_CONVERT_MATRIX;

protected:
	void create_render_target_texture_swap_chain( const ovrSizei& );
	void create_depth_stencil_texture_swap_chain( const ovrSizei& );

	void create_render_target_view();
	void create_depth_stencil_view();

	void create_layer( const ovrSizei&, const ovrSizei& );

	void setup_default_viewport( int );

public:
	OculusRift( Direct3D* );
	~OculusRift();

	void update();

	void setup_rendering();
	void setup_rendering_for_left_eye();
	void setup_rendering_for_right_eye();

	void finish_rendering();

	Vector get_eye_position( int ) const;
	Matrix get_eye_rotation( int ) const;
	Matrix get_projection_matrix( int, float, float ) const;

	float_t get_yaw() const { return yaw_; }
	float_t get_pitch() const { return pitch_; }
	float_t get_roll() const { return roll_; }

	float_t get_delta_yaw() const { return yaw_ - last_yaw_; }
	float_t get_delta_pitch() const { return pitch_ - last_pitch_; }
	float_t get_delta_roll() const { return roll_ - last_roll_; }

}; // class OculusRift

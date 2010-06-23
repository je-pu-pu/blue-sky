#include "Camera.h"
#include "matrix4x4.h"
#include <common/math.h>
#include <windows.h>

namespace blue_sky
{

Camera::Camera()
	: fov_( 90.f )
	, under_view_rate_( 0.f )
{
	
}

Camera::~Camera()
{
	
}

void Camera::set_under_view_rate( float rate )
{
	under_view_rate_ = math::clamp( rate, 0.f, 1.f );
	
	matrix4x4 m;
	m.rotate_x( under_view_rate_ * 90.f );
	// m.rotate_y( timeGetTime() * 0.01f );
	
	// look at
	// look_at_ = position() + vector3( 0.f, 0.f, 1.f ) * m;

	// up
	up_ = vector3( 0.f, 1.f, 0.f ) * m;

	// fov() = 60.f + under_view_rate_ * 30.f;
}

}; // namespace blue_sky
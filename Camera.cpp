#include "Camera.h"
#include "matrix4x4.h"
#include <common/math.h>
#include <windows.h>

namespace blue_sky
{

Camera::Camera()
	: fov_( 60.f )
	, front_( 0.f, 0.f, 1.f )
	, front_target_( 0.f, 0.f, 1.f )
	, under_view_rate_( 0.f )
{
	
}

Camera::~Camera()
{
	
}

void Camera::update()
{
	float speed = 0.1f;
	float speed_y = 1.f;

	front_.x() = math::chase( front_.x(), front_target_.x(), speed );
	front_.y() = math::chase( front_.y(), front_target_.y(), speed_y );
	front_.z() = math::chase( front_.z(), front_target_.z(), speed );
}

void Camera::set_under_view_rate( float rate )
{
	under_view_rate_ = math::clamp( rate, 0.f, 1.f );
	
	matrix4x4 m;
	m.rotate_x( under_view_rate_ * 90.f );
	// m.rotate_y( timeGetTime() * 0.01f );
	
	// look at
	vector3 at = front_;
	at *= ( 1.f - under_view_rate_ );
	at.y() = -under_view_rate_;

	look_at_ = position_ + at;

	// up
	up_ = vector3( 0.f, 1.f, 0.f ) * m;

	// fov() = 60.f + under_view_rate_ * 30.f;
}

}; // namespace blue_sky
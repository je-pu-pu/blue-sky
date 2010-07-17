#include "Camera.h"
#include "matrix4x4.h"
#include <common/math.h>
#include <windows.h>

namespace blue_sky
{

Camera::Camera()
	: default_front_( 0.f, 0.f, 1.f )
	, default_up_( 0.f, 1.f, 0.f )
	, fov_( 60.f )
{

}

Camera::~Camera()
{
	
}

void Camera::update()
{
//	rotate_degree().x() = math::chase( rotate_degree().x(), rotate_degree_target().x(), 2.f );
//	rotate_degree().y() = math::chase( rotate_degree().y(), rotate_degree_target().y(), 4.f );

	while ( rotate_degree_target().y() - rotate_degree().y() > +180.f ) rotate_degree().y() += 360.f;
	while ( rotate_degree_target().y() - rotate_degree().y() < -180.f ) rotate_degree().y() -= 360.f;
	
	rotate_degree().x() += ( rotate_degree_target().x() - rotate_degree().x() ) * 0.1f;
	rotate_degree().y() += ( rotate_degree_target().y() - rotate_degree().y() ) * 0.1f;
	rotate_degree().z() = math::chase( rotate_degree().z(), rotate_degree_target().z(), 0.8f );

	matrix4x4 m;
	m.rotate_y( rotate_degree().y() );

	front_ = default_front_ * m;

	// 
	matrix4x4 xr;
	xr.rotate_x( rotate_degree().x() );

	matrix4x4 yr;
	yr.rotate_y( rotate_degree().y() );

	matrix4x4 zr;
	zr.rotate_z( rotate_degree().z() );

	look_at_ = position() + default_front_ * xr * zr * yr;
	up_ = default_up_ * xr * zr * yr;
}

void Camera::set_fov( float fov )
{
	fov_ = fov;
}

}; // namespace blue_sky
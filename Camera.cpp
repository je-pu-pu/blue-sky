#include "Camera.h"
#include "matrix4x4.h"
#include <common/math.h>
#include <windows.h>

namespace blue_sky
{

Camera::Camera()
	: default_front_( 0.f, -1.f, 0.f )
	, default_up_( 0.f, 0.f, 1.f )
	, direction_degree_( 0.f )
	, direction_degree_target_( 0.f )
	, fov_( 120.f )
	, under_view_rate_( 0.f )
	, panorama_y_division_( 1 )
{
	set_fov( fov_ );
}

Camera::~Camera()
{
	
}

void Camera::update()
{
	float speed = 4.f;

	direction_degree_ = math::chase( direction_degree_, direction_degree_target_, speed );
}

void Camera::set_fov( float fov )
{
	fov_ = fov;

	matrix4x4 m;
	m.rotate_x( -fov_ / 2 );

	default_front_ *= m;
	default_up_ *= m;
}

void Camera::set_under_view_rate( float rate )
{
	under_view_rate_ = math::clamp( rate, 0.f, 1.f );
	// under_view_rate_ = 0.f;
	
	/*
	matrix4x4 xr;
	xr.rotate_x( under_view_rate_ * ( fov_ / 2.f ) );
	// m.rotate_y( timeGetTime() * 0.01f );

	matrix4x4 xru;
	xru.rotate_x( under_view_rate_ * ( 90.f ) + 60.f );
	
	matrix4x4 yr;
	yr.rotate_y( get_direction_degree() );

	// look at
	look_at_ = position() + ( vector3( 0.f, 0.f, 1.f ) * xr ) * yr;
	look_at_under_ = position() + ( vector3( 0.f, 0.f, 1.f ) * xru ) * yr;

	// up
	up_ = vector3( 0.f, 1.f, 0.f ) * xr * yr;
	up_under_ = vector3( 0.f, 1.f, 0.f ) * xru * yr;

	// set_fov( 120.f; // + under_view_rate_ * 30.f;
	*/
}

vector3 Camera::get_look_at_part( int n ) const
{
	float x_rot_degree = under_view_rate_ * ( fov_ / 2.f ) + ( n - panorama_y_division_ / 2 ) * ( fov_ / panorama_y_division_ ) + ( fov_ / panorama_y_division_ / 2.f );

	matrix4x4 xr;
	xr.rotate_x( x_rot_degree );

	matrix4x4 yr;
	yr.rotate_y( get_direction_degree() );

	return position_ + ( default_front_ * xr ) * yr;
}

vector3 Camera::get_up_part( int n ) const
{
	float x_rot_degree = under_view_rate_ * ( fov_ / 2.f ) + ( n - panorama_y_division_ / 2 ) * ( fov_ / panorama_y_division_ ) + ( fov_ / panorama_y_division_ / 2.f );

	matrix4x4 xr;
	xr.rotate_x( x_rot_degree );

	matrix4x4 yr;
	yr.rotate_y( get_direction_degree() );

	return default_up_ * xr * yr;
}

}; // namespace blue_sky
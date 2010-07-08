#include "Camera.h"
#include "matrix4x4.h"
#include <common/math.h>
#include <windows.h>

namespace blue_sky
{

Camera::Camera()
	: fov_( 120.f )
	, under_view_rate_( 0.f )
	, rotate_step_x_( 0 )
	, rotate_step_y_( 0 )
	, panorama_y_division_( 1 )
{
	set_fov( fov_ );
}

Camera::~Camera()
{
	
}

void Camera::update()
{
	rotate_degree().x() = math::chase( rotate_degree().x(), rotate_degree_target().x(), 2.f );
	rotate_degree().y() = math::chase( rotate_degree().y(), rotate_degree_target().y(), 4.f );
	rotate_degree().z() = math::chase( rotate_degree().z(), rotate_degree_target().z(), 4.f );

	/*
	for ( int n = 0; n < 3; n++ )
	{
		if ( rotate_degree().get( n ) == rotate_degree_target().get( n ) )
		{
			while ( rotate_degree().get( n ) <   0.f ) rotate_degree().get( n ) += 360.f;
//			while ( rotate_degree().get( n ) > 360.f ) rotate_degree().get( n ) -= 360.f;

			rotate_degree_target().get( n ) = rotate_degree().get( n );
		}
		else
		{
			// while ( rotate_degree().get( n ) - rotate_degree_target().get( n ) < -360.f ) rotate_degree_target().get( n ) += 360.f;
			// while ( rotate_degree().get( n ) - rotate_degree_target().get( n ) >  360.f ) rotate_degree_target().get( n ) -= 360.f;
		}
	}
	*/

	matrix4x4 m;
	m.rotate_y( rotate_degree().y() );

	front_ = default_front_ * m;
}

void Camera::set_fov( float fov )
{
	fov_ = fov;

	matrix4x4 m;
	// m.rotate_x( -fov_ / 2 );
	m.rotate_x( -90.f );

	default_front_ = vector3( 0.f, -1.f, 0.f ) * m;
	default_up_ = vector3( 0.f, 0.f, 1.f ) * m;
}

void Camera::set_under_view_rate( float rate )
{
	under_view_rate_ = math::clamp( rate, 0.f, 1.f );
}

vector3 Camera::get_look_at_part( int n ) const
{
	float x_rot_degree = -( ( fov_ * 0.5f ) - ( fov_ / panorama_y_division_ * 0.5f ) ) + ( fov_ / panorama_y_division_ ) * n;
	x_rot_degree += rotate_degree().x();

	matrix4x4 xr;
	xr.rotate_x( x_rot_degree );

	matrix4x4 yr;
	yr.rotate_y( rotate_degree().y() );

	matrix4x4 zr;
	zr.rotate_z( rotate_degree().z() );

	return position_ + ( default_front_ * xr ) * zr * yr;
}

vector3 Camera::get_up_part( int n ) const
{
	float x_rot_degree = -( ( fov_ * 0.5f ) - ( fov_ / panorama_y_division_ * 0.5f ) ) + ( fov_ / panorama_y_division_ ) * n;
	x_rot_degree += rotate_degree().x();

	matrix4x4 xr;
	xr.rotate_x( x_rot_degree );

	matrix4x4 yr;
	yr.rotate_y( rotate_degree().y() );

	matrix4x4 zr;
	zr.rotate_z( rotate_degree().z() );

	return default_up_ * xr * zr * yr;
}

void Camera::step_rotate_x( int step )
{
	const int step_degree_ = 45;
	const int max_step = 2;

	rotate_step_x_ += step;
	rotate_step_x_ = math::clamp( rotate_step_x_, -max_step, max_step );

	rotate_degree_target().x() = static_cast< float >( rotate_step_x_ * step_degree_ );
}

void Camera::step_rotate_y( int step )
{
	const int step_degree_ = 30;
	const int max_step = 3;

	rotate_step_y_ += step;
	rotate_step_y_ = math::clamp( rotate_step_y_, -max_step, max_step );

	rotate_degree_target().y() = static_cast< float >( rotate_step_y_ * step_degree_ );
}

}; // namespace blue_sky
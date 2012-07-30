#include "Camera.h"
#include "Player.h"
#include "matrix4x4.h"

#include <common/math.h>

namespace blue_sky
{

Camera::Camera()
	: default_front_( 0.f, 0.f, 1.f )
	, default_right_( 1.f, 0.f, 0.f )
	, default_up_( 0.f, 1.f, 0.f )
	, fov_( 60.f )
	, fov_target_( fov_ )
	, fov_default_( fov_ )
	, aspect_( 720.f / 480.f )
	, rotate_chase_speed_( 0.f )
{
	reset_rotate_chase_speed();
}

Camera::~Camera()
{
	
}

void Camera::update()
{
	while ( rotate_degree_target().y() - rotate_degree().y() > +180.f ) rotate_degree().y() += 360.f;
	while ( rotate_degree_target().y() - rotate_degree().y() < -180.f ) rotate_degree().y() -= 360.f;
	
	/*
	rotate_degree().x() += ( rotate_degree_target().x() - rotate_degree().x() ) * get_rotate_chase_speed();
	rotate_degree().y() += ( rotate_degree_target().y() - rotate_degree().y() ) * get_rotate_chase_speed();
	rotate_degree().z() = math::chase( rotate_degree().z(), rotate_degree_target().z(), 0.8f );
	*/

	rotate_degree() = rotate_degree_target();

	// 
	matrix4x4 xr;
	xr.rotate_x( rotate_degree().x() );

	matrix4x4 yr;
	yr.rotate_y( rotate_degree().y() );

	matrix4x4 zr;
	zr.rotate_z( rotate_degree().z() );
	
	front_ = default_front_ * xr * yr;
	right_ = default_right_ * xr * yr;

	look_at_ = position() + default_front_ * xr * zr * yr;
	up_ = default_up_ * xr * zr * yr;

	fov_ = fov_ * 0.9f + fov_target_ * 0.1f;
	fov_ = math::clamp( fov_, 2.f, 100.f );
}

void Camera::update_with_player( const Player* player )
{
	position().x() = player->get_transform().getOrigin().x();
	position().y() = player->get_transform().getOrigin().y() + player->get_eye_height();
	position().z() = player->get_transform().getOrigin().z();
}

void Camera::set_fov( float fov )
{
	fov_ = fov;
}

void Camera::set_fov_target( float fov_target )
{
	fov_target_ = math::clamp( fov_target, 0.f, 100.f );
}

}; // namespace blue_sky
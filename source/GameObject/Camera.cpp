#include "Camera.h"

#include <GameObject/Player.h>

#include <common/math.h>

namespace blue_sky
{

Camera::Camera()
	: position_()
	, look_at_( 0.f, 0.f, 1.f, 0.f )
	, up_( 0.f, 1.f, 0.f, 0.f )
	, rotate_degree_( 0.f, 0.f, 0.f, 0.f )
	, rotate_degree_target_( 0.f, 0.f, 0.f, 0.f )
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

void Camera::restart()
{
	reset_fov();

	rotate_degree_target_ = Vector( 0, 0, 0, 0 );
	rotate_degree_ = Vector( 0, 0, 0, 0 );
}

void Camera::update()
{
	while ( rotate_degree_target().y() - rotate_degree().y() > +180.f ) rotate_degree().set_y( rotate_degree().y() + 360.f );
	while ( rotate_degree_target().y() - rotate_degree().y() < -180.f ) rotate_degree().set_y( rotate_degree().y() - 360.f );

	rotate_degree() = rotate_degree_target();
	
	Matrix m;
	m.set_rotation_xyz(
		math::degree_to_radian( rotate_degree().x() ),
		math::degree_to_radian( rotate_degree().y() ),
		math::degree_to_radian( rotate_degree().z() )
	);

	front_ = Vector::Forward * m;
	right_ = Vector::Right * m;

	look_at_ = position() + front_;
	up_ = Vector::Up * m;

	fov_ = fov_ * 0.9f + fov_target_ * 0.1f;
	fov_ = math::clamp( fov_, 2.f, 100.f );
}

void Camera::update_with_player( const Player* player )
{
	if ( false )
	{
		// debug
		position_.set_x( player->get_transform().get_position().x() + player->get_front().x() * player->get_eye_depth() ); // * player->get_collision_depth() * 0.5f;
		position_.set_y( player->get_transform().get_position().y() + player->get_eye_height() + 5.f );
		position_.set_z( player->get_transform().get_position().z() + player->get_front().z() * player->get_eye_depth() ); // * player->get_collision_depth() * 0.5f;

		position_ += Vector( player->get_front().x(), 0.f, player->get_front().x(), 0.f ) * -2.f;
	}
	else
	{
		position_.set_x( player->get_transform().get_position().x() + player->get_front().x() * player->get_eye_depth() ); // * player->get_collision_depth() * 0.5f;
		position_.set_y( player->get_transform().get_position().y() + player->get_eye_height() );
		position_.set_z( player->get_transform().get_position().z() + player->get_front().z() * player->get_eye_depth() ); // * player->get_collision_depth() * 0.5f;
	}

	if ( player->is_dead() )
	{
		rotate_degree_target().set_z( ( 1.5f - player->get_eye_height() ) / 1.5f * -90.f );
	}
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
#include "Player.h"
#include "Stage.h"
#include "GridCell.h"

#include "GameMain.h"

#include "Sound.h"
#include "SoundManager.h"
#include "Input.h"

#include "Direct3D9.h"

#include "matrix4x4.h"

#include <game/AABB.h>

#include <common/exception.h>
#include <common/math.h>

#include <list>

namespace blue_sky
{

Player::Player()
	 : input_( 0 )
	 , step_speed_( 0.05f )
	 , gravity_( 0.01f )
	 , eye_height_( 1.5f )
	 , is_turn_avaiable_( true )
	 , is_jumping_( false )
	 , is_clambering_( false )
	 , is_falling_( false )
	 , up_count_( 0 )
{

}

void Player::step( float s )
{
	vector3 v = front() * s * get_step_speed();
	velocity().x() = v.x();
	velocity().z() = v.z();
}

void Player::side_step( float s )
{
	//
}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	set_direction_degree( get_direction_degree() + d );
}

/**
 * 更新
 *
 */
void Player::update()
{
	int w = const_cast< Input* >( input_ )->pop_mouse_wheel_queue();

	if ( w > 0 )
	{
		step_speed_ += 0.05f;
		step_speed_ = math::clamp( step_speed_, -0.05f, 0.1f );
	}
	if ( w < 0 )
	{
		step_speed_ -= 0.05f;
		step_speed_ = math::clamp( step_speed_, -0.05f, 0.1f );
	}

	step_speed_ = math::clamp( step_speed_, -0.02f, 0.5f );

	limit_velocity();
	update_position();

	up_count_ = math::chase( up_count_, 0, 1 );

	if ( up_count_ > 0 )
	{
		velocity().y() += get_gravity() * 3.f;
	}

	// gravity
	velocity().y() -= get_gravity();
	// velocity().y() -= 0.015f;
	// velocity().y() -= 0.004f;
	// velocity().y() -= 0.002f;
	// velocity().y() -= 0.001f;
	// velocity().y() -= 0.0001f;

	// 減速
	{
		velocity().x() *= 0.9f;
		velocity().z() *= 0.9f;

		if ( velocity().y() < -get_max_speed() * 0.02f )
		{
			is_jumping_ = true;
		}
	}

	if ( is_dead() )
	{
		eye_height_ -= 0.02f;
		eye_height_ = std::max( 0.2f, eye_height_ );
	}
}

void Player::on_collision_x( const GridCell& floor_cell_x )
{
	if (
		( velocity().y() <= 0.02f && floor_cell_x.height() - position().y() <= 2.f ) &&
		(
			( velocity().x() < 0.f && direction() == LEFT || velocity().x() > 0.f && direction() == RIGHT ) ||
			( floor_cell_x.height() - position().y() <= 1.f && ( velocity().x() < 0.f && direction() != RIGHT || velocity().x() > 0.f && direction() != LEFT ) )
		) )
	{
		velocity().y() = 0.02f;

		is_jumping_ = false;

		if ( ! is_clambering() && floor_cell_x.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall" );
	}

	velocity().x() *= 0.1f;
}

void Player::on_collision_y( const GridCell& floor_cell_y )
{
	// if ( is_jumping() && is_falling() && floor_cell_y.bound() > 0 )
	if ( is_jumping() && floor_cell_y.bound() > 0 )
	{
		// スーパージャンプ
		velocity().y() = 1.f;
	
		is_jumping_ = true;
		// step_speed_ = 0.2f;

		stop_sound( "super-jump" );
		play_sound( "super-jump" );

		float speed = 0.f;

		if ( is_falling_ )
		{
			speed = velocity_on_fall_.length_xz();
		}
		else
		{
			speed = velocity().length_xz();
		}

		velocity().x() = 0.f;
		velocity().z() = 0.f;
		velocity() += front() * speed;
	}
	else
	{
		// 着地失敗
		if ( is_falling_to_dead() )
		{
			kill();
			play_sound( "land" );
		}

		// 通常着地
		velocity().y() *= -0.01f;

		if ( is_jumping_ )
		{
			stop_sound( "jump" );
			play_sound( "land" );

			is_jumping_ = false;
			// step_speed_ = math::clamp( step_speed_, -0.05f, 0.1f );

			// velocity().x() = 0.f;
			// velocity().z() = 0.f;
		}
	}

	is_turn_avaiable_ = true;
	is_falling_ = false;
	is_clambering_ = false;
}

void Player::on_collision_z( const GridCell& floor_cell_z )
{
	if (
		( velocity().y() <= 0.02f && floor_cell_z.height() - position().y() <= 2.f ) && 
		(
			( velocity().z() < 0.f && direction() == BACK || velocity().z() > 0.f && direction() == FRONT ) ||
			( floor_cell_z.height() - position().y() <= 1.f && ( velocity().z() < 0.f && direction() != FRONT || velocity().z() > 0.f && direction() != BACK ) )
		) )
	{
		velocity().y() = 0.02f;

		is_jumping_ = false;

		if ( ! is_clambering() && floor_cell_z.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall" );
	}

	velocity().z() *= 0.1f;
}

/**
 * ジャンプ開始
 * 
 */	
void Player::jump()
{
	if ( is_jumping() ) return;
	
	velocity().y() = 0.3f;
	
	// velocity().x() += front_.x() * 0.4f;
	// velocity().z() += front_.z() * 0.4f;
	
	is_jumping_ = true;

	play_sound( "jump" );
}

/**
 * 落下開始
 *
 */
void Player::fall()
{
	if ( is_falling_ ) return;

	is_falling_ = true;

	velocity_on_fall_ = velocity();
	velocity() = vector3( 0.f, -get_max_speed(), 0.f );

	play_sound( "fall" );
}

/**
 * 
 */
bool Player::is_falling_to_dead() const
{
	return velocity().y() < -get_max_speed() * 0.6f;
}

void Player::kill()
{
	if ( ! is_dead() )
	{
		play_sound( "dead" );
	}

	ActiveObject::kill();
}

void Player::rebirth()
{
	eye_height_ = 1.5f;
	up_count_ = 0;

	velocity().init();

	restart();
}

void Player::set_input( const Input* input )
{
	input_ = input;
}

void Player::on_get_balloon()
{
	up_count_ += 120;

	play_sound( "ok" );
}

float Player::get_collision_width() const
{
	return 0.4f;
}

float Player::get_collision_height() const
{
	return 1.5f;
}

float Player::get_collision_depth() const
{
	return 0.4f;
}

} // namespace blue_sky

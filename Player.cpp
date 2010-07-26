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
	 , step_count_( 0 )
	 , step_speed_( 0.05f )
	 , gravity_( 0.01f )
	 , eye_height_( 1.5f )
	 , is_turn_avaiable_( true )
	 , is_jumping_( false )
	 , is_clambering_( false )
	 , is_falling_( false )
	 , is_umbrella_mode_( false )
	 , up_count_( 0 )
	 , rocket_count_( 0 )
{

}

void Player::step( float s )
{
	if ( is_rocketing() )
	{
		return;
	}

	step_count_++;

	update_step_speed();

	vector3 v = front() * s * get_step_speed();
	velocity().x() = v.x();
	velocity().z() = v.z();
}

void Player::side_step( float s )
{
	velocity() += right() * s * get_side_step_speed();
}

void Player::stop()
{
	if ( is_jumping() )
	{
		return;
	}

	step_count_ = 0;

	update_step_speed();
}

void Player::update_step_speed()
{
	if ( is_jumping() )
	{
		step_speed_ = math::chase( step_speed_, 0.1f, 0.001f );
	}

	if ( step_count_ <= 0 )
	{
		// stop
		step_speed_ = 0.f;
	}
	else if ( step_count_ <= 20 )
	{
		// slow walk
		step_speed_ = math::chase( step_speed_, 0.01f, 0.001f );
	}
	else if ( step_count_ <= 240 )
	{
		// slow walk
		step_speed_ = math::chase( step_speed_, 0.05f, 0.001f );
	}
	else
	{
		// run
		step_speed_ = math::chase( step_speed_, 0.1f, 0.001f );
	}

	if ( is_jumping() || is_rocketing() || is_clambering() )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
	else if ( step_count_ >= 20 && step_count_ < 240 )
	{
		play_sound( "walk", true, false );
	}
	else if ( step_count_ >= 240 )
	{
		stop_sound( "walk" );
		play_sound( "run", true, false );
	}
	else if ( step_count_ == 0 )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	if ( is_rocketing() )
	{
		return;
	}

	set_direction_degree( get_direction_degree() + d );
}

/**
 * 更新
 *
 */
void Player::update()
{
	limit_velocity();
	update_position();

	up_count_ = math::chase( up_count_, 0, 1 );
	rocket_count_ = math::chase( rocket_count_, 0, 1 );

	// gravity
	if ( up_count_ > 0 )
	{
		velocity().y() = math::chase( velocity().y(), 0.4f, 0.01f );
	}
	else if ( is_umbrella_mode_ )
	{
		velocity().y() = math::chase( velocity().y(), -0.1f, 0.02f );
	}
	else if ( ! is_rocketing() )
	{
		velocity().y() -= get_gravity();
	}

	// 減速
	if ( ! is_rocketing() )
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
		( ! is_jumping()  ) &&
		( velocity().y() <= 0.02f && floor_cell_x.height() - position().y() <= 2.f ) &&
		(
			( velocity().x() < 0.f && direction() == LEFT || velocity().x() > 0.f && direction() == RIGHT ) ||
			( floor_cell_x.height() - position().y() <= 1.f && ( velocity().x() < 0.f && direction() != RIGHT || velocity().x() > 0.f && direction() != LEFT ) )
		) )
	{
		velocity().y() = get_clambering_speed();

		is_jumping_ = false;
		step_count_ = 0;
		step_speed_ = 0.f;

		if ( ! is_clambering() && floor_cell_x.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall", false, false );
	}

	if ( is_rocketing() )
	{
		stop_rocket();
		velocity().x() *= -0.8f;
	}
	else
	{
		velocity().x() *= 0.1f;
	}
}

void Player::on_collision_y( const GridCell& floor_cell_y )
{
	// if ( is_jumping() && is_falling() && floor_cell_y.bound() > 0 )
	if ( is_jumping() && floor_cell_y.bound() > 0 )
	{
		// スーパージャンプ
		velocity().y() = 1.f;
	
		is_jumping_ = true;

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
		}

		is_umbrella_mode_ = false;
	}

	is_turn_avaiable_ = true;
	is_falling_ = false;
	is_clambering_ = false;
}

void Player::on_collision_z( const GridCell& floor_cell_z )
{
	if (
		( ! is_jumping()  ) &&
		( velocity().y() <= 0.02f && floor_cell_z.height() - position().y() <= 2.f ) && 
		(
			( velocity().z() < 0.f && direction() == BACK || velocity().z() > 0.f && direction() == FRONT ) ||
			( floor_cell_z.height() - position().y() <= 1.f && ( velocity().z() < 0.f && direction() != FRONT || velocity().z() > 0.f && direction() != BACK ) )
		) )
	{
		velocity().y() = get_clambering_speed();

		is_jumping_ = false;
		step_count_ = 0;
		step_speed_ = 0.f;

		if ( ! is_clambering() && floor_cell_z.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall", false, false );
	}

	if ( is_rocketing() )
	{
		stop_rocket();
		velocity().z() *= -0.8f;
	}
	else
	{
		velocity().z() *= 0.1f;
	}
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
	rocket_count_ = 0;

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
 * 落下傘モード開始
 *
 */
void Player::start_umbrella_mode()
{
	is_umbrella_mode_ = true;
}

/**
 * このまま落下すると死亡するかどうかを取得する
 *
 */
bool Player::is_falling_to_dead() const
{
	if ( is_umbrella_mode_ ) return false;

	if ( ! last_floor_cell() ) return false;
	if ( ! floor_cell() ) return false;
	

	return velocity().y() < -0.2f && last_floor_cell()->height() - floor_cell()->height() >= 20;
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

void Player::rocket( const vector3& direction )
{
	rocket_count_ = 120;
	velocity() = direction * get_max_speed();
}

void Player::stop_rocket()
{
	rocket_count_ = 0;
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

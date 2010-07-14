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
	 , stage_( 0 )
	 , position_( 0.f, 50.f, 0.f )
	 , direction_degree_( 0.f )
	 , step_speed_( 0.06f )
	 , eye_height_( 1.5f )
	 , aabb_( vector3( -get_collision_width() * 0.5f, 0.f, -get_collision_depth() * 0.5f ), vector3( get_collision_width() * 0.5f, get_collision_height(), get_collision_depth() * 0.5f ) )
	 , is_dead_( false )
	 , is_turn_avaiable_( true )
	 , is_jumping_( false )
	 , is_clambering_( false )
	 , is_falling_( false )
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
	direction_degree_ += d;
	
	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	matrix4x4 m;
	m.rotate_y( direction_degree_ );

	front_ = vector3( 0.f, 0.f, 1.f ) * m;

	if ( direction_degree_ < 45.f ) direction_ = FRONT;
	else if ( direction_degree_ < 45.f + 90.f * 1.f ) direction_ = RIGHT;
	else if ( direction_degree_ < 45.f + 90.f * 2.f ) direction_ = BACK;
	else if ( direction_degree_ < 45.f + 90.f * 3.f ) direction_ = LEFT;
	else direction_ = FRONT;
}

/**
 * 更新
 *
 */
void Player::update()
{
	if ( velocity().length() > get_max_speed() )
	{
		velocity().normalize() *= get_max_speed();
	}

	const vector3 last_position = position();

	position().x() += velocity().x();
	position().x() = math::clamp( position().x(), 0.f, static_cast< float >( stage_->width() ) );

	const GridCell& floor_cell_x = get_floor_cell();

	if ( position().y() < floor_cell_x.height() )
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

		position().x() = last_position.x();
		velocity().x() *= 0.1f;
	}

	position().z() += velocity().z();
	position().z() = math::clamp( position().z(), 0.f, static_cast< float >( stage_->depth() ) );

	const GridCell& floor_cell_z = get_floor_cell();

	if ( position().y() < floor_cell_z.height() )
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

		position().z() = last_position.z();
		velocity().z() *= 0.1f;
	}

	position().y() += velocity().y();

	if ( position().y() >= 300.f )
	{
		position().y() = 300.f;
		velocity().y() = 0.f;
	}

	const GridCell& floor_cell_y = get_floor_cell();
	
	// 着地処理
	if ( position().y() < floor_cell_y.height() )
	{
		position().y() = floor_cell_y.height();

		// if ( is_jumping() && is_falling() && floor_cell_y.bound() > 0 )
		if ( is_jumping() && floor_cell_y.bound() > 0 )
		{
			// スーパージャンプ
			velocity_.y() = 1.f;
	
			is_jumping_ = true;
			step_speed_ += 0.01f;

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
				is_dead_ = true;
				play_sound( "dead" );
				play_sound( "land" );
			}

			// 通常着地
			velocity().y() *= -0.01f;

			if ( is_jumping_ )
			{
				stop_sound( "jump" );
				play_sound( "land" );

				is_jumping_ = false;
				step_speed_ = 0.06f;

				// velocity().x() = 0.f;
				// velocity().z() = 0.f;
			}
		}

		is_turn_avaiable_ = true;
		is_falling_ = false;
		is_clambering_ = false;
	}

	position().y() = std::max( 0.f, position().y() );

	// gravity
	velocity().y() -= 0.01f;
	// velocity().y() -= 0.015f;
	// velocity().y() -= 0.004f;
	// velocity().y() -= 0.002f;
	// velocity().y() -= 0.001f;
	// velocity().y() -= 0.0001f;

	if ( input_->press( Input::B ) )
	{
		velocity().x() = 0.f;
		velocity().z() = 0.f;
	}
	else if ( is_jumping() )
	{
		velocity().x() = math::chase( velocity().x(), 0.f, 0.001f );
		velocity().z() = math::chase( velocity().z(), 0.f, 0.001f );
	}
	else
	{
		velocity().x() = math::chase( velocity().x(), 0.f, 0.0015f );
		velocity().z() = math::chase( velocity().z(), 0.f, 0.0015f );

		if ( velocity().y() < -get_max_speed() * 0.02f )
		{
			is_jumping_ = true;
		}
	}

	if ( velocity().y() < -10.f )
	{
		velocity().y() = -10.f;
	}

	if ( is_dead() )
	{
		eye_height_ -= 0.02f;
		eye_height_ = std::max( 0.2f, eye_height_ );
	}
}

/**
 * ジャンプ開始
 * 
 */	
void Player::jump()
{
	if ( is_jumping() ) return;
	
	velocity_.y() = 0.5f;
	
	// velocity_.x() += front_.x() * 0.4f;
	// velocity_.z() += front_.z() * 0.4f;
	
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

void Player::rebirth()
{
	is_dead_ = false;
	direction_ = FRONT;
	direction_degree_ = 0.f;
	eye_height_ = 1.5f;
}

const GridCell& Player::get_floor_cell_center() const
{
	return stage_->cell( static_cast< int >( position_.x() ), static_cast< int >( position_.z() ) );
}

const GridCell& Player::get_floor_cell_left_front() const
{
	return stage_->cell( static_cast< int >( position().x() - get_collision_width() * 0.5f ), static_cast< int >( position().z() + get_collision_depth() * 0.5f ) );
}

const GridCell& Player::get_floor_cell_right_front() const
{
	return stage_->cell( static_cast< int >( position().x() + get_collision_width() * 0.5f ), static_cast< int >( position().z() + get_collision_depth() * 0.5f ) );
}

const GridCell& Player::get_floor_cell_left_back() const
{
	return stage_->cell( static_cast< int >( position().x() - get_collision_width() * 0.5f ), static_cast< int >( position().z() - get_collision_depth() * 0.5f ) );
}

const GridCell& Player::get_floor_cell_right_back() const
{
	return stage_->cell( static_cast< int >( position().x() + get_collision_width() * 0.5f ), static_cast< int >( position().z() - get_collision_depth() * 0.5f ) );
}

void Player::set_stage( const Stage* stage )
{
	stage_ = stage;
}

void Player::set_input( const Input* input )
{
	input_ = input;
}

/**
 *
 */
float Player::get_max_speed() const
{
	return 1.f;
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

/**
 * プレイヤーの接触している一番高いグリッドセルを返す
 */
const GridCell& Player::get_floor_cell() const
{
	std::list< const GridCell* > grid_cell_list;

	grid_cell_list.push_back( & get_floor_cell_left_front() );
	grid_cell_list.push_back( & get_floor_cell_right_front() );
	grid_cell_list.push_back( & get_floor_cell_left_back() );
	grid_cell_list.push_back( & get_floor_cell_right_back() );

	grid_cell_list.sort( GridCell::height_less() );

	return * grid_cell_list.back();
}

void Player::play_sound( const char* name, bool loop ) const
{
	Sound* sound = GameMain::getInstance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->play( loop );
	}
}

void Player::stop_sound( const char* name ) const
{
	Sound* sound = GameMain::getInstance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

} // namespace blue_sky

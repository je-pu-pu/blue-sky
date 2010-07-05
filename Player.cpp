#include "Player.h"
#include "Stage.h"
#include "GridCell.h"

#include "GameMain.h"

#include "Sound.h"
#include "SoundManager.h"

#include "Direct3D9.h"

#include "matrix4x4.h"

#include <common/exception.h>

#include <list>

namespace blue_sky
{

Player::Player()
	 : stage_( 0 )
	 , direction_( FRONT )
	 , direction_degree_( 0.f )
	 , is_turn_avaiable_( true )
	 , is_jumping_( false )
	 , is_falling_( false )
{
	//
	turn( 0 );
}

void Player::step( float s )
{
	if ( is_falling() )
	{
		s = 0.f;
	}
	else if ( is_jumping() )
	{
		s *= 1.5f;
	}

	velocity() += front() * s * get_step_speed();
}

void Player::side_step( float s )
{
	if ( is_jumping() )
	{
		s *= 1.5f;
	}
	
	// velocity() += right() * s * get_side_step_speed();
}

void Player::turn( int d )
{
	direction_ = static_cast< Direction >( ( direction_ + d + DIRECTION_MAX ) % DIRECTION_MAX );

	switch ( direction_ )
	{
	case FRONT : front_ = vector3(  0.f,  0.f,  1.f ); right_ = vector3(  1.f,  0.f,  0.f ); break;
	case RIGHT : front_ = vector3(  1.f,  0.f,  0.f ); right_ = vector3(  0.f,  0.f, -1.f ); break;
	case BACK  : front_ = vector3(  0.f,  0.f, -1.f ); right_ = vector3( -1.f,  0.f,  0.f ); break;
	case LEFT  : front_ = vector3( -1.f,  0.f,  0.f ); right_ = vector3(  0.f,  0.f,  1.f ); break;
	default : COMMON_THROW_EXCEPTION;
	}

	direction_degree_ += d * 90.f;

	if ( d )
	{
		GameMain::getInstance()->get_sound_manager()->get_sound( "turn" )->play( false );
	}

	is_turn_avaiable_ = false;
}

/**
 * 更新
 */
void Player::update()
{
	if ( velocity().length() > get_max_speed() )
	{
		velocity().normalize() *= get_max_speed();
	}

	const vector3 last_position = position();

	position().x() += velocity().x();
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
		}
		else
		{
			GameMain::getInstance()->get_sound_manager()->get_sound( "collision_wall" )->play( false );
		}

		position().x() = last_position.x();
		velocity().x() *= 0.5f;
	}

	position().z() += velocity().z();
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
		}
		else
		{
			GameMain::getInstance()->get_sound_manager()->get_sound( "collision_wall" )->play( false );
		}

		position().z() = last_position.z();
		velocity().z() *= 0.5f;
	}

	position().y() += velocity().y();
	const GridCell& floor_cell_y = get_floor_cell();
	
	// 着地処理
	if ( position().y() < floor_cell_y.height() )
	{
		position().y() = floor_cell_y.height();

		if ( is_jumping() && floor_cell_y.bound() > 0 )
		{
			// スーパージャンプ
			velocity_.y() = 0.5f;
	
			is_jumping_ = true;

			GameMain::getInstance()->get_sound_manager()->get_sound( "jump" )->play( false );

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
			if ( floor_cell_y.height() == 0 && velocity().y() < -get_max_speed() * 0.5f )
			{
				position().set( 0.f, 0.f, 0.f );
			}

			// 通常着地
			velocity().y() *= -0.01f;

			if ( is_jumping_ )
			{
				GameMain::getInstance()->get_sound_manager()->get_sound( "jump" )->stop();
				GameMain::getInstance()->get_sound_manager()->get_sound( "land" )->play( false );

				is_jumping_ = false;

				velocity().x() = 0.f;
				velocity().z() = 0.f;
			}
		}

		is_turn_avaiable_ = true;
		is_falling_ = false;
	}

	position().y() = std::max( 0.f, position().y() );

	velocity().y() -= 0.01f;
	// velocity().y() -= 0.001f;
	// velocity().y() -= 0.0001f;

	if ( is_jumping() )
	{
		velocity().x() *= 0.99f;
		velocity().z() *= 0.99f;
	}

	if ( ! is_jumping() )
	{
		velocity().x() *= 0.95f;
		velocity().z() *= 0.95f;

		if ( velocity().y() < -get_max_speed() * 0.05f )
		{
			is_jumping_ = true;
		}
	}

	if ( velocity().y() < -10.f )
	{
		velocity().y() = -10.f;
	}
}

/**
 * ジャンプ開始
 * 
 */	
void Player::jump()
{
	if ( is_jumping() ) return;
	
	velocity_.y() = 0.3f;
	// velocity_.z() = 0.1f;
	
	is_jumping_ = true;

	GameMain::getInstance()->get_sound_manager()->get_sound( "jump" )->play( false );
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

/**
 *
 */
float Player::get_max_speed()
{
	return 1.f;
}

float Player::get_collision_width() const
{
	return 0.4f;
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

} // namespace blue_sky

#include "Player.h"
#include "Stage.h"

#include "GameMain.h"

#include "Sound.h"
#include "SoundManager.h"

#include "Direct3D9.h"

#include "matrix4x4.h"

#include <common/exception.h>

namespace blue_sky
{

Player::Player()
	 : is_jumping_( false )
	 , direction_( FRONT )
	 , direction_degree_( 0.f )
	 , stage_( 0 )
{
	//
	turn( 0 );
}

void Player::step( float s )
{
	if ( ! is_jumping() )
	{
		velocity() += front() * s * get_step_speed();
	}
}

void Player::side_step( float s )
{
	if ( ! is_jumping() )
	{
		velocity() += right() * s * get_side_step_speed();
	}
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

	float floor_height = 0.f;

	position().x() += velocity().x();
	floor_height = get_floor_height();

	if ( position().y() < floor_height )
	{
		position().x() = last_position.x();
		velocity().x() *= 0.8f;

		if ( floor_height - position().y() <= 2.f )
		{
			velocity().y() = 0.1f;
		}
	}

	position().z() += velocity().z();
	floor_height = get_floor_height();

	if ( position().y() < floor_height )
	{
		position().z() = last_position.z();
		velocity().z() *= 0.8f;
		
		if ( floor_height - position().y() <= 2.f )
		{
			velocity().y() = 0.1f;
		}
	}

	position().y() += velocity().y();
	floor_height = get_floor_height(); 
	
	if ( position().y() < floor_height )
	{
		// position().y() = last_position.y();

		position().y() = floor_height;
		velocity().y() *= -0.01f;

		if ( is_jumping_ )
		{
			GameMain::getInstance()->getSoundManager()->get_sound( "jump" )->stop();
			GameMain::getInstance()->getSoundManager()->get_sound( "land" )->play( false );

			is_jumping_ = false;

			velocity().x() = 0.f;
			velocity().z() = 0.f;
		}
	}

	position().y() = std::max( 0.f, position().y() );

	velocity().y() -= 0.008f;

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
 * ジャンプ処理
 * 
 */	
void Player::jump()
{
	if ( is_jumping() ) return;
	
	velocity_.y() = 0.5f;
	
	is_jumping_ = true;

	GameMain::getInstance()->getSoundManager()->get_sound( "jump" )->play( false );
}

float Player::get_floor_height_center() const
{
	return static_cast< float >( stage_->chip( static_cast< int >( position_.x() ), static_cast< int >( position_.z() ) ) );
}

float Player::get_floor_height_left_front() const
{
	return stage_->chip( static_cast< int >( position().x() - get_collision_width() * 0.5f ), static_cast< int >( position().z() + get_collision_depth() * 0.5f ) );
}

float Player::get_floor_height_right_front() const
{
	return stage_->chip( static_cast< int >( position().x() + get_collision_width() * 0.5f ), static_cast< int >( position().z() + get_collision_depth() * 0.5f ) );
}

float Player::get_floor_height_left_back() const
{
	return stage_->chip( static_cast< int >( position().x() - get_collision_width() * 0.5f ), static_cast< int >( position().z() - get_collision_depth() * 0.5f ) );	
}

float Player::get_floor_height_right_back() const
{
	return stage_->chip( static_cast< int >( position().x() + get_collision_width() * 0.5f ), static_cast< int >( position().z() - get_collision_depth() * 0.5f ) );
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
	return 10.f;
}

float Player::get_collision_width() const
{
	return 0.8f;
}

float Player::get_collision_depth() const
{
	return 0.8f;
}

float Player::get_floor_height() const
{
	return std::max( std::max( std::max( get_floor_height_left_front(), get_floor_height_right_front() ), get_floor_height_left_back() ), get_floor_height_right_back() );
}

} // namespace blue_sky

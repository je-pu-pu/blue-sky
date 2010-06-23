#include "Player.h"
#include "GameMain.h"
#include "SoundManager.h"
#include "Sound.h"

#include "Direct3D9.h"

#include "matrix4x4.h"

#include <common/exception.h>

namespace blue_sky
{

Player::Player()
	 : jumping_( false )
	 , direction_( FRONT )
{
	//
}

void Player::step( float s )
{
	velocity() += front() * s * get_step_speed();
}

void Player::side_step( float s )
{
	velocity() += right() * s * get_side_step_speed();
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
}

/**
 * 更新
 */
void Player::update()
{
	if ( velocity().length() > max_speed() )
	{
		velocity().normalize() *= max_speed();
	}

	position() += velocity();

	if ( position().y() < get_floor_height() )
	{
		position().y() = get_floor_height();
		velocity().y() *= -0.2f;

		jumping_ = false;

		GameMain::getInstance()->getSoundManager()->get_sound( "test" )->stop();
	}

	velocity().y() -= 0.008f;
	velocity().x() *= 0.95f;
	velocity().z() *= 0.95f;
	
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
	if ( jumping_ ) return;
	
	velocity_.set( 0.f, 0.5f, 0.f );
	
	jumping_ = true;

	GameMain::getInstance()->getSoundManager()->get_sound( "test" )->play( false );
}

/**
 *
 */
float Player::max_speed()
{
	return 10.f;
}

} // namespace blue_sky

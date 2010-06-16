#include "Player.h"

namespace blue_sky
{

Player::Player()
	 : jumping_( false )
{
	//
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
}

/**
 *
 */
float Player::max_speed()
{
	return 10.f;
}

} // namespace blue_sky

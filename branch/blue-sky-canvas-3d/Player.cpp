#include "Player.h"

#include "GameMain.h"
#include "Input.h"

namespace blue_sky
{

Player::Player()
{
	
}

/**
 * �X�V
 *
 */
void Player::update()
{
	get_velocity() = Vector3( 0, 0, 0 );
}

/**
 * �W�����v�J�n
 * 
 */	
void Player::jump()
{
	get_velocity() += Vector3( 0, 5, 0 );

}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	/*
	if ( is_rocketing() )
	{
		return;
	}
	*/

	set_direction_degree( get_direction_degree() + d );
}

} // namespace blue_sky

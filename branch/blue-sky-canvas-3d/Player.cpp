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
	
}

/**
 * �W�����v�J�n
 * 
 */	
void Player::jump()
{
	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity( Vector3( 0, 7.5, 0 ) );
}

} // namespace blue_sky

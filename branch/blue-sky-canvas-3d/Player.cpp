#include "Player.h"

#include "GameMain.h"
#include "Input.h"

namespace blue_sky
{

Player::Player()
{
	
}

/**
 * 更新
 *
 */
void Player::update()
{
	
}

/**
 * ジャンプ開始
 * 
 */	
void Player::jump()
{
	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity( Vector3( 0, 7.5, 0 ) );
}

} // namespace blue_sky

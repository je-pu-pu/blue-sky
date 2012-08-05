#include "Balloon.h"
#include "Player.h"

namespace blue_sky
{

Balloon::Balloon()
	: player_( 0 )
	, flicker_( 0 )
{

}

/**
 * XV
 *
 */
void Balloon::update()
{
	flicker_ += 0.02f;

	if ( player_ )
	{
		if ( player_->get_balloon() == this )
		{
			set_location( player_->get_location() + player_->get_front() * 0.5f - player_->get_right() * 0.5f + Vector3( 0, 2 + std::sin( flicker_ ) * 0.1f, 0 ) );
			set_direction_degree( 0 );

			if ( player_->get_action_mode() != Player::ACTION_MODE_BALLOON )
			{
				kill();
			}
		}
		else
		{
			get_rigid_body()->setActivationState( true );
			get_rigid_body()->setGravity( Vector3( 0, 9.8f, 0 ) );

			if ( get_location().y() >= get_start_location().y() + 100.f )
			{
				kill();
			}
		}
	}
	else
	{
		set_location( get_location().x(), get_start_location().y() + 1.f + std::sin( flicker_ ), get_location().z() );
		set_direction_degree( get_direction_degree() + 1.f );
	}
}

void Balloon::restart()
{
	ActiveObject::restart();

	flicker_ = 0.f;
	player_ = 0;

	get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );
}

} // namespace blue_sky

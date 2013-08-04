#include "Switch.h"
#include "AnimationPlayer.h"
#include "Player.h"
#include "Stone.h"
#include "memory.h"

namespace blue_sky
{

Switch::Switch()
	: state_( OFF )
	, state_switching_timer_( 0.f )
{
	
}

void Switch::restart()
{
	ActiveObject::restart();

	state_ = OFF;
	state_switching_timer_ = 0.f;

	get_animation_player()->play( "Off", false, false );

	get_rigid_body()->setAngularFactor( 0 );
	set_mass( 100.f );
}

/**
 * çXêV
 *
 */
void Switch::update()
{
	//

	state_switching_timer_ -= get_frame_elapsed_time();
	
	if ( state_switching_timer_ < 0.f )
	{
		state_switching_timer_ = 0.f;
	}
}

void Switch::on_collide_with( Player* player )
{
	if ( player->get_velocity().length() >= 0.5f && player->is_moving_to( this ) )
	{
		do_switch();
	}
}

void Switch::on_collide_with( Stone* stone )
{
	if ( stone->get_velocity().length() >= 0.5f )
	{
		do_switch();
	}
}

void Switch::do_switch()
{
	if ( state_switching_timer_ > 0.f )
	{
		return;
	}

	if ( state_ == BROKEN )
	{
		return;
	}

	if ( state_ == ON )
	{
		state_ = OFF;
		state_switching_timer_ = 2.f;

		get_animation_player()->play( "TurnOff", false, false );
		play_sound( "switch-off" );
	}
	else if ( state_ == OFF )
	{
		state_ = ON;
		state_switching_timer_ = 2.f;

		get_animation_player()->play( "TurnOn", false, false );
		play_sound( "switch-on" );
	}
}

void Switch::do_break()
{
	if ( state_ == BROKEN )
	{
		return;
	}

	state_ = BROKEN;

	get_animation_player()->play( "OnOff", false, true );
}

} // namespace blue_sky

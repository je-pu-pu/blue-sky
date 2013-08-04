#include "BaseSwitch.h"
#include "AnimationPlayer.h"
#include "Player.h"
#include "Stone.h"
#include "memory.h"

namespace blue_sky
{

BaseSwitch::BaseSwitch()
	: state_( OFF )
{
	
}

void BaseSwitch::restart()
{
	ActiveObject::restart();

	state_ = OFF;
	contact_object_timer_map_.clear();
}

/**
 * XV
 *
 */
void BaseSwitch::update()
{
	//

	for ( auto i = contact_object_timer_map_.begin(); i != contact_object_timer_map_.end(); )
	{
		i->second -= get_frame_elapsed_time();

		if ( i->second <= 0.f )
		{
			i = contact_object_timer_map_.erase( i );
		}
		else
		{
			++i;
		}
	}
}

void BaseSwitch::on_collide_with( Player* player )
{
	if ( player->get_velocity().length() >= 0.5f && can_game_object_switch( player ) )
	{
		do_switch();
	}

	contact_object_timer_map_[ player ] = 0.5f;
}

void BaseSwitch::on_collide_with( Stone* stone )
{
	if ( stone->get_velocity().length() >= 0.5f && can_game_object_switch( stone ) )
	{
		do_switch();		
	}

	contact_object_timer_map_[ stone ] = 0.5f;
}

bool_t BaseSwitch::do_switch()
{
	if ( state_ == BROKEN )
	{
		return false;
	}

	bool_t result = false;

	if ( state_ == ON )
	{
		if ( turn_off() )
		{
			on_event( "turn-off" );
			return true;
		}
	}
	else if ( state_ == OFF )
	{
		if ( turn_on() )
		{
			on_event( "turn-on" );
			return true;
		}
	}

	return false;
}

bool_t BaseSwitch::do_break()
{
	if ( state_ == BROKEN )
	{
		return false;
	}

	state_ = BROKEN;

	on_break();

	return true;
}

bool_t BaseSwitch::turn_on()
{
	if ( state_ == ON )
	{
		return false;
	}

	state_ = ON;

	on_turn_on();

	return true;
}

bool_t BaseSwitch::turn_off()
{
	if ( state_ == OFF )
	{
		return false;
	}

	state_ = OFF;

	on_turn_off();

	return true;
}

bool_t BaseSwitch::can_game_object_switch( const GameObject* o ) const
{
	auto i = contact_object_timer_map_.find( o );

	if ( i == contact_object_timer_map_.end() )
	{
		return true;
	}

	return false;
}

} // namespace blue_sky

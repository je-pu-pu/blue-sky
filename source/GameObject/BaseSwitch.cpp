#include "BaseSwitch.h"
#include "Player.h"
#include <blue_sky/GameObjectRef.h>

namespace blue_sky
{

BaseSwitch::BaseSwitch()
	: state_( State::OFF )
{
	
}

void BaseSwitch::restart()
{
	ActiveObject::restart();

	state_ = State::OFF;
	contact_object_cache_.clear();
}

/**
 * 更新
 *
 */
void BaseSwitch::update()
{
	contact_object_cache_.update( get_delta_time() );
}

void BaseSwitch::action( const string_t& action )
{
	if ( action == "turn-on" )
	{
		turn_on();
	}
	else if ( action == "turn-off" )
	{
		turn_off();
	}
	else if ( action == "break" )
	{
		do_break();
	}
}

void BaseSwitch::on_collide_with( Player* player )
{
	if ( player->get_velocity().length() >= 0.5f && can_game_object_switch( player->get_entity_id() ) )
	{
		do_switch();
	}

	contact_object_cache_.cache( player->get_entity_id(), 0.5f );
}

/**
 * 石に当たった : 速く動いている石で、最近触れていなければ入・切を切り替える
 *
 * @return 拾わない（false）
 */
bool BaseSwitch::on_collide_with_stone( const GameObjectRef& stone )
{
	if ( stone.get_velocity().length() >= 0.5f && can_game_object_switch( stone.get_entity_id() ) )
	{
		do_switch();		
	}

	contact_object_cache_.cache( stone.get_entity_id(), 0.5f );

	return false;
}

bool_t BaseSwitch::do_switch()
{
	if ( state_ == State::BROKEN )
	{
		return false;
	}

	if ( state_ == State::ON )
	{
		if ( turn_off() )
		{
			on_event( "turn-off" );
			return true;
		}
	}
	else if ( state_ == State::OFF )
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
	if ( state_ == State::BROKEN )
	{
		return false;
	}

	state_ = State::BROKEN;

	on_break();

	return true;
}

bool_t BaseSwitch::turn_on()
{
	if ( state_ == State::ON )
	{
		return false;
	}

	state_ = State::ON;

	on_turn_on();

	return true;
}

bool_t BaseSwitch::turn_off()
{
	if ( state_ == State::OFF )
	{
		return false;
	}

	state_ = State::OFF;

	on_turn_off();

	return true;
}

void BaseSwitch::add_event_handler( const char_t* name, const EventHandler& handler )
{
	event_handler_map_[ name ].push_back( handler );
}

bool_t BaseSwitch::can_game_object_switch( core::ecs::EntityId id ) const
{
	return ! contact_object_cache_.is_cached( id );
}

} // namespace blue_sky

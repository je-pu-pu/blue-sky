#include "ActiveObjectManager.h"
#include "ActiveObject.h"
#include "AnimationPlayer.h"

#include "memory.h"

namespace blue_sky
{

ActiveObjectManager::ActiveObjectManager()
{
	
}

ActiveObjectManager::~ActiveObjectManager()
{
	clear();
}

void ActiveObjectManager::clear()
{
	for ( ActiveObjectList::iterator i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		delete *i;
	}

	active_object_list_.clear();
	named_active_object_map_.clear();
	target_location_map_.clear();
}

void ActiveObjectManager::restart()
{
	for ( auto i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		( *i )->restart();
	}

	target_location_map_.clear();
}

void ActiveObjectManager::add_active_object( ActiveObject* active_object )
{
	active_object_list_.insert( active_object );
}

void ActiveObjectManager::name_active_object( const string_t& name, ActiveObject* active_object )
{
	named_active_object_map_[ name ] = active_object;
}

void ActiveObjectManager::set_target_location( ActiveObject* active_object, const Vector3& target_location, float_t speed )
{
	target_location_map_[ active_object ] = std::make_tuple( target_location, speed );
}

ActiveObject* ActiveObjectManager::get_active_object( const string_t& name )
{
	auto i = named_active_object_map_.find( name );

	if ( i == named_active_object_map_.end() )
	{
		return 0;
	}

	return i->second;
}


void ActiveObjectManager::update()
{
	for ( auto i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		// if ( ! (*i)->is_dead() )
		{
			(*i)->update();
		}

		if ( ( *i )->get_animation_player() )
		{
			( *i )->get_animation_player()->update();
		}
	}

	for ( auto i = target_location_map_.begin(); i != target_location_map_.end(); )
	{
		i->first->update_velocity_by_target_location( std::get< 0 >( i->second ), std::get< 1 >( i->second ) );

		if ( ( std::get< 0 >( i->second ) - i->first->get_location() ).length() < 0.1f )
		{
			i = target_location_map_.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/**
 * ‘S‚Ä‚Ì ActiveObject ‚ð•`‰æ‚·‚é
 *
 */
void ActiveObjectManager::render()
{

}

}; // namespace blue_sky
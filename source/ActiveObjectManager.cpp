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
}

void ActiveObjectManager::add_active_object( ActiveObject* active_object )
{
	active_object_list_.insert( active_object );
}

void ActiveObjectManager::name_active_object( const string_t& name, ActiveObject* active_object )
{
	named_active_object_map_[ name ] = active_object;
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
	for ( ActiveObjectList::iterator i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
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
}

/**
 * ‘S‚Ä‚Ì ActiveObject ‚ð•`‰æ‚·‚é
 *
 */
void ActiveObjectManager::render()
{

}

}; // namespace blue_sky
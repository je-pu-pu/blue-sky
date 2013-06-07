#include "ActiveObjectManager.h"
#include "ActiveObject.h"
#include "AnimationPlayer.h"

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
}

void ActiveObjectManager::add_active_object( ActiveObject* active_object )
{
	active_object_list_.insert( active_object );
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
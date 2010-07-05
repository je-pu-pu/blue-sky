#include "GridObjectManager.h"
#include "GridObject.h"
#include "GridData.h"
#include "Direct3D9Mesh.h"

namespace blue_sky
{

GridObjectManager::GridObjectManager()
{
	
}

GridObjectManager::~GridObjectManager()
{
	clear();
}

void GridObjectManager::clear()
{
	for ( GridObjectList::iterator i = grid_object_list_.begin(); i != grid_object_list_.end(); ++i )
	{
		delete *i;
	}

	grid_object_list_.clear();
}

void GridObjectManager::add_grid_object( GridObject* grid_object )
{
	grid_object_list_.insert( grid_object );
}


}; // namespace blue_sky
#include "GridDataManager.h"
#include "GridData.h"

namespace blue_sky
{

GridDataManager::GridDataManager()
{
	
}

GridDataManager::~GridDataManager()
{
	clear();
}

void GridDataManager::clear()
{
	for ( GridDataList::iterator i = grid_data_list_.begin(); i != grid_data_list_.end(); ++i )
	{
		delete i->second;
	}

	grid_data_list_.clear();
}

GridData* GridDataManager::load( const char* name )
{
	GridDataList::const_iterator i = grid_data_list_.find( name );

	if ( i != grid_data_list_.end() )
	{
		return i->second;
	}

	GridData* grid_data = GridData::load_file( ( std::string( "media/object/" ) + name ).c_str() );

	if ( ! grid_data )
	{
		return 0;
	}

	grid_data_list_[ name ] = grid_data;
	grid_data->set_name( name );

	return grid_data;
}


}; // namespace blue_sky
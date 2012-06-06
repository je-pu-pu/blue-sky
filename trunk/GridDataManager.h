#ifndef BLUE_SKY_GRID_DATA_MANAGER_H
#define BLUE_SKY_GRID_DATA_MANAGER_H

#include <map>
#include <string>

namespace blue_sky
{

class GridData;

/**
 * •¡”‚Ì GridData ‚ğŠÇ—‚·‚é Manager
 *
 */
class GridDataManager
{
public:
	typedef std::map< std::string, GridData* > GridDataList;

private:
	GridDataList grid_data_list_;

public:
	GridDataManager();
	~GridDataManager();

	void clear();	
	GridData* load( const char* );

	GridDataList& grid_data_list() { return grid_data_list_; }
	const GridDataList& grid_data_list() const { return grid_data_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_DATA_MANAGER_H
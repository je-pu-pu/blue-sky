#ifndef BLUE_SKY_GRID_OBJECT_MANAGER_H
#define BLUE_SKY_GRID_OBJECT_MANAGER_H

#include <set>

namespace blue_sky
{

class GridObject;

/**
 * ï°êîÇÃ GridObject Çä«óùÇ∑ÇÈ Manager
 *
 */
class GridObjectManager
{
public:
	typedef std::set< GridObject* > GridObjectList;

private:
	GridObjectList grid_object_list_;

public:
	GridObjectManager();
	~GridObjectManager();

	void add_grid_object( GridObject* );

	GridObjectList& grid_object_list() { return grid_object_list_; }
	const GridObjectList& grid_object_list() const { return grid_object_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_OBJECT_MANAGER_H
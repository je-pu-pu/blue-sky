#ifndef BLUE_SKY_GRID_DATA_H
#define BLUE_SKY_GRID_DATA_H

#include "GridCell.h"

namespace blue_sky
{

/**
 * グリッド上に配置されるオブジェクトのデータ
 *
 */
class GridData
{
private:
	int width_;
	int depth_;

	GridCell* cell_;

	static GridCell null_cell_;
public:
	GridData( int, int );
	virtual ~GridData();

	int width() const { return width_; }
	int depth() const { return depth_; }

	GridCell& cell( int, int );
	const GridCell& cell( int, int ) const;

	void put( int, int, int, const GridData* );
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_DATA_H
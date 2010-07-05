#ifndef BLUE_SKY_GRID_DATA_H
#define BLUE_SKY_GRID_DATA_H

#include "GridCell.h"
#include <string>

class Direct3D9Mesh;

namespace blue_sky
{

/**
 * グリッド上に配置されるオブジェクトのデータ
 *
 */
class GridData
{
public:
	typedef Direct3D9Mesh Mesh;

private:
	std::string name_;

	int width_;
	int depth_;

	GridCell* cell_;
	const Mesh* mesh_;

	static GridCell null_cell_;

	GridData();
	static const Mesh* load_mesh( const char* );

public:
	static GridData* load_file( const char* );

	GridData( int, int, const Mesh* );
	virtual ~GridData();

	const char* get_name() const { return name_.c_str(); }
	void set_name( const char* name ) { name_ = name; }

	int width() const { return width_; }
	int depth() const { return depth_; }

	GridCell& cell( int, int );
	const GridCell& cell( int, int ) const;

	const Mesh* mesh() const { return mesh_; }

	void put( int, int, int, const GridData* );
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_DATA_H
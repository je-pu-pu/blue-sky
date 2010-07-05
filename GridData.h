#ifndef BLUE_SKY_GRID_DATA_H
#define BLUE_SKY_GRID_DATA_H

#include "GridCell.h"

class Direct3D9Mesh;

namespace blue_sky
{

/**
 * �O���b�h��ɔz�u�����I�u�W�F�N�g�̃f�[�^
 *
 */
class GridData
{
public:
	typedef Direct3D9Mesh Mesh;

private:
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

	int width() const { return width_; }
	int depth() const { return depth_; }

	GridCell& cell( int, int );
	const GridCell& cell( int, int ) const;

	const Mesh* mesh() const { return mesh_; }

	void put( int, int, int, const GridData* );
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_DATA_H
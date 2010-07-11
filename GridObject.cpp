#include "GridObject.h"

#include "GridData.h"
#include "Direct3D9Mesh.h"

namespace blue_sky
{

GridObject::GridObject( int x, int y, int z, int r, GridData* grid_data )
	: x_( x )
	, y_( y )
	, z_( z )
	, rotate_degree_( r )
	, grid_data_( grid_data )
{
	
}

GridObject::~GridObject()
{
	
}

int GridObject::width() const { return grid_data_->width(); }
int GridObject::depth() const { return grid_data_->depth(); }

const GridObject::Mesh* GridObject::mesh() const { return grid_data_->mesh(); }

}; // namespace blue_sky
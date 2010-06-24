#include "GridObject.h"

#include "GridData.h"
#include "Direct3D9Mesh.h"

namespace blue_sky
{

GridObject::GridObject( int x, int y, int z, GridData* grid_data, Mesh* mesh )
	: x_( x )
	, y_( y )
	, z_( z )
	, grid_data_( grid_data )
	, mesh_( mesh )
{
	
}

GridObject::~GridObject()
{
	
}

int GridObject::width() const { return grid_data_->width(); }
int GridObject::depth() const { return grid_data_->depth(); }

}; // namespace blue_sky
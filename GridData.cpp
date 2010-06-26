#include "GridData.h"
#include "GridCell.h"

#include <common/exception.h>
#include <cassert>

namespace blue_sky
{

GridCell GridData::null_cell_( 0, 0 ); 

GridData::GridData( int w, int d )
	: width_( w )
	, depth_( d )
	, cell_( new GridCell[ width_ * depth_ ] )
{
	
}

GridData::~GridData()
{
	delete [] cell_;
}

GridCell& GridData::cell( int x, int z )
{
	assert( x >= 0 );
	assert( z >= 0 );
	assert( x < width_ );
	assert( z < depth_ );

	return cell_[ z * width_ + x ];
}

const GridCell& GridData::cell( int x, int z ) const
{
	if ( x < 0 ) return null_cell_;
	if ( z < 0 ) return null_cell_;
	if ( x >= width_ ) return null_cell_;
	if ( z >= depth_ ) return null_cell_;

	return cell_[ z * width_ + x ];
}

void GridData::put( int px, int py, int pz, const GridData* grid_data )
{
	for ( int z = 0; z < grid_data->depth(); z++ )
	{
		for ( int x = 0; x < grid_data->width(); x++ )
		{
			cell( px + x, pz + z ).bound() = grid_data->cell( x, z ).bound();
			cell( px + x, pz + z ).height() = std::max( 0, py + grid_data->cell( x, z ).height() );
		}
	}
}

}; // namespace blue_sky
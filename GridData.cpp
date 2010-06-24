#include "GridData.h"

#include <common/exception.h>
#include <cassert>

namespace blue_sky
{

GridData::GridData( int w, int d )
	: width_( w )
	, depth_( d )
	, chip_( new ChipType[ width_ * depth_ ] )
{
	for ( int z = 0; z < depth_; z++ )
	{
		for ( int x = 0; x < width_; x++ )
		{
			chip( x, z ) = 0;
		}
	}
}

GridData::~GridData()
{
	delete [] chip_;
}

GridData::ChipType& GridData::chip( int x, int z )
{
	assert( x >= 0 );
	assert( z >= 0 );
	assert( x < width_ );
	assert( z < depth_ );

	return chip_[ z * width_ + x ];
}

GridData::ChipType GridData::chip( int x, int z ) const
{
	if ( x < 0 ) return 0;
	if ( z < 0 ) return 0;
	if ( x >= width_ ) return 0;
	if ( z >= depth_ ) return 0;

	return chip_[ z * width_ + x ];
}

void GridData::put( int px, int py, int pz, const GridData* grid_data )
{
	for ( int z = 0; z < grid_data->depth(); z++ )
	{
		for ( int x = 0; x < grid_data->width(); x++ )
		{
			chip( px + x, pz + z ) = std::max( 0, py + grid_data->chip( x, z ) );
		}
	}
}

}; // namespace blue_sky
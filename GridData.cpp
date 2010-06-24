#include "GridData.h"

#include <common/math.h>
#include <common/random.h>

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
	static ChipType none = 0;

	if ( x < 0 ) return none;
	if ( z < 0 ) return none;
	if ( x >= width_ ) return none;
	if ( z >= depth_ ) return none;

	return chip_[ z * width_ + x ];
}

GridData::ChipType GridData::chip( int x, int z ) const
{
	static ChipType none = 0;

	if ( x < 0 ) return none;
	if ( z < 0 ) return none;
	if ( x >= width_ ) return none;
	if ( z >= depth_ ) return none;

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
#include "Stage.h"

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

Stage::Stage( int w, int d )
	: width_( w )
	, depth_( d )
	, map_chip_( new MapChipType[ width_ * depth_ ] )
{
	for ( int x = 0; x < width_; x++ )
	{
		map_chip( x, 0 ) = 0;
	}

	for ( int z = 1; z < depth_; z++ )
	{
		for ( int x = 0; x < width_; x++ )
		{
			if ( common::random( 0, 1 ) == 0 )
			{
				map_chip( x, z ) = map_chip( x, z - 1 ) + 2;
			}
			else
			{
				map_chip( x, z ) = map_chip( x, z - 1 ); // math::clamp( map_chip( x, z - 1 ) + common::random( -1, 1 ), 0, 100 );
			}
		}
	}
}

Stage::~Stage()
{
	delete [] map_chip_;
}

Stage::MapChipType& Stage::map_chip( int x, int z )
{
	static MapChipType none = 0;

	if ( x < 0 ) return none;
	if ( z < 0 ) return none;
	if ( x >= width_ ) return none;
	if ( z >= depth_ ) return none;

	return map_chip_[ z * width_ + x ];
}

}; // namespace blue_sky
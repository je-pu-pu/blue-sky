#include "Stage.h"

#include <common/random.h>

namespace blue_sky
{

Stage::Stage( int w, int d )
	: width_( w )
	, depth_( d )
	, map_chip_( new MapChipType[ width_ * depth_ ] )
{
	for ( int z = 0; z < depth_; z++ )
	{
		for ( int x = 0; x < width_; x++ )
		{
			map_chip( x, z ) = common::random( 0, 255 );
		}
	}
}

Stage::~Stage()
{
	delete [] map_chip_;
}

Stage::MapChipType& Stage::map_chip( int x, int z )
{
	return map_chip_[ z * width_ + x ];
}

}; // namespace blue_sky
#include "Building.h"

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

Building::Building( int w, int d )
	: GridData( w, d )
{
	ChipType data[]  = {
		20,20,20,20,20,20,20,20,20,20,
		20,19,19,19,19,19,19,19,19,20,
		20,19,21,21,19,19,19,19,19,20,
		20,19,21,21,19,19,19,19,19,20,
		20,19,19,19,19,19,19,19,19,20,
		20,19,19,19,19,19,19,19,19,20,
		20,19,19,19,19,19,19,19,19,20,
		20,19,19,19,19,19,19,19,19,20,
		20,19,19,19,19,19,19,19,19,20,
		20,20,20,20,20,20,20,20,20,20
	};

	for ( int z = 0; z < depth(); z++ )
	{
		for ( int x = 0; x < width(); x++ )
		{
			int aa = data[ ( depth() - z - 1 ) * width() + x ];
			chip( x, z ) = aa;
		}
	}
}

Building::~Building()
{
	
}

}; // namespace blue_sky
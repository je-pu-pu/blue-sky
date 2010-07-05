#include "Building.h"
#include "GridCell.h"

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

Building::Building( int w, int d, const Mesh* mesh )
	: GridData( w, d, mesh )
{
	unsigned char data[]  = {
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
			cell( x, z ).height() = data[ ( depth() - z - 1 ) * width() + x ];
		}
	}

	cell( 2, 6 ).bound() = 10;
	cell( 3, 6 ).bound() = 10;
	cell( 2, 7 ).bound() = 10;
	cell( 3, 7 ).bound() = 10;
}

Building::~Building()
{
	
}

}; // namespace blue_sky
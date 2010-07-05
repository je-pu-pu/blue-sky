#include "House.h"
#include "GridCell.h"

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

House::House( int w, int d, const Mesh* mesh )
	: GridData( w, d, mesh )
{
	unsigned char data[]  = {
		0,7,7,7,7,7,7,7,
		3,7,7,7,7,7,7,7,
		3,7,7,7,7,7,7,7,
		3,7,7,7,7,7,7,7,
		3,7,7,7,7,7,7,7,
		3,3,3,3,3,0,0,0
	};

	for ( int z = 0; z < depth(); z++ )
	{
		for ( int x = 0; x < width(); x++ )
		{
			cell( x, z ).height() = data[ ( depth() - z - 1 ) * width() + x ];

			if ( cell( x, z ).height() == 7 )
			{
				cell( x, z ).bound() = 1;
			}
		}
	}


}

House::~House()
{
	
}

}; // namespace blue_sky
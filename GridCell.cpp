#include "GridCell.h"

#include <common/exception.h>
#include <cassert>

namespace blue_sky
{

GridCell::GridCell()
	: height_( 0 )
	, bound_( 0 )
{

}

GridCell::GridCell( unsigned char h, unsigned  char b )
	: height_( h )
	, bound_( b )
{

}

GridCell::~GridCell()
{
	
}

}; // namespace blue_sky
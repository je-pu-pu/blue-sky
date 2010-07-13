#include "Stage.h"

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

Stage::Stage( int w, int d )
	: GridData( w, d )
{
	aabb_list_.push_back( new AABB( vector3( 20.f, 4.f, 20.f ), vector3( 22.f, 5.f, 21.f ) ) );
}

Stage::~Stage()
{
	for ( AABBList::iterator i = aabb_list_.begin(); i != aabb_list_.end(); ++i )
	{
		delete *i;
	}
}

const Stage::AABB* Stage::get_collision_aabb( const AABB& aabb ) const
{
	for ( AABBList::const_iterator i = aabb_list_.begin(); i != aabb_list_.end(); ++i )
	{
		if ( (*i)->collision_detection( aabb ) )
		{
			return *i;
		}
	}

	return 0;
}

}; // namespace blue_sky
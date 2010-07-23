#include "Goal.h"

namespace blue_sky
{

Goal::Goal()
{
	local_aabb_list().clear();
	local_aabb_list().push_back( AABB( vector3( -2, -1, -2 ), vector3( 2, 1, 2 ) ) );
}

/**
 * çXêV
 *
 */
void Goal::update()
{

}

} // namespace blue_sky

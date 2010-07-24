#include "Balloon.h"

namespace blue_sky
{

Balloon::Balloon()
{
	local_aabb_list().clear();
	local_aabb_list().push_back( AABB( vector3( -1, -1, -1 ), vector3( 1, 1, 1 ) ) );
}

/**
 * çXêV
 *
 */
void Balloon::update()
{
	update_global_aabb_list();
}

} // namespace blue_sky

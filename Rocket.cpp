#include "Rocket.h"

namespace blue_sky
{

Rocket::Rocket()
{
	local_aabb_list().clear();
	local_aabb_list().push_back( AABB( vector3( -2, -2, -2 ), vector3( 2, 2, 2 ) ) );
}

/**
 * çXêV
 *
 */
void Rocket::update()
{
	set_direction_degree( get_direction_degree() + 1.f );

	update_global_aabb_list();
}

void Rocket::restart()
{
	ActiveObject::restart();
}

} // namespace blue_sky

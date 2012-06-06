#include "Rocket.h"

namespace blue_sky
{

Rocket::Rocket()
{
	setup_local_aabb_list();
}

/**
 * XV
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

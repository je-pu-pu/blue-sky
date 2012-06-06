#include "Medal.h"

namespace blue_sky
{

Medal::Medal()
{
	update_global_aabb_list();
}

/**
 * XV
 *
 */
void Medal::update()
{
	set_direction_degree( get_direction_degree() + 1.f );
}

void Medal::restart()
{
	ActiveObject::restart();
}

} // namespace blue_sky

#include "Poison.h"

namespace blue_sky
{

Poison::Poison()
{
	update_global_aabb_list();
}

/**
 * çXêV
 *
 */
void Poison::update()
{
	set_direction_degree( get_direction_degree() + 1.f );
}

void Poison::restart()
{
	ActiveObject::restart();
}

} // namespace blue_sky

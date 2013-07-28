#include "Rocket.h"

namespace blue_sky
{

Rocket::Rocket()
{
	
}

void Rocket::restart()
{
	ActiveObject::restart();
}

/**
 * XV
 *
 */
void Rocket::update()
{
	// set_direction_degree( get_direction_degree() + 1.f );
}

} // namespace blue_sky

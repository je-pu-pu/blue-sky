#include "Rocket.h"

#include "memory.h"

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
 * çXêV
 *
 */
void Rocket::update()
{
	// set_direction_degree( get_direction_degree() + 1.f );
}

void Rocket::on_collide_with( Player* )
{
	kill();
}

} // namespace blue_sky

#include "Rocket.h"

namespace blue_sky
{

Rocket::Rocket()
{
	
}

void Rocket::restart()
{
	ActiveObject::restart();

	set_no_contact_response( false );
}

/**
 * XV
 *
 */
void Rocket::update()
{
	// set_direction_degree( get_direction_degree() + 1.f );
}

void Rocket::on_collide_with( Player* )
{
	kill();

	set_no_contact_response( true );
}

} // namespace blue_sky

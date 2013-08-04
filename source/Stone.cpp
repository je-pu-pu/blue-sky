#include "Stone.h"

#include "memory.h"

namespace blue_sky
{

Stone::Stone()
{
	
}

void Stone::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setGravity( Vector3( 0, -9.0, 0 ) );
}

/**
 * çXêV
 *
 */
void Stone::update()
{
	set_direction_degree( get_direction_degree() + 1.f );
}

void Stone::on_collide_with( Player* )
{
	kill();
}

} // namespace blue_sky

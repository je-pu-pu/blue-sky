#include "Stone.h"

#include "memory.h"

namespace blue_sky
{

Stone::Vector3 Stone::GravityDefault( 0.f, -8.f, 0.f );

Stone::Stone()
{
	
}

void Stone::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setCcdSweptSphereRadius( 0.2f );
	get_rigid_body()->setCcdMotionThreshold( 0.2f );
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

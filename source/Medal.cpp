#include "Medal.h"

#include "memory.h"

namespace blue_sky
{

Medal::Medal()
{

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

	get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );
}

} // namespace blue_sky

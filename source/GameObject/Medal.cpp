#include "Medal.h"

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
	set_direction_degree( get_direction_degree() + per_sec( 90.f ) );
}

void Medal::restart()
{
	ActiveObject::restart();
}

} // namespace blue_sky

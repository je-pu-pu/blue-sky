#include "Goal.h"

#include "memory.h"

namespace blue_sky
{

Goal::Goal()
{

}

void Goal::restart()
{
	ActiveObject::restart();

	set_mass( 100.f );
}

/**
 * çXêV
 *
 */
void Goal::update()
{

}

} // namespace blue_sky

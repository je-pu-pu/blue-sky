#include "Goal.h"

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
 * XV
 *
 */
void Goal::update()
{

}

} // namespace blue_sky

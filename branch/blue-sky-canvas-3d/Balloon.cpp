#include "Balloon.h"

namespace blue_sky
{

Balloon::Balloon()
	: flicker_( 0 )
{

}

/**
 * çXêV
 *
 */
void Balloon::update()
{
	flicker_ += 0.02f;

	set_location( get_location().x(), get_start_location().y() + 1.f + std::sin( flicker_ ), get_location().z() );
	set_direction_degree( get_direction_degree() + 1.f );
}

void Balloon::restart()
{
	ActiveObject::restart();

	flicker_ = 0.f;
}

} // namespace blue_sky

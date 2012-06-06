#include "Balloon.h"

namespace blue_sky
{

Balloon::Balloon()
	: flicker_( 0 )
{
	setup_local_aabb_list();
}

/**
 * çXêV
 *
 */
void Balloon::update()
{
	flicker_ += 0.02f;

	position().y() = start_position().y() + std::sin( flicker_ );
	set_direction_degree( get_direction_degree() + 1.f );

	update_global_aabb_list();
}

void Balloon::restart()
{
	ActiveObject::restart();

	flicker_ = 0.f;
}

} // namespace blue_sky

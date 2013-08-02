#include "Umbrella.h"

#include "memory.h"

namespace blue_sky
{

Umbrella::Umbrella()
	: flicker_( 0 )
{
	setup_local_aabb_list();
}

/**
 * çXêV
 *
 */
void Umbrella::update()
{
	flicker_ += 0.01f;

	position().y() = start_position().y() + std::sin( flicker_ );
	set_direction_degree( get_direction_degree() + 1.f );

	update_global_aabb_list();
}

void Umbrella::restart()
{
	ActiveObject::restart();

	flicker_ = 0.f;
}

} // namespace blue_sky

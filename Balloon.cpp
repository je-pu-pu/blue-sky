#include "Balloon.h"

namespace blue_sky
{

Balloon::Balloon()
	: flicker_( 0 )
{
	local_aabb_list().clear();
	local_aabb_list().push_back( AABB( vector3( -1, -1, -1 ), vector3( 1, 1, 1 ) ) );
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

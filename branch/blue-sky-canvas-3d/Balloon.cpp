#include "Balloon.h"

namespace blue_sky
{

Balloon::Balloon()
	: flicker_( 0 )
{

}

/**
 * XV
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

	get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );
}

} // namespace blue_sky

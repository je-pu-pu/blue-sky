#include "Goal.h"

namespace blue_sky
{

Goal::Goal()
{

}

void Goal::restart()
{
	get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );
}

/**
 * XV
 *
 */
void Goal::update()
{

}

} // namespace blue_sky

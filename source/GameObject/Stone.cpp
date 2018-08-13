#include "Stone.h"
#include <btBulletDynamicsCommon.h> ///< @todo Bullet を直接使わない

namespace blue_sky
{

Vector Stone::GravityDefault( 0.f, -8.f, 0.f );

Stone::Stone()
{
	
}

void Stone::restart()
{
	ActiveObject::restart();

	// 石が薄い壁を貫通しないようにする
	/// @todo Bullet を直接使わない
	get_rigid_body()->setCcdSweptSphereRadius( 0.2f );
	get_rigid_body()->setCcdMotionThreshold( 0.2f );
}

/**
 * 更新
 *
 */
void Stone::update()
{
	set_direction_degree( get_direction_degree() + 1.f );
}

void Stone::on_collide_with( Player* )
{
	kill();
}

} // namespace blue_sky

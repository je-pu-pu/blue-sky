#include "Girl.h"
#include "Player.h"
#include "AnimationPlayer.h"
#include <common/random.h>

#include "memory.h"

namespace blue_sky
{

Girl::Girl()
	: flicker_( 0.f )
{
	
}

/**
 * リスタート時の処理
 *
 */
void Girl::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setAngularFactor( 0 );
	get_rigid_body()->setFriction( 0 );

	flicker_ = common::random( 0.f, 10.f );

	get_animation_player()->play( "Float", true, true );
	get_animation_player()->set_speed( 0.5f );
}

/**
 * 更新
 *
 */
void Girl::update()
{
	chase_direction_to( player_->get_location(), 1.f );

	set_velocity( Vector3( 0.f, 0.f, 0.f ) );
	update_location_by_flicker( get_start_location() );
}

} // namespace blue_sky

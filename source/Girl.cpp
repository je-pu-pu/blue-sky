#include "Girl.h"
#include "AnimationPlayer.h"
#include <common/random.h>

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
	// ActiveObject::restart();

	/// @todo まとめる
	// get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );

	flicker_ = common::random( 0.f, 10.f );

	get_animation_player()->play( "Walk", true, true );
}

/**
 * 更新
 *
 */
void Girl::update()
{
	set_location( get_location().x(), get_start_location().y() + 1.f + std::sin( flicker_ ), get_location().z() );
}

} // namespace blue_sky

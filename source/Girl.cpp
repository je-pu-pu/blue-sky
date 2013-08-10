#include "Girl.h"
#include "Player.h"
#include "AnimationPlayer.h"
#include <common/random.h>

#include "memory.h"

namespace blue_sky
{

Girl::Girl()
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

	mode_ = MODE_STAND;
	get_animation_player()->play( "Stand", true, true );

	flicker_base_location_ = get_start_location();
}

/**
 * 更新
 *
 */
void Girl::update()
{
	if ( mode_ == MODE_FLOAT )
	{
		set_velocity( Vector3( 0.f, 0.f, 0.f ) );
		update_velocity_by_flicker( flicker_base_location_, get_flicker_scale() );
	}

	/*
	get_animation_player()->play( "Float", true, true );
	get_animation_player()->set_speed( 0.5f );

	chase_direction_to( player_->get_location(), 1.f );
	*/
}

void Girl::action( const string_t& s )
{
	ActiveObject::action( s );

	if ( s == "float" )
	{
		mode_ = MODE_FLOAT;
	}
}

} // namespace blue_sky

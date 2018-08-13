#include "Girl.h"
#include "Player.h"
#include <common/random.h>

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

	set_angular_factor( 0 );
	set_friction( 0 );

	mode_ = MODE_STAND;

	play_animation( "Stand", true, true );

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
		set_velocity( Vector( 0.f, 0.f, 0.f ) );
		update_velocity_by_flicker( flicker_base_location_, get_flicker_scale() );
	}

	/*
	play_animation( "Float", true, true );
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

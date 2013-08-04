#include "Switch.h"
#include "AnimationPlayer.h"
#include "Player.h"
#include "Stone.h"
#include "memory.h"

namespace blue_sky
{

Switch::Switch()
{
	
}

void Switch::restart()
{
	BaseSwitch::restart();

	get_animation_player()->play( "Off", false, false );

	get_rigid_body()->setAngularFactor( 0 );
	set_mass( 100.f );
}

/**
 * XV
 *
 */
void Switch::update()
{
	BaseSwitch::update();
}

void Switch::on_break()
{
	get_animation_player()->play( "OnOff", false, true );
}

void Switch::on_turn_on()
{
	get_animation_player()->play( "TurnOn", false, false );
	play_sound( "switch-on" );
}

void Switch::on_turn_off()
{
	get_animation_player()->play( "TurnOff", false, false );
	play_sound( "switch-off" );
}

} // namespace blue_sky

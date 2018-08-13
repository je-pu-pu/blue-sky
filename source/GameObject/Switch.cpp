#include "Switch.h"
#include "Player.h"
#include "Stone.h"

namespace blue_sky
{

Switch::Switch()
{
	
}

void Switch::restart()
{
	BaseSwitch::restart();

	play_animation( "Off", false, false );

	set_angular_factor( 0.f );
	set_friction( 1.5f );
	set_mass( 100.f );
}

/**
 * çXêV
 *
 */
void Switch::update()
{
	BaseSwitch::update();
}

void Switch::on_break()
{
	play_animation( "OnOff", false, true );
}

void Switch::on_turn_on()
{
	play_animation( "TurnOn", false, false );
	play_sound( "switch-on" );
}

void Switch::on_turn_off()
{
	play_animation( "TurnOff", false, false );
	play_sound( "switch-off" );
}

} // namespace blue_sky

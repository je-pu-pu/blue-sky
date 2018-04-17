#include "AreaSwitch.h"

namespace blue_sky
{

AreaSwitch::AreaSwitch( float_t w, float_t h, float_t d )
	: width_( w )
	, height_( h )
	, depth_( d )
{
	
}

void AreaSwitch::restart()
{
	BaseSwitch::restart();
}

/**
 * çXêV
 *
 */
void AreaSwitch::update()
{
	BaseSwitch::update();
}

} // namespace blue_sky

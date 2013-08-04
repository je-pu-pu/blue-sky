#include "Umbrella.h"

#include "memory.h"

namespace blue_sky
{

Umbrella::Umbrella()
	: flicker_( 0 )
{
	
}

/**
 * çXêV
 *
 */
void Umbrella::update()
{

}

void Umbrella::restart()
{
	ActiveObject::restart();
}

void Umbrella::on_collide_with( Player* )
{
	kill();
}

} // namespace blue_sky

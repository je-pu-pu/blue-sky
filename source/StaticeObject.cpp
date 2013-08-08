#include "StaticObject.h"

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include "memory.h"

namespace blue_sky
{

StaticObject::StaticObject( float_t w, float_t h, float_t d, float_t mass )
	: collision_width_( w )
	, collision_height_( h )
	, collision_depth_( d )
	, mass_( mass )
{
	
}

StaticObject::~StaticObject()
{
	
}

void StaticObject::update()
{

}

} // namespace blue_sky

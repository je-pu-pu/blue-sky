#include "StaticObject.h"
#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

StaticObject::StaticObject( float_t w, float_t h, float_t d )
	: collision_width_( w )
	, collision_height_( h )
	, collision_depth_( d )
{
	
}

StaticObject::~StaticObject()
{
	
}

void StaticObject::update_transform()
{
	ActiveObject::update_transform();
}

} // namespace blue_sky

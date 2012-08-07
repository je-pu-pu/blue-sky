#include "DynamicObject.h"

#include "Player.h"
#include "StaticObject.h"

#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

DynamicObject::DynamicObject( float_t w, float_t h, float_t d )
	: collision_width_( w )
	, collision_height_( h )
	, collision_depth_( d )
{
	
}

DynamicObject::~DynamicObject()
{
	
}

void DynamicObject::update_transform()
{
	ActiveObject::update_transform();
}

void DynamicObject::on_collide_with( Player* o )
{
	play_collision_sound( o );
}

void DynamicObject::on_collide_with( StaticObject* o )
{
	play_collision_sound( o );
}

void DynamicObject::on_collide_with_ground()
{
	play_collision_sound( 0 );
}

void DynamicObject::play_collision_sound( const ActiveObject* o )
{
	if ( get_velocity().length() < 1.f )
	{
		return;
	}

	if ( ! o || o->is_hard() )
	{
		play_sound( "soda-can-long-1", false, false );
	}
}

} // namespace blue_sky

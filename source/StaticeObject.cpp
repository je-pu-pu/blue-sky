#include "StaticObject.h"

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include "memory.h"

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

void StaticObject::update()
{
	/*
	get_rigid_body()->setActivationState( true );

	static float a = 0.f;
	a += 0.0025f;

	// Quaternion q( 0.f, a, 0.f );
	// get_rigid_body()->getWorldTransform().setRotation( q );
	// get_rigid_body()->getInterpolationWorldTransform().setRotation( q );

	// get_rigid_body()->getWorldTransform().getOrigin().setX( std::sin( a ) * 2.5f );
	
	set_velocity( get_velocity() + Vector3( 0.01f, 0, 0 ) );
	

	// get_rigid_body()->setCenterOfMassTransform( t );
	// get_rigid_body()->setWorldTransform( get_transform() );
	// get_rigid_body()->setInterpolationWorldTransform( get_transform() );
	*/
}

} // namespace blue_sky

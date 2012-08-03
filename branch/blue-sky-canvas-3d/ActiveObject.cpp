#include "ActiveObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "GameMain.h"

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: rigid_body_( 0 )
	, is_dead_( false )
	, transform_( 0 )

	, direction_degree_( 0 )

	, start_location_( 0, 0, 0 )

	, front_( 0, 0, 1 )
	, right_( 1, 0, 0 )
{
	transform_ = new Transform();
	transform_->setIdentity();
}

ActiveObject::~ActiveObject()
{
	delete transform_;
}

void ActiveObject::restart()
{
	is_dead_ = false;
	
	if ( get_rigid_body() && transform_ )
	{
		get_transform().setOrigin( start_location_ + Vector3( 0, get_collision_height() / 2.f, 0 ) );
		get_rigid_body()->getMotionState()->setWorldTransform( get_transform() );
		
		get_rigid_body()->setWorldTransform( get_transform() );
		get_rigid_body()->setInterpolationWorldTransform( get_transform() );
		
		get_rigid_body()->setLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->clearForces();
	}
}

void ActiveObject::limit_velocity()
{
	Vector3 v = get_rigid_body()->getLinearVelocity();

	v.setX( math::clamp( v.x(), -get_max_speed(), get_max_speed() ) );
	v.setZ( math::clamp( v.z(), -get_max_speed(), get_max_speed() ) );

	get_rigid_body()->setLinearVelocity( v );
}

void ActiveObject::update_rigid_body_velocity()
{
	limit_velocity();

	get_rigid_body()->setActivationState( true );
	// get_rigid_body()->setLinearVelocity( get_velocity() );
}

void ActiveObject::update_transform()
{
	if ( rigid_body_ )
	{
		rigid_body_->getMotionState()->getWorldTransform( * transform_ );
		Transform offset;
		offset.setIdentity();
		offset.setOrigin( Vector3( 0, -get_collision_height() / 2.f, 0 ) );

		*transform_ = *transform_ * offset;
	}
}

void ActiveObject::set_start_location( float_t x, float_t y, float_t z )
{
	start_location_.setValue( x, y, z );
	get_transform().getOrigin() = start_location_;
}

void ActiveObject::set_location( float_t x, float_t y, float_t z )
{
	get_transform().getOrigin().setValue( x, y, z );
}

void ActiveObject::set_rotation( float_t x, float_t y, float_t z )
{
	Quaternion q;
	q.setEuler( math::degree_to_radian( x ), math::degree_to_radian( y ), math::degree_to_radian( z ) );

	get_transform().setRotation( q );
}

ActiveObject::Transform& ActiveObject::get_transform()
{
	return * transform_;
}

const ActiveObject::Transform& ActiveObject::get_transform() const
{
	return * transform_;
}

const ActiveObject::Vector3& ActiveObject::get_velocity() const
{
	return get_rigid_body()->getLinearVelocity();
}



void ActiveObject::set_direction_degree( float d )
{
	direction_degree_ = d;

	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	Matrix m;
	m.setEulerZYX( 0, math::degree_to_radian( -direction_degree_ ), 0 );

	front_ = Vector3( 0.f, 0.f, 1.f ) * m;
	right_ = Vector3( 1.f, 0.f, 0.f ) * m;
}

ActiveObject::DynamicsWorld* ActiveObject::get_dynamics_world() const
{
	return GameMain::get_instance()->get_physics()->get_dynamics_world();
}

void ActiveObject::kill()
{
	is_dead_ = true;
}

float ActiveObject::get_elapsed_time() const
{
	return GameMain::get_instance()->get_elapsed_time();
}

void ActiveObject::play_sound( const char* name, bool loop, bool force ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		if ( force || ! sound->is_playing() )
		{
			sound->set_3d_position( get_location().x(), get_location().y(), get_location().z() );
			sound->set_3d_velocity( get_velocity().x() * 60.f, get_velocity().z() * 60.f, get_velocity().z() * 60.f );
			sound->play( loop );
		}
	}
}

void ActiveObject::stop_sound( const char* name ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

} // namespace blue_sky

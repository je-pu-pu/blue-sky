#include "GameObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "GameMain.h"

#include <common/math.h>

namespace blue_sky
{

GameObject::GameObject()
	: rigid_body_( 0 )
	, transform_( 0 )
{
	transform_ = new Transform();
	transform_->setIdentity();
}

GameObject::~GameObject()
{
	delete transform_;
}

void GameObject::update_transform()
{
	if ( rigid_body_ )
	{
		* transform_ = rigid_body_->getWorldTransform();
		Transform offset;
		offset.setIdentity();
		offset.setOrigin( Vector3( 0, -get_height_offset(), 0 ) );

		*transform_ = *transform_ * offset;
	}
}

void GameObject::set_mass( float_t mass )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	btVector3 local_inertia( 0, 0, 0 );
	get_rigid_body()->getCollisionShape()->calculateLocalInertia( mass, local_inertia );
	get_rigid_body()->setMassProps( mass, local_inertia );
}

GameObject::Transform& GameObject::get_transform()
{
	return * transform_;
}

const GameObject::Transform& GameObject::get_transform() const
{
	return * transform_;
}

void GameObject::set_location( const Vector3& v )
{
	get_transform().getOrigin() = v;

	if ( rigid_body_ )
	{
		get_rigid_body()->getMotionState()->setWorldTransform( get_transform() );
		
		get_rigid_body()->setWorldTransform( get_transform() );
		get_rigid_body()->setInterpolationWorldTransform( get_transform() );
	}
}

void GameObject::set_location( float_t x, float_t y, float_t z )
{
	set_location( Vector3( x, y, z ) );
}

const GameObject::Vector3& GameObject::get_velocity() const
{
	return get_rigid_body()->getLinearVelocity();
}

void GameObject::set_velocity( const Vector3& v )
{
	get_rigid_body()->setActivationState( true );

	get_rigid_body()->setLinearVelocity( v );
	// get_rigid_body()->setInterpolationLinearVelocity( v );
}

void GameObject::on_collide_with( GameObject* o )
{
	o->on_collide_with( this );
}

float GameObject::get_elapsed_time() const
{
	return GameMain::get_instance()->get_elapsed_time();
}

GameObject::DynamicsWorld* GameObject::get_dynamics_world() const
{
	return GameMain::get_instance()->get_physics()->get_dynamics_world();
}

void GameObject::play_sound( const char* name, bool loop, bool force ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		if ( force || ! sound->is_playing() )
		{
			sound->set_3d_position( get_location().x(), get_location().y(), get_location().z() );
			sound->set_3d_velocity( get_velocity().x(), get_velocity().z(), get_velocity().z() );
			sound->play( loop );
		}
	}
}

void GameObject::stop_sound( const char* name ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

void GameObject::fade_out_sound( const char* name ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->fade_out();
	}
}

} // namespace blue_sky

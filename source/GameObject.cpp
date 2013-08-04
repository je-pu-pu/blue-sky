#include "GameObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "GameMain.h"
#include "Scene.h"

#include <common/math.h>

#include "memory.h"

namespace blue_sky
{

GameObject::Vector3 GameObject::GravityDefault( 0.f, -9.8f, 0.f );
GameObject::Vector3 GameObject::GravityZero( 0.f, 0.f, 0.f );

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

/**
 * @todo 調べる : set_rotation_degree() で設定した角度がクリアされる？
 *
 */
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
	if ( v == get_velocity() )
	{
		return;
	}

	get_rigid_body()->setLinearVelocity( v );
	get_rigid_body()->setInterpolationLinearVelocity( v );

	get_rigid_body()->activate();
	get_rigid_body()->setActivationState( true );

}

bool GameObject::is_moving_to( const GameObject* o ) const
{
	Vector3 v = get_velocity();
	v.normalize();
	v *= 0.01f;

	float_t current_length = ( o->get_location() - get_location() ).length();
	float_t next_length = ( o->get_location() - ( get_location() + v ) ).length();

	return next_length < current_length;
}

void GameObject::on_collide_with( GameObject* o )
{
	o->on_collide_with( this );
}

/**
 * 前のフレームからの経過秒を返す
 *
 * @return 前のフレームからの経過秒
 */
float_t GameObject::get_frame_elapsed_time() const
{
	return GameMain::get_instance()->get_elapsed_time();
}

/**
 * 現在のシーンが開始してからの経過秒を返す
 *
 * @return 現在のシーンが開始してからの経過秒
 */
float_t GameObject::get_scene_elapsed_time() const
{
	return GameMain::get_instance()->get_current_scene()->get_total_elapsed_time();
}

/**
 * 点滅時においてオブジェクトが表示されるかどうかを返す
 *
 * @param blink_count 1 秒の間に点滅する回数
 * @return bool 点滅時において、現在オブジェクトが表示される場合は true を、表示されない場合は false を返す
 */
bool_t GameObject::is_visible_in_blink( float_t blink_count ) const
{
	return std::fmodf( get_scene_elapsed_time(), 1.f / blink_count ) < ( 0.5f / blink_count );
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

#include "GameObject.h"

#include <ActiveObjectPhysics.h>
#include <GameMain.h>

#include <Scene/Scene.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

// GameObject::Vector3 GameObject::GravityDefault( 0.f, -3.711f, 0.f ); // 火星
Vector GameObject::GravityDefault( 0.f, -9.8f, 0.f, 0.f ); // 地球

GameObject::GameObject()
	: rigid_body_( 0 )
{
	transform_.set_identity();
}

GameObject::~GameObject()
{
	
}

/**
 * 変換行列を物理演算の結果で更新する
 *
 */
void GameObject::update_transform()
{
	if ( ! rigid_body_ )
	{
		return;
	}

	transform_ = Transform( rigid_body_->getWorldTransform() );
	Transform offset;
	offset.set_identity();
	offset.set_position( Vector( 0, -get_height_offset(), 0 ) );

	transform_ = transform_ * offset;
}

/**
 * 変換行列を物理演算に送る
 *
 */
void GameObject::commit_transform()
{
	if ( ! rigid_body_ )
	{
		return;
	}

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0, get_height_offset(), 0 ) );


	btTransform t = btTransform( transform_ ) * offset;
	rigid_body_->setWorldTransform( t );
	rigid_body_->setInterpolationWorldTransform( t );
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

void GameObject::set_gravity( const Vector& gravity )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	get_rigid_body()->setGravity( gravity );
}

void GameObject::set_kinematic( bool is_kinematic )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	if ( is_kinematic )
	{
		get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
	}
	else
	{
		get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT );
	}
}

void GameObject::set_no_contact_response( bool is_no_contact_response )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	if ( is_no_contact_response )
	{
		get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
	else
	{
		get_rigid_body()->setCollisionFlags( get_rigid_body()->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
}

/**
 * 摩擦係数を設定する
 *
 * @param v 摩擦係数
 */
void GameObject::set_friction( float_t v )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	get_rigid_body()->setFriction( v );
}

/**
 * 回転係数を設定する
 *
 * @param v 回転係数
 */
void GameObject::set_angular_factor( float_t v )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	get_rigid_body()->setAngularFactor( v );
}

/**
 * 回転係数を設定する
 *
 * @param v 軸毎の回転係数
 */
void GameObject::set_angular_factor( const Vector& v )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	get_rigid_body()->setAngularFactor( v );
}

Transform& GameObject::get_transform()
{
	return transform_;
}

const Transform& GameObject::get_transform() const
{
	return transform_;
}

/**
 * 
 *
 */
void GameObject::set_location( const Vector& v )
{
	get_transform().set_position( v );
	commit_transform();
}

void GameObject::set_location( float_t x, float_t y, float_t z )
{
	set_location( Vector( x, y, z ) );
}

const Vector& GameObject::get_velocity() const
{
	return reinterpret_cast< const Vector& >( get_rigid_body()->getLinearVelocity() );
}

void GameObject::set_velocity( const Vector& v )
{
	if ( v == get_velocity() )
	{
		return;
	}

	get_rigid_body()->setLinearVelocity( v );
	get_rigid_body()->setInterpolationLinearVelocity( v );

	get_rigid_body()->activate( true );
	get_rigid_body()->setActivationState( true );
}

bool GameObject::is_moving_to( const GameObject* o ) const
{
	Vector v = get_velocity();
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
 * ゆらぎによる高さのオフセットを取得する
 *
 * @param scale ゆらぎの大きさ
 * @return ゆらぎによる高さのオフセット
 */
float_t GameObject::get_flicker_height_offset( float_t scale ) const
{
	common::random_set_seed( reinterpret_cast< int >( this ) );
	float_t a = common::random( 0.f, 10.f );

	return std::cos( a + get_scene_elapsed_time() ) * scale * 0.5f;
}

/**
 * 指定した位置にゆらぎによる高さのオフセットを加え、現在の位置として設定する
 *
 * @param base_location ゆらぎの中心となる位置
 * @param scale ゆらぎのスケール値
 */
void GameObject::update_location_by_flicker( const Vector& base_location, float_t scale )
{
	set_location( base_location + Vector( 0.f, get_flicker_height_offset( scale ), 0.f ) );
}

/**
 * 指定した位置にゆらぎによる高さのオフセットを加え、その位置を目標地点として移動量を設定する
 *
 * @param base_location ゆらぎの中心となる位置
 * @param scale ゆらぎのスケール値
 */
void GameObject::update_velocity_by_flicker( const Vector& base_location, float_t scale )
{
	Vector target_location = base_location + Vector( 0.f, get_flicker_height_offset( scale ), 0.f );
	Vector relative_position = target_location - get_location();

	set_velocity( Vector( get_velocity().x(), relative_position.y(), get_velocity().z() ) );
}

/**
 * 指定した位置に移動するための移動量を設定する
 *
 * @param target_location 目的地
 * @param speed 速度
 */
void GameObject::update_velocity_by_target_location( const Vector& target_location, float_t speed )
{
	Vector relative_position = target_location - get_location();

	if ( relative_position.length() > 0.1f )
	{
		relative_position.normalize();
	}

	set_velocity( relative_position * speed );
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

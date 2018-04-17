#include "GameObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "GameMain.h"
#include "Scene.h"

#include <common/math.h>
#include <common/random.h>

#include "memory.h"

namespace blue_sky
{

// GameObject::Vector3 GameObject::GravityDefault( 0.f, -3.711f, 0.f ); // �ΐ�
GameObject::Vector3 GameObject::GravityDefault( 0.f, -9.8f, 0.f ); // �n��
GameObject::Vector3 GameObject::GravityZero( 0.f, 0.f, 0.f );

GameObject::GameObject()
	: rigid_body_( 0 )
{
	transform_.setIdentity();
}

GameObject::~GameObject()
{
	
}

/**
 * �ϊ��s��𕨗����Z�̌��ʂōX�V����
 *
 */
void GameObject::update_transform()
{
	if ( ! rigid_body_ )
	{
		return;
	}

	transform_ = rigid_body_->getWorldTransform();
	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, -get_height_offset(), 0 ) );

	transform_ = transform_ * offset;
}

/**
 * �ϊ��s��𕨗����Z�ɑ���
 *
 */
void GameObject::commit_transform()
{
	if ( ! rigid_body_ )
	{
		return;
	}

	Transform offset;
	offset.setIdentity();
	offset.setOrigin( Vector3( 0, get_height_offset(), 0 ) );


	Transform t = transform_ * offset;
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

void GameObject::set_gravity( const Vector3& gravity )
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

void GameObject::set_angular_factor( const Vector3& v )
{
	if ( ! get_rigid_body() )
	{
		return;
	}

	get_rigid_body()->setAngularFactor( v );
}

GameObject::Transform& GameObject::get_transform()
{
	return transform_;
}

const GameObject::Transform& GameObject::get_transform() const
{
	return transform_;
}

/**
 * 
 *
 */
void GameObject::set_location( const Vector3& v )
{
	get_transform().getOrigin() = v;
	commit_transform();
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

	get_rigid_body()->activate( true );
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
 * �O�̃t���[������̌o�ߕb��Ԃ�
 *
 * @return �O�̃t���[������̌o�ߕb
 */
float_t GameObject::get_frame_elapsed_time() const
{
	return GameMain::get_instance()->get_elapsed_time();
}

/**
 * ���݂̃V�[�����J�n���Ă���̌o�ߕb��Ԃ�
 *
 * @return ���݂̃V�[�����J�n���Ă���̌o�ߕb
 */
float_t GameObject::get_scene_elapsed_time() const
{
	return GameMain::get_instance()->get_current_scene()->get_total_elapsed_time();
}

/**
 * ��炬�ɂ�鍂���̃I�t�Z�b�g���擾����
 *
 * @param scale ��炬�̑傫��
 * @return ��炬�ɂ�鍂���̃I�t�Z�b�g
 */
float_t GameObject::get_flicker_height_offset( float_t scale ) const
{
	common::random_set_seed( reinterpret_cast< int >( this ) );
	float_t a = common::random( 0.f, 10.f );

	return std::cos( a + get_scene_elapsed_time() ) * scale * 0.5f;
}

/**
 * �w�肵���ʒu�ɂ�炬�ɂ�鍂���̃I�t�Z�b�g�������A���݂̈ʒu�Ƃ��Đݒ肷��
 *
 * @param base_location ��炬�̒��S�ƂȂ�ʒu
 * @param scale ��炬�̃X�P�[���l
 */
void GameObject::update_location_by_flicker( const Vector3& base_location, float_t scale )
{
	set_location( base_location + Vector3( 0.f, get_flicker_height_offset( scale ), 0.f ) );
}

/**
 * �w�肵���ʒu�ɂ�炬�ɂ�鍂���̃I�t�Z�b�g�������A���̈ʒu��ڕW�n�_�Ƃ��Ĉړ��ʂ�ݒ肷��
 *
 * @param base_location ��炬�̒��S�ƂȂ�ʒu
 * @param scale ��炬�̃X�P�[���l
 */
void GameObject::update_velocity_by_flicker( const Vector3& base_location, float_t scale )
{
	Vector3 target_location = base_location + Vector3( 0.f, get_flicker_height_offset( scale ), 0.f );
	Vector3 relative_position = target_location - get_location();

	set_velocity( Vector3( get_velocity().x(), relative_position.y(), get_velocity().z() ) );
}

/**
 * �w�肵���ʒu�Ɉړ����邽�߂̈ړ��ʂ�ݒ肷��
 *
 * @param target_location �ړI�n
 * @param speed ���x
 */
void GameObject::update_velocity_by_target_location( const Vector3& target_location, float_t speed )
{
	Vector3 relative_position = target_location - get_location();

	if ( relative_position.length() )
	{
		relative_position.normalize();
		set_velocity( relative_position * speed );
	}
}

/**
 * �_�Ŏ��ɂ����ăI�u�W�F�N�g���\������邩�ǂ�����Ԃ�
 *
 * @param blink_count 1 �b�̊Ԃɓ_�ł����
 * @return bool �_�Ŏ��ɂ����āA���݃I�u�W�F�N�g���\�������ꍇ�� true ���A�\������Ȃ��ꍇ�� false ��Ԃ�
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

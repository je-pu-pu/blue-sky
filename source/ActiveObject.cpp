#include "ActiveObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "DrawingModel.h"
#include "AnimationPlayer.h"
#include "GameMain.h"

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: drawing_model_( 0 )
	, object_constant_buffer_( new ObjectConstantBuffer( GameMain::get_instance()->get_direct_3d() ) )
	, animation_player_( 0 )
	, is_dead_( false )

	, direction_degree_( 0 )

	, start_location_( 0, 0, 0 )
	, start_rotation_( 0, 0, 0 )
	, start_direction_degree_( 0 )

	, front_( 0, 0, 1 )
	, right_( 1, 0, 0 )
{
	
}

ActiveObject::~ActiveObject()
{
	delete object_constant_buffer_;
	delete animation_player_;
}

/**
 * アニメーション再生をセットアップする
 *
 */
void ActiveObject::setup_animation_player()
{
	if ( animation_player_ )
	{
		return;
	}

	if ( get_drawing_model()->has_animation() )
	{
		animation_player_ = get_drawing_model()->create_animation_player();
	}
}

void ActiveObject::restart()
{
	is_dead_ = false;
	
	if ( get_rigid_body() )
	{
		get_transform().setOrigin( start_location_ + Vector3( 0, get_height_offset(), 0 ) );
// 		get_transform().setRotation( Quaternion( math::degree_to_radian( start_rotation_.x() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.z() ) ) );

		get_rigid_body()->activate( true );
		get_rigid_body()->getMotionState()->setWorldTransform( get_transform() );
		
		get_rigid_body()->setWorldTransform( get_transform() );
		get_rigid_body()->setInterpolationWorldTransform( get_transform() );
		
		get_rigid_body()->setLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->clearForces();

		set_direction_degree( start_direction_degree_ );
	}
}

void ActiveObject::update()
{

}

void ActiveObject::limit_velocity()
{
	Vector3 v = get_rigid_body()->getLinearVelocity();

	v.setX( math::clamp( v.x(), -get_max_speed(), get_max_speed() ) );
	v.setZ( math::clamp( v.z(), -get_max_speed(), get_max_speed() ) );

	set_velocity( v );
}

void ActiveObject::set_start_location( float_t x, float_t y, float_t z )
{
	start_location_.setValue( x, y, z );
	get_transform().getOrigin() = start_location_;
}

void ActiveObject::set_start_rotation( float_t x, float_t y, float_t z )
{
	start_rotation_.setValue( x, y, z );

	Quaternion q;
	q.setEulerZYX( math::degree_to_radian( start_rotation_.z() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.x() ) );
	get_transform().setRotation( q );
}

void ActiveObject::set_start_direction_degree( float d )
{
	start_direction_degree_ = d;
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

	front_.normalize();
	right_.normalize();

	Transform t = get_rigid_body()->getCenterOfMassTransform();

	m.setEulerZYX( 0.f, math::degree_to_radian( get_direction_degree() ), 0.f );
	t.setBasis( m );
	get_rigid_body()->setCenterOfMassTransform( t );
}

/**
 * 方向を指定した目的の方向に近づける
 *
 * @param d 目的の方向
 * @param speed 速度
 */
void ActiveObject::chase_direction_degree( float_t d, float_t speed )
{
	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	while ( d < 0.f ) d += 360.f;
	while ( d > 360.f ) d -= 360.f;

	float_t diff = d - direction_degree_;

	while ( diff < -180.f ) diff += 360.f;
	while ( diff >  180.f ) diff -= 360.f;

	set_direction_degree( math::chase( direction_degree_, direction_degree_ + diff, speed ) );
}

void ActiveObject::kill()
{
	is_dead_ = true;

	/*
	if ( get_rigid_body() )
	{
		set_location( 0.f, -100.f, 0.f );
	}
	*/
}

} // namespace blue_sky

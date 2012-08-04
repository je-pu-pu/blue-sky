#include "Player.h"

#include "Balloon.h"

#include "DrawingModel.h"
#include "DrawingLine.h"

#include "GameMain.h"
#include "Input.h"

#include <common/math.h>

#include <btBulletCollisionCommon.h>

namespace blue_sky
{

Player::Player()
	: is_on_footing_( false )
	, is_jumping_( false )
	, is_jumpable_( false )
	, action_mode_( ACTION_MODE_NONE )
	, uncontrollable_timer_( 0.f )
	, eye_height_( 1.5f )
	, eye_depth_( 0.f )
	, has_medal_( false )
{
	
}

/**
 * 再スタート
 *
 */
void Player::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setAngularFactor( 0 );
	get_rigid_body()->setFriction( 0 );

	is_on_footing_ = false;
	is_jumping_ = false;
	is_jumpable_ = false;

	eye_height_ = 1.5f;
	eye_depth_ = 0.f;

	set_action_mode( ACTION_MODE_NONE );
	has_medal_ = false;

	set_last_footing_height_to_current_height();
}

/**
 * 更新
 *
 */
void Player::update()
{
	stop();
	limit_velocity();

	update_on_footing();
	update_jumpable();
	update_jumping();

	if ( action_mode_ == ACTION_MODE_BALLOON )
	{
		get_rigid_body()->setLinearVelocity(
			Vector3(
				get_rigid_body()->getLinearVelocity().x(),
				math::chase( get_rigid_body()->getLinearVelocity().y(), 3.f, 0.5f ),
				get_rigid_body()->getLinearVelocity().z()
			)
		);

		if ( get_location().y() - action_base_position_.y() >= get_balloon_action_length() )
		{
			play_sound( "balloon-burst" );
			set_action_mode( ACTION_MODE_NONE );
			is_jumping_ = true;
			is_action_pre_finish_ = false;
		}
		else if ( get_location().y() - action_base_position_.y() >= get_balloon_action_length() * 0.5f )
		{
			is_action_pre_finish_ = true;
		}
	}

	if ( ! is_on_footing() )
	{
		eye_depth_ *= 0.95f;
		eye_depth_ = math::clamp( eye_depth_, 0.f, 1.f );
	}

	if ( is_falling_to_die() )
	{
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 1.f, 0.f, 0.f, 0.f ) );
	}
	else
	{
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.f, 0.f, 0.f, -0.25f ) );
	}

	if ( is_on_footing() )
	{
		if ( is_falling_to_die() )
		{
			kill();
		}
		else
		{
			set_last_footing_height_to_current_height();
		}
	}

	if ( is_dead() )
	{
		eye_height_ *= 0.95f;
		eye_height_ = std::max( eye_height_, 0.15f );
	}

	uncontrollable_timer_ = math::chase< float_t >( uncontrollable_timer_, 0.f, get_elapsed_time() );
}

/**
 * 現在ジャンプ可能かどうかのフラグを更新する
 *
 */
void Player::update_jumpable()
{
	is_jumpable_ = false;

	// ジャンプ直後と着地直後はジャンプできない
	if ( abs( get_velocity().y() ) > 3.f )
	{
		return;
	}

	is_jumpable_ = is_on_footing();
}

void Player::update_on_footing()
{
	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_rigid_body()->getCenterOfMassPosition().y();
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();
	const float_t ray_length = get_collision_height() * 0.5f + 0.01f;

	// 中心 + 四隅の設置を調べる
	if (
		check_on_footing( Vector3( x, y, z ), ray_length ) ||
		check_on_footing( Vector3( x - get_collision_width() / 2.f, y, z - get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x + get_collision_width() / 2.f, y, z - get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x - get_collision_width() / 2.f, y, z + get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x + get_collision_width() / 2.f, y, z + get_collision_depth() / 2.f ), ray_length ) )
	{
		is_on_footing_ = true;
	}
	else
	{
		is_on_footing_ = false;
	}
}

/**
 * 現在ジャンプ中かどうかのフラグを更新する
 *
 */
void Player::update_jumping()
{
	if ( is_jumping_ && is_jumpable() )
	{
		is_jumping_ = false;
	}
}

class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
{
private:
	const Player::RigidBody* rigid_body_;

public:
	ClosestNotMe( const btVector3& s, const btVector3& d, const Player::RigidBody* rb )
		: ClosestRayResultCallback( s, d )
		, rigid_body_( rb )
	{ }

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if ( rayResult.m_collisionObject == rigid_body_ )
		{
			return 1.0;
		}

		if ( rayResult.m_collisionObject->getUserPointer() )
		{
			ActiveObject* a = reinterpret_cast< ActiveObject* >( rayResult.m_collisionObject->getUserPointer() );
			
			if ( ! a->is_hard() )
			{
				return 1.0;
			}
		}

		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

bool Player::check_on_footing( const Vector3& from, float_t ray_length ) const
{
	Vector3 to = from + Vector3( 0, -ray_length, 0 );
	
	ClosestNotMe ray_callback( from, to, get_rigid_body() );
	ray_callback.m_closestHitFraction = 1.0;

	get_dynamics_world()->rayTest( from, to, ray_callback );

	return ray_callback.hasHit();
}

void Player::step( float_t s )
{
	if ( is_uncontrollable() )
	{
		return;
	}

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity(
		Vector3(
			get_rigid_body()->getLinearVelocity().x() + ( get_front() * s ).x() * get_step_speed(),
			get_rigid_body()->getLinearVelocity().y(),
			get_rigid_body()->getLinearVelocity().z() + ( get_front() * s ).z() * get_step_speed()
		)
	);

	// eye_depth_ *= 0.95f;
}

void Player::side_step( float_t s )
{
	if ( is_uncontrollable() )
	{
		return;
	}

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity(
		Vector3(
			get_rigid_body()->getLinearVelocity().x() + ( get_right() * s ).x() * get_side_step_speed(),
			get_rigid_body()->getLinearVelocity().y(),
			get_rigid_body()->getLinearVelocity().z() + ( get_right() * s ).z() * get_side_step_speed()
		)
	);

	// eye_depth_ *= 0.95f;
}

/**
 * ジャンプ開始
 * 
 */	
void Player::jump()
{
	if ( ! is_jumpable() )
	{
		return;
	}

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity(
		Vector3(
			get_rigid_body()->getLinearVelocity().x(),
			get_rigid_body()->getLinearVelocity().y() + 4.f,
			get_rigid_body()->getLinearVelocity().z()
		)
	);

	is_jumping_ = true;
}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	/*
	if ( is_rocketing() )
	{
		return;
	}
	*/

	set_direction_degree( get_direction_degree() + d );

	/*
	Transform t = get_rigid_body()->getCenterOfMassTransform();
	t.getRotation().setEulerZYX( 0, get_direction_degree(), 0 );

	get_rigid_body()->setCenterOfMassTransform( t );
	*/

	Transform t = get_rigid_body()->getCenterOfMassTransform();

	Matrix m;
	m.setEulerZYX( 0.f, math::degree_to_radian( get_direction_degree() ), 0.f );
	t.setBasis( m );
	get_rigid_body()->setCenterOfMassTransform( t );
}

void Player::stop()
{
	if ( is_uncontrollable() )
	{
		return;
	}

	Vector3 v = get_rigid_body()->getLinearVelocity();

	if ( is_jumping() )
	{
		v.setX( v.x() * 0.95f );
		v.setZ( v.z() * 0.95f );
	}
	else
	{
		v.setX( v.x() * 0.9f );
		v.setZ( v.z() * 0.9f );
	}

	get_rigid_body()->setLinearVelocity( v );
}

void Player::damage( const Vector3& to )
{
	uncontrollable_timer_ = 1.f;

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity( to );

	is_jumping_ = true;
}

void Player::kill()
{
	if ( is_dead() )
	{
		return;
	}

	ActiveObject::kill();

	play_sound( "dead" );

	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setAngularFactor( 1.f );
	get_rigid_body()->setFriction( 1.f );
	get_rigid_body()->applyForce( get_front() * 2000.f, Vector3( 0.1f, 1.5f, 0.f ) );
}

void Player::set_eye_depth( float d )
{
	if ( ! is_on_footing() )
	{
		return;
	}

	eye_depth_ = math::clamp( d, 0.f, 1.f );
}

void Player::add_eye_depth( float d )
{
	set_eye_depth( eye_depth_ + d );
}

bool Player::is_falling() const
{
	return get_rigid_body()->getLinearVelocity().y() < -1.f;
}

/**
 * このまま落下すると死亡するかどうかを取得する
 *
 */
bool Player::is_falling_to_die() const
{
	/*
	if ( get_rigid_body()->getLinearVelocity().y() < -15.f )
	{
		return true;
	}
	*/

	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_last_footing_height() + get_collision_height() * 0.5f;
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();
	const float_t ray_length = get_collision_height() * 0.5f + get_height_to_die();

	// 中心 + 四隅の設置を調べる
	if (
		check_on_footing( Vector3( x, y, z ), ray_length ) ||
		check_on_footing( Vector3( x - get_collision_width() / 2.f, y, z - get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x + get_collision_width() / 2.f, y, z - get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x - get_collision_width() / 2.f, y, z + get_collision_depth() / 2.f ), ray_length ) ||
		check_on_footing( Vector3( x + get_collision_width() / 2.f, y, z + get_collision_depth() / 2.f ), ray_length ) )
	{
		return false;
	}

	return true;
}

void Player::on_collide_with( Balloon* balloon )
{
	balloon->kill();

	is_jumping_ = false;

	/*
	is_falling_ = false;
	is_action_pre_finish_ = false;
	*/

	set_action_mode( ACTION_MODE_BALLOON );
	set_action_base_position_to_current_position();

	// set_last_footing_height_to_current_height();

	play_sound( "balloon-get" );
}

void Player::on_collide_with( Robot* robot )
{
	Vector3 v = -get_front() * 20.f;
	v.setY( 10.f );

	damage( v );
}

void Player::set_action_mode( ActionMode action_mode )
{
	if ( action_mode == action_mode_ )
	{
		return;
	}

	/*
	// 自動傘選択
	if ( action_mode_ != ACTION_MODE_UMBRELLA && action_mode_ != ACTION_MODE_NONE )
	{
		if ( action_mode == ACTION_MODE_NONE && item_count_[ ITEM_TYPE_UMBRELLA ] && floor_cell() && is_if_fall_to_die( floor_cell()->height() ) )
		{
			action_mode = ACTION_MODE_UMBRELLA;
		}
	}
	*/

	action_mode_ = action_mode;

	if ( action_mode_ == ACTION_MODE_NONE )
	{
		get_rigid_body()->setGravity( Vector3( 0, -9.8f, 0 ) );
	}
	else
	{
		get_rigid_body()->setGravity( Vector3( 0, 0, 0 ) );
	}
}

} // namespace blue_sky

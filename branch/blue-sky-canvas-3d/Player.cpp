#include "Player.h"

#include "GameMain.h"
#include "Input.h"

#include <btBulletCollisionCommon.h>

namespace blue_sky
{

Player::Player()
	: is_jumping_( false )
	, is_jumpable_( true )
{
	
}

/**
 * 更新
 *
 */
void Player::update()
{
	stop();

	update_jumpable();
	update_jumping();
}

/**
 * 現在ジャンプ可能かどうかのフラグを更新する
 *
 */
void Player::update_jumpable()
{
	// 中心 + 四隅の設置を調べる
	if (
		is_on_ground( 0, 0 ) ||
		is_on_ground( -get_collision_width() / 2.f, -get_collision_depth() / 2.f ) ||
		is_on_ground( +get_collision_width() / 2.f, -get_collision_depth() / 2.f ) ||
		is_on_ground( -get_collision_width() / 2.f, +get_collision_depth() / 2.f ) ||
		is_on_ground( +get_collision_width() / 2.f, +get_collision_depth() / 2.f ) )
	{
		is_jumpable_ = true;
	}
	else
	{
		is_jumpable_ = false;
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
		if (rayResult.m_collisionObject == rigid_body_ )
		{
			return 1.0;
		}

		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

bool Player::is_on_ground( float_t offset_x, float_t offset_z ) const
{
	Vector3 from = get_rigid_body()->getCenterOfMassPosition()  + Vector3( offset_x, -0.1f, offset_z );
	Vector3 to = from + Vector3( 0, -( get_collision_height() / 2.f + 0.01f ), 0 );
	
	ClosestNotMe ray_callback( from, to, get_rigid_body() );
	ray_callback.m_closestHitFraction = 1.0;

	get_dynamics_world()->rayTest( from, to, ray_callback );

	return ray_callback.hasHit();
}

void Player::step( float_t s )
{
	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity(
		Vector3(
			get_rigid_body()->getLinearVelocity().x() + ( get_front() * s ).x() * get_step_speed(),
			get_rigid_body()->getLinearVelocity().y(),
			get_rigid_body()->getLinearVelocity().z() + ( get_front() * s ).z() * get_step_speed()
		)
	);
}

void Player::side_step( float_t s )
{
	get_rigid_body()->setActivationState( true );
	get_rigid_body()->setLinearVelocity(
		Vector3(
			get_rigid_body()->getLinearVelocity().x() + ( get_right() * s ).x() * get_side_step_speed(),
			get_rigid_body()->getLinearVelocity().y(),
			get_rigid_body()->getLinearVelocity().z() + ( get_right() * s ).z() * get_side_step_speed()
		)
	);
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
			get_rigid_body()->getLinearVelocity().y() + 5.f,
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
}

void Player::stop()
{
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

} // namespace blue_sky

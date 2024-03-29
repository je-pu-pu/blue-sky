#include "Player.h"

#include "Ladder.h"
#include "Balloon.h"
#include "Rocket.h"
#include "Medal.h"
#include "Robot.h"
#include "Stone.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

#include <common/math.h>
#include <common/serialize.h>

namespace blue_sky
{

Player::Player()
	: is_on_footing_( false )
	, is_jumping_( false )
	, is_jumpable_( false )
	, is_clambering_( false )
	, is_located_on_die_( false )
	, is_located_on_safe_( false )
	, is_on_ladder_( false )
	, is_facing_to_block_( false )
	, can_clamber_( false )
	, can_peer_down_( false )
	, can_throw_( false )
	, is_flickering_( false )
	, step_count_( 0 )
	, step_speed_( 0.25f )
	, action_mode_( ActionMode::NONE )
	, action_timer_( 0.f )
	, is_action_pre_finish_( false )
	, balloon_sequence_count_( 0 )
	, balloon_sound_request_( 0 )
	, uncontrollable_timer_( 0.f )
	, pitch_( 0.f )
	, eye_height_( 1.5f )
	, eye_depth_( 0.f )
	, has_medal_( false )
	, selected_item_type_( ItemType::NONE )
	, last_footing_height_( 0.f )
	, ladder_( nullptr )
	, balloon_( nullptr )
	, hp_( 1 )
{
	// set_rigid_body( get_physics_manager()->create_capsule_rigid_body( this ) );
}

/**
 * 再スタート
 *
 */
void Player::restart()
{
	ActiveObject::restart();

	set_angular_factor( 0.f );
	set_friction( 0.f );
	set_mass( 50.f );

	is_on_footing_ = false;
	is_jumping_ = false;
	is_jumpable_ = false;
	is_clambering_ = false;
	is_located_on_die_ = false;
	is_located_on_safe_ = false;
	is_on_ladder_ = false;
	is_facing_to_block_ = false;
	can_clamber_ = false;
	can_peer_down_ = false;
	can_throw_ = false;

	is_flickering_ = false;

	step_count_ = 0;
	step_speed_ = 0.25f;

	set_action_mode( ActionMode::NONE );

	is_action_pre_finish_ = false;
	balloon_sequence_count_ = 0;
	balloon_sound_request_ = 0;

	pitch_ = 0.f;
	
	eye_height_ = 1.5f;
	eye_depth_ = 0.f;

	has_medal_ = false;

	for ( int n = 0; n < static_cast< int >( ItemType::MAX ); n++ )
	{
		item_count_[ n ] = 0;
	}
	item_count_[ static_cast< int >( ItemType::SCOPE ) ] = 1;

	selected_item_type_ = ItemType::NONE;

	ladder_ = nullptr;
	balloon_ = nullptr;
	stone_list_.clear();

	hp_ = 1;

	set_last_footing_height_to_current_height();
}

/**
 * 更新
 *
 */
void Player::update()
{
	get_rigid_body()->setActivationState( true );

	update_on_footing();
	update_jumpable();
	update_jumping();
	update_located_to_die();
	update_facing_to_block();
	update_can_clamber();
	update_can_peer_down();
	update_can_throw();

	action_timer_ += get_frame_elapsed_time();

	if ( action_mode_ == ActionMode::BALLOON )
	{
		set_velocity( Vector( get_velocity().x(), std::min( 3.f, math::chase( get_velocity().y(), 3.f, get_velocity().y() < 0.f ? 0.5f : 0.25f ) ), get_velocity().z() ) );

		if ( get_location().y() - action_base_position_.y() >= get_balloon_action_length() )
		{
			play_sound( "balloon-burst" );
			set_action_mode( ActionMode::NONE );
			is_jumping_ = true;
			is_action_pre_finish_ = false;
		}
		else if ( get_location().y() - action_base_position_.y() >= get_balloon_action_length() * 0.75f )
		{
			is_action_pre_finish_ = true;
		}
	}
	else if ( action_mode_ == ActionMode::ROCKET )
	{
		// ロケット
		if ( ( get_location() - action_base_position_ ).length() >= get_rocket_action_length() || action_timer_ >= 2.5f )
		{
			finish_rocketing();
		}
		else if ( ( get_location() - action_base_position_ ).length() >= get_rocket_action_length() * 0.8f )
		{
			is_action_pre_finish_ = true;
		}
	}

	if ( is_flickering() )
	{
		update_velocity_by_flicker( Vector( get_location().x(), action_base_position_.y(), get_location().z() ), get_flicker_scale() );
	}

	if ( ! can_peer_down() )
	{
		eye_depth_ *= 0.9f;
		eye_depth_ = math::clamp( eye_depth_, 0.f, 1.f );
	}

	if ( is_located_on_safe() )
	{
		get_model()->get_line()->set_color( Color( 0.25f, 0.25f, 1.f, 0.f ) );
	}
	else if ( is_located_on_die() )
	{
		get_model()->get_line()->set_color( Color( 1.f, 0.f, 0.f, 0.f ) );

		if ( ! is_dead() && ! is_on_footing() && ! is_on_ladder() && get_rigid_body()->getLinearVelocity().y() < -7.5f && get_location().y() > 10.f && get_location().y() < 30.f &&  get_action_mode() == ActionMode::NONE )
		{
			play_sound( "fall", false, false );
		}
	}
	else
	{
		get_model()->get_line()->set_color( Color( 0.f, 0.f, 0.f, -0.25f ) );
	}

	if ( is_on_footing() )
	{
		if ( is_located_on_die() )
		{
			kill();
		}
		else
		{
			// 通常着地
			if ( get_velocity().y() < -2.f )
			{
				stop_sound( "fall" );
				play_sound( "land", false, false );

				// バウンドしないようにする
				set_velocity( Vector( get_velocity().x(), 0.f, get_velocity().z() ) );
			}

			set_last_footing_height_to_current_height();
		}

		if ( action_mode_ != ActionMode::BALLOON )
		{
			balloon_sequence_count_ = 0;
		}
	}

	if ( ! is_located_on_die() || is_on_ladder() )
	{
		stop_sound( "fall" );
	}

	if ( is_dead() )
	{
		eye_height_ *= 0.95f;
		eye_height_ = std::max( eye_height_, 0.15f );
	}

	uncontrollable_timer_ = math::chase< float_t >( uncontrollable_timer_, 0.f, get_frame_elapsed_time() );

	limit_velocity();

	update_gravity();

	// 当たり判定のためにリセット
	is_on_ladder_ = false;
}

void Player::limit_velocity()
{
	if ( is_uncontrollable() )
	{
		ActiveObject::limit_velocity();

		return;
	}

	Vector v = get_velocity();

	if ( is_rocketing() )
	{
		// v *= 1.01f;
	}
	else if ( is_jumping() )
	{
		v.set_x( v.x() * 0.95f );
		v.set_z( v.z() * 0.95f );
	}
	else
	{
		v.set_x( v.x() * 0.9f );
		v.set_z( v.z() * 0.9f );

		if ( is_on_ladder() )
		{
			v.set_y( v.y() * 0.5f );
		}
	}

	if ( ! is_rocketing() )
	{
		Vector xz_v( v.x(), 0.f, v.z() );

		if ( xz_v.length() > get_max_run_velocity() )
		{
			xz_v /= xz_v.length() / get_max_run_velocity();
			v.set_x( xz_v.x() );
			v.set_z( xz_v.z() );
		}
	}

	set_velocity( v );

	ActiveObject::limit_velocity();
}

/**
 * 現在ジャンプ可能かどうかのフラグを更新する
 *
 */
void Player::update_jumpable()
{
	is_jumpable_ = false;

	if ( action_mode_ == ActionMode::BALLOON || action_mode_ == ActionMode::ROCKET )
	{
		is_jumpable_ = true;
		return;
	}

	// ジャンプ直後と着地直後はジャンプできない
	if ( abs( get_velocity().y() ) > 3.f )
	{
		return;
	}

	is_jumpable_ = is_on_footing() || ( is_on_ladder() && ! is_facing_to_block() );
}

void Player::update_on_footing()
{
	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_rigid_body()->getCenterOfMassPosition().y();
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();
	const float_t ray_length = get_collision_height() * 0.5f + 0.05f;

	const float_t margin = 0.05f;
	const Vector front = get_front() * ( get_collision_depth() * 0.5f - margin );
	const Vector right = get_right() * ( get_collision_width() * 0.5f - margin );
	const Vector center = Vector( x, y, z );

	// 中心 + 四隅 + 四点 の設置を調べる
	if (
		check_on_footing( center, ray_length ) ||

		check_on_footing( center - right - front, ray_length ) ||
		check_on_footing( center + right - front, ray_length ) ||
		check_on_footing( center - right + front, ray_length ) ||
		check_on_footing( center + right + front, ray_length ) ||
		
		check_on_footing( center - right, ray_length ) ||
		check_on_footing( center + right, ray_length ) ||
		check_on_footing( center - front, ray_length ) ||
		check_on_footing( center + front, ray_length ) )
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
	if ( is_jumping_ && get_velocity().y() < 0.f && ( is_on_footing() || is_on_ladder() ) )
	{
		is_jumping_ = false;
	}
}

/**
 * このまま落下すると死亡するかどうかの状態を更新する
 *
 */
void Player::update_located_to_die()
{
	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_rigid_body()->getCenterOfMassPosition().y();
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();

	const float_t margin = 0.05f;
	const Vector front = get_front() * ( get_collision_depth() * 0.5f - margin );
	const Vector right = get_right() * ( get_collision_width() * 0.5f - margin );
	const Vector center = Vector( x, y, z );

	is_located_on_safe_ = false;

	// 中心 + 四隅 + 四点 の設置を調べる
	if (
		get_last_footing_height() - get_footing_height( center, true ) < get_height_to_die() ||

		get_last_footing_height() - get_footing_height( center - right - front, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center + right - front, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center - right + front, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center + right + front, true ) < get_height_to_die() ||
		
		get_last_footing_height() - get_footing_height( center - right, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center + right, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center - front, true ) < get_height_to_die() ||
		get_last_footing_height() - get_footing_height( center + front, true ) < get_height_to_die() )
	{
		is_located_on_die_ = false;
	}
	else
	{
		is_located_on_die_ = true;
	}
}

class WithoutMeContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	GameObject* me_;
	bool is_hit_;

public:
	explicit WithoutMeContactResultCallback( GameObject* me )
		: me_( me )
		, is_hit_( false )
	{

	}

	virtual	btScalar addSingleResult( btManifoldPoint& /* cp */, const btCollisionObjectWrapper* o1,int /* partId0 */, int /* index0 */, const btCollisionObjectWrapper* o2, int /* partId1 */, int /* index1 */ )
	{
		GameObject* go1 = static_cast< GameObject* >( o1->getCollisionObject()->getUserPointer() );
		GameObject* go2 = static_cast< GameObject* >( o2->getCollisionObject()->getUserPointer() );

		if ( go1 == me_ || go2 == me_ )
		{
			return 1.f;
		}

		if ( go1 && ! go1->is_block() )
		{
			return 1.f;
		}

		if ( go2 && ! go2->is_block() )
		{
			return 1.f;
		}

		is_hit_ = true;

		btQuaternion q1 = o1->getWorldTransform().getRotation();

		return 1.f;
	}

	bool is_hit() const { return is_hit_; }
	void clear() { is_hit_ = false; }
};

/**
 * 障害物に直面しているかのフラグを更新する
 *
 * facing_to_block_ は 
 * 
 */
void Player::update_facing_to_block()
{
	is_facing_to_block_ = false;

	btVector3 half( 0.05f, 0.05f, 0.05f );
	btBoxShape shape( half );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0.f, is_clambering() ? -0.1f : shape.getHalfExtentsWithMargin().y() + 0.25f, get_collision_depth() * 0.5f ) );
	
	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( get_transform() ) * offset );

	WithoutMeContactResultCallback callback( this );
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->contactTest( & collision_object, callback );

	is_facing_to_block_ = callback.is_hit();


	// test
	// get_physics_manager()->contact_test( )
}

/**
 * よじ登りが可能かのフラグを更新する
 *
 */
void Player::update_can_clamber()
{
	can_clamber_ = false;

	if ( ! is_facing_to_block() )
	{
		return;
	}

	if ( get_last_footing_height() - get_location().y() > 5.f )
	{
		return;
	}

	btVector3 half( get_collision_width() * 0.5f, get_collision_height() * 0.5f, get_collision_depth() * 0.5f );
	btBoxShape shape( half );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0.f, get_collision_height() * 0.5f + 0.1f, get_collision_depth() * 0.5f ) );
	
	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( get_transform() ) * offset );

	WithoutMeContactResultCallback callback_low( this );
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->contactTest( & collision_object, callback_low );

	if ( ! callback_low.is_hit() )
	{
		can_clamber_ = true;
		return;
	}

	offset.setOrigin( btVector3( 0.f, get_collision_height() * 0.5f + 1.5f, get_collision_depth() ) );
	collision_object.setWorldTransform( btTransform( get_transform() ) * offset );

	WithoutMeContactResultCallback callback_high( this );
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->contactTest( & collision_object, callback_high );

	can_clamber_ = ! callback_high.is_hit();
}

/**
 * のぞき込みが可能かのフラグを更新する
 *
 */
void Player::update_can_peer_down()
{
	if ( ! is_on_footing() )
	{
		can_peer_down_ = false;
		return;
	}

	btVector3 half( 0.25f, 0.25f, 0.25f );
	btBoxShape shape( half );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0.f, get_eye_height(), get_collision_depth() * 0.5f + get_eye_depth() ) );
	
	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( get_transform() ) * offset );

	WithoutMeContactResultCallback callback( this );
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->contactTest( & collision_object, callback );

	can_peer_down_ = ! callback.is_hit();
}

/**
 * 石投げが可能かのフラグを更新する
 *
 */
void Player::update_can_throw()
{
	btVector3 half( 0.25f, 0.25f, 0.25f );
	btBoxShape shape( half );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( 0.f, get_eye_height(), get_collision_depth() ) );
	
	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( get_transform() ) * offset );

	WithoutMeContactResultCallback callback( this );
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->contactTest( & collision_object, callback );

	can_throw_ = ! callback.is_hit();
}

/**
 * 移動速度を更新する
 *
 */
void Player::update_step_speed()
{
	if ( action_mode_ == ActionMode::BALLOON )
	{
		step_speed_ = math::chase( step_speed_, get_max_walk_step_speed(), 0.01f );
	}
	else if ( step_count_ <= 0 )
	{
		// stop
		step_speed_ *= 0.5f;
	}
	else if ( step_count_ <= 150 || ! can_running() )
	{
		// slow walk
		step_speed_ = math::chase( step_speed_, get_max_walk_step_speed(), 0.01f );
	}
	else
	{
		// run
		step_speed_ = math::chase( step_speed_, get_max_run_step_speed(), 0.001f );
	}

	if ( ! is_on_footing() )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
	else if ( is_running() )
	{
		stop_sound( "walk" );
		play_sound( "run", true, false );
	}
	else if ( is_walking() )
	{
		stop_sound( "run" );
		play_sound( "walk", true, false );
	}
	else
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}

	if ( get_step_speed() >= get_max_run_step_speed() && ! is_jumping() && ! is_falling() )
	{
		play_sound( "short-breath", true, false );
	}
	else
	{
		fade_out_sound( "short-breath" );
	}
}

/**
 * 走ることができるかを調べる
 *
 */
bool Player::can_running() const
{
	return get_velocity().length() > 1.f && item_count_[ static_cast< int >( ItemType::STONE ) ] < 3;
}

/**
 * 重力を更新する
 *
 */
void Player::update_gravity()
{
	if ( action_mode_ == ActionMode::BALLOON || action_mode_ == ActionMode::ROCKET || ( is_on_ladder() && ! is_jumping() ) )
	{
		get_rigid_body()->setGravity( Vector::Zero );
	}
	else
	{
		get_rigid_body()->setGravity( get_default_gravity() );
	}
}

/**
 * 
 *
 * @param from レイを飛ばし始める原点
 * @param ray_length レイの長さ
 */
bool Player::check_on_footing( const Vector& from, float_t ray_length, bool include_soft_footing ) const
{
	Vector to = from + Vector( 0, -ray_length, 0 );
	
	ClosestNotMe ray_callback( from, to, get_rigid_body(), include_soft_footing );
	ray_callback.m_closestHitFraction = 1.0;

	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->rayTest( from, to, ray_callback );

	return ray_callback.hasHit();
}

/**
 * 指定した座標から地面までの間で、一番高い足場の高さを取得する
 *
 * @param Vector3 from 
 * @param bool include_soft_footing 風船などのやわらかいものを足場に含むフラグ
 */
float_t Player::get_footing_height( const Vector& from, bool include_soft_footing ) const
{
	Vector to = from;
	to.set_y( 0.f );

	ClosestNotMe ray_callback( from, to, get_rigid_body(), include_soft_footing );
	ray_callback.m_closestHitFraction = 1.0;

	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->rayTest( from, to, ray_callback );

	if ( ray_callback.m_collisionObject && ray_callback.m_collisionObject->getUserPointer() )
	{
		const ActiveObject* a = reinterpret_cast< const ActiveObject* >( ray_callback.m_collisionObject->getUserPointer() );
		
		/// debug !!!
		// const char* xxx = typeid( a ).name();

		if ( a->is_safe_footing() )
		{
			// !!!
			const_cast< Player* >( this )->is_located_on_safe_ = true;
		}
	}

	return ( 1.f - ray_callback.m_closestHitFraction ) * from.y();
}

/**
 * 現在梯子の上り下り以外の移動を禁止するか
 *
 */
bool Player::is_ladder_step_only() const
{
	return is_on_ladder() && ! is_on_footing();
}

void Player::step( float_t s )
{
	if ( is_uncontrollable() || is_rocketing() )
	{
		return;
	}

	step_count_++;
	update_step_speed();

	set_velocity(
		get_velocity() +
		Vector(
			( get_front() * s ).x() * get_step_speed(),
			0.f,
			( get_front() * s ).z() * get_step_speed()
		)
	);
}

void Player::side_step( float_t s )
{
	if ( is_uncontrollable() || is_rocketing() )
	{
		return;
	}

	set_velocity(
		get_velocity() +
		Vector(
			( get_right() * s ).x() * get_side_step_speed(),
			0.f,
			( get_right() * s ).z() * get_side_step_speed()
		)
	);
}

/**
 * 
 *
 */
void Player::ladder_step( float_t s )
{
	if ( is_uncontrollable() || is_jumping() )
	{
		return;
	}

	Vector v = get_velocity() + Vector( 0.f, pitch_ * s, 0.f );
	v.set_y( math::clamp( v.y(), -get_ladder_step_speed(), get_ladder_step_speed() ) );

	set_velocity( v );

	play_sound( "ladder-step", true, false );
}

void Player::release_ladder()
{
	ladder_ = nullptr;
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

	if ( is_on_ladder() )
	{
		set_velocity( get_velocity() + Vector( 0.f, 4.f, 0.f ) + get_front() * 4.f );
	}
	else
	{
		set_velocity( get_velocity() + Vector( 0.f, 4.f, 0.f ) );
	}

	is_jumping_ = true;

	set_action_mode( ActionMode::NONE );

	stop_sound( "short-breath" );

	if ( is_running() )
	{
		play_sound( "short-breath-jump" );
	}
	else
	{
		play_sound( "jump" );
	}
}

void Player::super_jump()
{
	set_velocity( Vector( get_velocity().x(), get_velocity().y() + 20.f, get_velocity().z() ) );
}

/**
 * よじ登る
 *
 */
void Player::clamber()
{
	if ( can_clamber() )
	{
		set_velocity( Vector( 0.f, 2.f, 0.f ) + get_front() * 2.f );

		if ( ! is_clambering() )
		{
			is_clambering_ = true;

			play_sound( "clamber", false, false );
		}
	}
	else
	{
		is_clambering_ = false;
	}
}

/**
 * よじ登りの中止
 *
 */
void Player::stop_clamber()
{
	is_clambering_ = false;
}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	set_direction_degree( get_direction_degree() + d );
}

/**
 *
 */
void Player::set_direction_degree( float d, bool force )
{
	if ( ! force )
	{
		if ( is_rocketing() )
		{
			return;
		}
	}

	ActiveObject::set_direction_degree( d );
}

void Player::stop()
{
	step_count_ = 0;
	update_step_speed();
}

void Player::stop_ladder_step()
{
	stop_sound( "ladder-step" );
}

void Player::rocket( const Vector& direction )
{
	if ( get_item_count( ItemType::ROCKET ) <= 0 )
	{
		return;
	}

	is_jumping_ = true;
	action_timer_ = 0.f;

	set_action_mode( ActionMode::ROCKET );
	set_action_base_position_to_current_position();

	set_velocity( direction * get_rocket_initial_velocity() );

	item_count_[ static_cast< int >( ItemType::ROCKET ) ]--;

	if ( item_count_[ static_cast< int >( ItemType::ROCKET ) ] <= 0 )
	{
		selected_item_type_ = ItemType::NONE;
	}

	play_sound( "rocket" );
}

void Player::finish_rocketing()
{
	stop_sound( "rocket" );
	play_sound( "rocket-burst" );
	set_action_mode( ActionMode::NONE );
	is_jumping_ = true;
	is_action_pre_finish_ = false;
}

void Player::throw_stone( const Vector& direction )
{
	if ( get_item_count( ItemType::STONE ) <= 0 )
	{
		return;
	}

	if ( ! can_throw() )
	{
		return;
	}
	
	Stone* stone = stone_list_.back();
	stone_list_.pop_back();
	item_count_[ static_cast< int >( ItemType::STONE ) ]--;

	if ( item_count_[ static_cast< int >( ItemType::STONE ) ] <= 0 )
	{
		selected_item_type_ = ItemType::NONE;
	}

	stone->restart();
	stone->set_location( get_location() + ( get_front() * get_collision_depth() ) + Vector( 0.f, get_eye_height(), 0.f ) );
	stone->set_velocity( direction * 8.f );

	play_sound( "stone-throw" );
}

void Player::damage( const Vector& to )
{
	uncontrollable_timer_ = 1.5f;

	set_velocity( to );

	is_jumping_ = true;

	play_sound( "damage-1" );
}

void Player::kill()
{
	if ( is_dead() )
	{
		return;
	}

	ActiveObject::kill();

	stop_sound( "fall" );
	stop_sound( "walk" );
	stop_sound( "run" );

	play_sound( "dead" );

	get_rigid_body()->setActivationState( true );
	set_angular_factor( 1.f );
	set_friction( 1.f );
	// get_rigid_body()->applyForce( get_front() * 2000.f, Vector3( 0.1f, 1.5f, 0.f ) );
}

void Player::set_eye_depth( float d )
{
	if ( ! is_on_footing() )
	{
		return;
	}

	if ( get_location().y() < 5.f )
	{
		return;
	}

	eye_depth_ = math::clamp( d, 0.f, 1.f );
}

void Player::add_eye_depth( float d )
{
	if ( d > 0.f && ! can_peer_down() )
	{
		return;
	}

	set_eye_depth( eye_depth_ + d );
}

bool Player::is_falling() const
{
	return get_rigid_body()->getLinearVelocity().y() < -1.f;
}

void Player::on_collide_with( Balloon* balloon )
{
	if ( is_flickering_ )
	{
		return;
	}

	if ( balloon->get_player() )
	{
		return;
	}

	balloon->set_player( this );
	this->balloon_ = balloon;

	is_jumping_ = false;

	/*
	is_falling_ = false;
	is_action_pre_finish_ = false;
	*/

	set_action_mode( ActionMode::BALLOON );
	set_action_base_position_to_current_position();

	set_last_footing_height_to_current_height();

	stop_flickering();

	balloon_sequence_count_ += 1;
	balloon_sound_request_ = balloon_sequence_count_;
}

void Player::on_collide_with( Rocket* /* rocket */ )
{
	item_count_[ static_cast< int >( ItemType::ROCKET ) ]++;
	selected_item_type_ = ItemType::ROCKET;

	play_sound( "rocket-get" );
}

void Player::on_collide_with( Stone* stone )
{
	item_count_[ static_cast< int >( ItemType::STONE ) ]++;
	selected_item_type_ = ItemType::STONE;

	stone_list_.push_back( stone );

	play_sound( "stone-get" );
}

void Player::on_collide_with( Medal* medal )
{
	has_medal_ = true;
	
	medal->kill();

	play_sound( "medal-get" );
}

void Player::on_collide_with( Robot* robot )
{
	/*
	if ( is_uncontrollable() )
	{
		if ( uncontrollable_timer_ < 1.f )
		{
			kill();

			return;
		}
	}
	*/

	if ( robot->get_mode() != Robot::Mode::CHASE )
	{
		return;
	}

	if ( ! is_uncontrollable() )
	{
		hp_--;
	}

	if ( hp_ <= 0 )
	{
		kill();

		return;
	}

	Vector v = -get_front() * 10.f;
	v.set_y( 2.5f );

	damage( v );
}

void Player::on_collide_with( Ladder* l )
{
	is_on_ladder_ = true;
	ladder_ = l;

	if ( is_rocketing() )
	{
		finish_rocketing();
	}

	set_last_footing_height_to_current_height();
}

void Player::on_collide_with( StaticObject* )
{
	if ( is_rocketing() )
	{
		finish_rocketing();
	}
}

void Player::on_collide_with( DynamicObject* )
{
	if ( is_rocketing() )
	{
		finish_rocketing();
	}
}

void Player::set_action_mode( ActionMode action_mode )
{
	if ( action_mode == action_mode_ )
	{
		return;
	}

	/*
	// 自動傘選択
	if ( action_mode_ != ActionMode::UMBRELLA && action_mode_ != ActionMode::NONE )
	{
		if ( action_mode == ActionMode::NONE && item_count_[ ItemType::UMBRELLA ] && floor_cell() && is_if_fall_to_die( floor_cell()->height() ) )
		{
			action_mode = ActionMode::UMBRELLA;
		}
	}
	*/

	action_mode_ = action_mode;
	action_timer_ = 0.f;

	stop_sound( "fall" );
}

int Player::get_item_count( ItemType item_type ) const
{
	if ( item_type <= ItemType::NONE ) return 0;
	if ( item_type >= ItemType::MAX ) return 0;

	return item_count_[ static_cast< int >( item_type ) ];
}

void Player::select_prev_item()
{
	for ( int type = static_cast< int >( selected_item_type_ ) - 1; type >= static_cast< int >( ItemType::NONE ); type-- )
	{
		if ( type == static_cast< int >( ItemType::NONE ) || item_count_[ type ] )
		{
			selected_item_type_ = static_cast< ItemType >( type );
			play_sound( "click" );
			break;
		}
	}
}

void Player::select_next_item()
{
	for ( int type = static_cast< int >( selected_item_type_ ) + 1; type < static_cast< int >( ItemType::MAX ); type++ )
	{
		if ( item_count_[ type ] )
		{
			selected_item_type_ = static_cast< ItemType >( type );
			play_sound( "click" );
			break;
		}
	}
}

/**
 * 望遠鏡モードを切り替える
 *
 */
void Player::switch_scope_mode()
{
	if ( action_mode_ == ActionMode::SCOPE )
	{
		set_action_mode( ActionMode::NONE );
		selected_item_type_ = ItemType::NONE;
	}
	else
	{
		action_mode_ = ActionMode::SCOPE;
	}
}

} // namespace blue_sky

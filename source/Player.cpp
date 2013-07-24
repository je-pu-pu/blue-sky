#include "Player.h"

#include "Balloon.h"
#include "Medal.h"
#include "Robot.h"

#include "DrawingModel.h"
#include "DrawingLine.h"

#include "GameMain.h"
#include "Input.h"

#include "ActiveObjectPhysics.h"

#include <common/math.h>
#include <common/serialize.h>

namespace blue_sky
{

Player::Player()
	: is_on_footing_( false )
	, is_jumping_( false )
	, is_jumpable_( false )
	, is_falling_to_die_( false )
	, is_falling_to_balloon_( false )
	, step_count_( 0 )
	, step_speed_( 0.25f )
	, action_mode_( ACTION_MODE_NONE )
	, is_action_pre_finish_( false )
	, balloon_sequence_count_( 0 )
	, uncontrollable_timer_( 0.f )
	, eye_height_( 1.5f )
	, eye_depth_( 0.f )
	, has_medal_( false )
	, last_footing_height_( 0.f )
	, balloon_( 0 )
	, hp_( 1 )
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
	is_falling_to_die_ = false;
	is_falling_to_balloon_ = false;

	step_count_ = 0;
	step_speed_ = 0.25f;

	eye_height_ = 1.5f;
	eye_depth_ = 0.f;

	set_action_mode( ACTION_MODE_NONE );
	has_medal_ = false;

	balloon_ = 0;

	hp_ = 1;

	set_last_footing_height_to_current_height();
	set_mass( 50.f );
}

/**
 * 更新
 *
 */
void Player::update()
{
	get_rigid_body()->setActivationState( true );

	limit_velocity();

	update_on_footing();
	update_jumpable();
	update_jumping();
	update_falling_to_die();

	if ( action_mode_ == ACTION_MODE_BALLOON )
	{
		set_velocity( Vector3( get_velocity().x(), math::chase( get_velocity().y(), 3.f, 0.5f ), get_velocity().z() ) );

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

	if ( is_falling_to_balloon() )
	{
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.25f, 0.25f, 1.f, 0.f ) );
	}
	else if ( is_falling_to_die() )
	{
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 1.f, 0.f, 0.f, 0.f ) );

		if ( ! is_dead() && ! is_on_footing() && get_rigid_body()->getLinearVelocity().y() < -7.5f && get_location().y() > 10.f && get_location().y() < 30.f &&  get_action_mode() == ACTION_MODE_NONE )
		{
			play_sound( "fall", false, false );
		}
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
			// 通常着地
			if ( get_velocity().y() < -1.f )
			{
				stop_sound( "fall" );
				play_sound( "land", false, false );

				// バウンドしないようにする
				set_velocity( Vector3( get_velocity().x(), 0.f, get_velocity().z() ) );
			}

			set_last_footing_height_to_current_height();
		}

		if ( action_mode_ != ACTION_MODE_BALLOON )
		{
			balloon_sequence_count_ = 0;
		}
	}

	if ( ! is_falling_to_die() )
	{
		stop_sound( "fall" );
	}

	if ( is_dead() )
	{
		eye_height_ *= 0.95f;
		eye_height_ = std::max( eye_height_, 0.15f );
	}

	uncontrollable_timer_ = math::chase< float_t >( uncontrollable_timer_, 0.f, get_frame_elapsed_time() );
}

void Player::update_transform()
{
	ActiveObject::update_transform();

	/*
	if ( get_location().y() < 0.f )
	{
		set_location( Vector3( get_location().x(), 0.f, get_location().z() ) );
		is_on_footing_ = true;
	}
	*/
}

void Player::limit_velocity()
{
	ActiveObject::limit_velocity();

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

	set_velocity( v );
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

	is_jumpable_ = is_on_footing() || action_mode_ == ACTION_MODE_BALLOON;
}

void Player::update_on_footing()
{
	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_rigid_body()->getCenterOfMassPosition().y();
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();
	const float_t ray_length = get_collision_height() * 0.5f + 0.05f;

	const float_t margin = 0.05f;
	const Vector3 front = get_front() * ( get_collision_depth() * 0.5f - margin );
	const Vector3 right = get_right() * ( get_collision_width() * 0.5f - margin );
	const Vector3 center = Vector3( x, y, z );

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
	if ( is_jumping_ && is_jumpable() )
	{
		is_jumping_ = false;
	}
}

/**
 * このまま落下すると死亡するかどうかの状態を更新する
 *
 */
void Player::update_falling_to_die()
{
	const float_t x = get_rigid_body()->getCenterOfMassPosition().x();
	const float_t y = get_rigid_body()->getCenterOfMassPosition().y();
	const float_t z = get_rigid_body()->getCenterOfMassPosition().z();

	const float_t margin = 0.05f;
	const Vector3 front = get_front() * ( get_collision_depth() * 0.5f - margin );
	const Vector3 right = get_right() * ( get_collision_width() * 0.5f - margin );
	const Vector3 center = Vector3( x, y, z );

	is_falling_to_balloon_ = false;

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
		is_falling_to_die_ = false;
	}
	else
	{
		is_falling_to_die_ = true;
	}
}

/**
 * 移動速度を更新する
 *
 */
void Player::update_step_speed()
{
	if ( action_mode_ == ACTION_MODE_BALLOON )
	{
		step_speed_ = math::chase( step_speed_, get_max_walk_step_speed(), 0.01f );
	}
	else if ( step_count_ <= 0 )
	{
		// stop
		step_speed_ *= 0.5f;
	}
	else if ( step_count_ <= 150 )
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
 *
 *
 *
 */
bool Player::check_on_footing( const Vector3& from, float_t ray_length, bool include_soft_footing ) const
{
	Vector3 to = from + Vector3( 0, -ray_length, 0 );
	
	ClosestNotMe ray_callback( from, to, get_rigid_body(), include_soft_footing );
	ray_callback.m_closestHitFraction = 1.0;

	get_dynamics_world()->rayTest( from, to, ray_callback );

	return ray_callback.hasHit();
}

/**
 * 指定した座標から地面までの間で、一番高い足場の高さを取得する
 *
 * @param Vector3 from 
 * @param bool include_soft_footing 風船などのやわらかいものを足場に含むフラグ
 */
float_t Player::get_footing_height( const Vector3& from, bool include_soft_footing ) const
{
	Vector3 to = from;
	to.setY( 0.f );

	ClosestNotMe ray_callback( from, to, get_rigid_body(), include_soft_footing );
	ray_callback.m_closestHitFraction = 1.0;

	get_dynamics_world()->rayTest( from, to, ray_callback );

	if ( ray_callback.m_collisionObject && ray_callback.m_collisionObject->getUserPointer() )
	{
		const ActiveObject* a = reinterpret_cast< const ActiveObject* >( ray_callback.m_collisionObject->getUserPointer() );
		
		/// debug !!!
		// const char* xxx = typeid( a ).name();

		if ( a->is_balloon() )
		{
			// !!!
			const_cast< Player* >( this )->is_falling_to_balloon_ = true;
		}
	}

	return ( 1.f - ray_callback.m_closestHitFraction ) * from.y();
}

void Player::step( float_t s )
{
	if ( is_uncontrollable() )
	{
		return;
	}

	step_count_++;
	update_step_speed();

	set_velocity( get_velocity() + Vector3( ( get_front() * s ).x() * get_step_speed(), 0, ( get_front() * s ).z() * get_step_speed() ) );
}

void Player::side_step( float_t s )
{
	if ( is_uncontrollable() )
	{
		return;
	}

	set_velocity(
		get_velocity() +
		Vector3(
			( get_right() * s ).x() * get_side_step_speed(),
			0,
			( get_right() * s ).z() * get_side_step_speed()
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

	set_velocity( Vector3( get_velocity().x(), get_velocity().y() + 4.f, get_velocity().z() ) );

	is_jumping_ = true;

	set_action_mode( ACTION_MODE_NONE );

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
	set_velocity( Vector3( get_velocity().x(), get_velocity().y() + 20.f, get_velocity().z() ) );
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
}

void Player::stop()
{
	step_count_ = 0;
	update_step_speed();
}

void Player::damage( const Vector3& to )
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
	get_rigid_body()->setAngularFactor( 1.f );
	get_rigid_body()->setFriction( 1.f );
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
	set_eye_depth( eye_depth_ + d );
}

bool Player::is_falling() const
{
	return get_rigid_body()->getLinearVelocity().y() < -1.f;
}

void Player::on_collide_with( Balloon* balloon )
{
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

	set_action_mode( ACTION_MODE_BALLOON );
	set_action_base_position_to_current_position();

	set_last_footing_height_to_current_height();

	// play_sound( "balloon-get" );

	balloon_sequence_count_ += 1;
	balloon_sequence_count_ = math::clamp( balloon_sequence_count_, 1, 7 );

	for ( int n = 1; n <= 7; n++ )
	{
		if ( n != balloon_sequence_count_ )
		{
			stop_sound( ( std::string( "balloon-" ) + common::serialize( n ) ).c_str() );
		}
	}

	play_sound( ( std::string( "balloon-" ) + common::serialize( balloon_sequence_count_ ) ).c_str() );
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

	if ( robot->get_mode() != Robot::MODE_CHASE )
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

	Vector3 v = -get_front() * 10.f;
	v.setY( 2.5f );

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

	stop_sound( "fall" );
}

} // namespace blue_sky

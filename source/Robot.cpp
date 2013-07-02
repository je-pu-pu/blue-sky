#include "Robot.h"
#include "Player.h"

#include "AnimationPlayer.h"

#include "DrawingModel.h"
#include "DrawingLine.h"

#include "ActiveObjectPhysics.h"

#include <common/math.h>

namespace blue_sky
{

Robot::Robot()
	: player_( 0 )
	, mode_( MODE_STAND )
	, timer_( 0 )
{
	
}

void Robot::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setAngularFactor( 0 );
	get_rigid_body()->setFriction( 0 );

	timer_ = 0;
}

void Robot::update()
{
	timer_ += get_elapsed_time();

	get_rigid_body()->setActivationState( true );

	Vector3 relative_position = player_->get_location() - get_location();
	relative_position.setY( 0 );

	if ( relative_position.length() < 7.5f )
	{
		// mode_ = MODE_CHASE;
	}
	else 
	{
		mode_ = MODE_STAND;
	}

	set_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ) );

	get_front() = relative_position;
	get_front().normalize();

	if ( mode_ == MODE_CHASE )
	{
		set_velocity( Vector3( get_front().x() * 2.f, get_velocity().y(), get_front().z() * 2.f ) );
		set_drawing_model( drawing_model_list_[ static_cast< int >( timer_ * 3 ) % drawing_model_list_.size() ] );

		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.8f, 0, 0, -0.25f ) );
		get_animation_player()->play( "Walk", false, true );
	}
	else
	{
		set_velocity( Vector3( 0.f, 0.f, 0.f ) );
		set_drawing_model( drawing_model_list_.front() );
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0, 0, 0, 0 ) );
		// get_animation_player()->play( "Test", false, true );
		get_animation_player()->play( "Stand", false, true );
		
		if ( relative_position.length() < 7.5f )
		{
			Vector3 from(
				get_rigid_body()->getCenterOfMassPosition().x(),
				get_rigid_body()->getCenterOfMassPosition().y() + 1.5f, // 目線
				get_rigid_body()->getCenterOfMassPosition().z() );

			Vector3 to(
				player_->get_rigid_body()->getCenterOfMassPosition().x(),
				player_->get_rigid_body()->getCenterOfMassPosition().y() + 1.5f, // プレイヤーの目線
				player_->get_rigid_body()->getCenterOfMassPosition().z() );

			ClosestNotMeAndHim ray_callback( from, to, get_rigid_body(), player_->get_rigid_body(), false );
			ray_callback.m_closestHitFraction = 1.0;

			get_dynamics_world()->rayTest( from, to, ray_callback );
		
			// プレイヤーとの間に障害物がなければ
			if ( ! ray_callback.hasHit() )
			{
				mode_ = MODE_CHASE;
			}
		}
	}
}

/// @todo 削除
void Robot::add_drawing_model( const DrawingModel* m )
{
	drawing_model_list_.push_back( m );
	set_drawing_model( drawing_model_list_.front() );
}

} // namespace blue_sky

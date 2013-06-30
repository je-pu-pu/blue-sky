#include "Robot.h"
#include "Player.h"

#include "DrawingModel.h"
#include "DrawingLine.h"

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
		mode_ = MODE_CHASE;
	}
	else 
	{
		mode_ = MODE_STAND;
	}

	/*
	set_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ) );

	get_front() = relative_position;
	get_front().normalize();
	*/

	if ( mode_ == MODE_CHASE )
	{
		// set_velocity( Vector3( get_front().x() * 2.f, get_velocity().y(), get_front().z() * 2.f ) );
		set_drawing_model( drawing_model_list_[ static_cast< int >( timer_ * 3 ) % drawing_model_list_.size() ] );

		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.8f, 0, 0, -0.25f ) );
	}
	else
	{
		set_drawing_model( drawing_model_list_.front() );
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0, 0, 0, 0 ) );
	}
}

/// @todo �폜
void Robot::add_drawing_model( const DrawingModel* m )
{
	drawing_model_list_.push_back( m );
	set_drawing_model( drawing_model_list_.front() );
}

} // namespace blue_sky

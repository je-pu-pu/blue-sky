#include "Robot.h"
#include "Player.h"

#include "AnimationPlayer.h"

#include "DrawingModel.h"
#include "DrawingLine.h"

#include "ActiveObjectPhysics.h"

#include <common/math.h>

#include <iostream>

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

	if ( mode_ == MODE_CHASE )
	{
		// �^�[�Q�b�g�̕�������
		Vector3 relative_position = player_->get_location() - get_location();
		relative_position.setY( 0 );

		set_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ) );

		get_front() = relative_position;
		get_front().normalize();

		set_velocity( Vector3( get_front().x() * 2.f, get_velocity().y(), get_front().z() * 2.f ) );
		set_drawing_model( drawing_model_list_[ static_cast< int >( timer_ * 3 ) % drawing_model_list_.size() ] );

		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.8f, 0, 0, -0.25f ) );
		get_animation_player()->play( "Walk", false, true );

		if ( caluclate_target_lost() )
		{
			mode_ = MODE_STAND;
		}
	}
	else
	{
		set_velocity( Vector3( 0.f, 0.f, 0.f ) );
		set_drawing_model( drawing_model_list_.front() );
		get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0, 0, 0, 0 ) );
		// get_animation_player()->play( "Test", false, true );
		get_animation_player()->play( "Stand", false, true );
		
		if ( caluclate_target_visible() )
		{
			mode_ = MODE_CHASE;
		}
	}
}

/**
 * �^�[�Q�b�g ( �v���C���[ ) ���������������v�Z����
 *
 * @param �^�[�Q�b�g�����������ꍇ�́Atrue ���A�^�[�Q�b�g��⑫���̏ꍇ�� false ��Ԃ�
 */
bool Robot::caluclate_target_lost() const
{
	Vector3 relative_position = player_->get_location() - get_location();
	relative_position.setY( 0 );

	Scalar relative_length = relative_position.length(); // �^�[�Q�b�g�Ƃ̋���

	/// @todo �����Ǝ���
	if ( relative_length > 20.f )
	{
		return true;
	}

	return false;
}

/**
 * �^�[�Q�b�g ( �v���C���[ ) ��ڎ��ł��邩���v�Z����
 *
 * @param �^�[�Q�b�g ( �v���C���[ ) ��ڎ��ł���ꍇ�́Atrue ���A�ڎ��ł��Ȃ��ꍇ�� false ��Ԃ�
 */
bool Robot::caluclate_target_visible() const
{
	Vector3 relative_position = player_->get_location() - get_location();

	Scalar relative_length = relative_position.length(); // �^�[�Q�b�g�Ƃ̋���
	
	/// ����̋���
	const float_t eyeshot_length_short = 3.f;
	const float_t eyeshot_length_middle = 10.f;
	const float_t eyeshot_length_long = 20.f;

	/// ����̋����ɉ���������p ( -1.f .. 1.f ) ( 1.f : ���� / 0.f : �^�� / -1.f : �^���  ) 
	const float_t eyeshot_angle_short = -0.25f;
	const float_t eyeshot_angle_middle = 0.5f;
	const float_t eyeshot_angle_long = 0.75f;

	if ( relative_length > eyeshot_length_long )
	{
		return false;
	}

	// ���{�b�g�̖ڂ̈ʒu
	Vector3 from = get_rigid_body()->getCenterOfMassPosition();
	from.setY( from.y() + 1.5f ); // �ڐ��̍�����ǉ�

	// �^�[�Q�b�g�̖ڂ̈ʒu
	Vector3 to = player_->get_rigid_body()->getCenterOfMassPosition();
	to.setY( to.y() + 1.5f ); // �ڐ��̍�����ǉ�
	
	float_t eyeshot_angle = 0.f;

	if ( relative_length > eyeshot_length_middle )
	{
		eyeshot_angle = eyeshot_angle_long;
	}
	else if ( relative_length > eyeshot_length_short )
	{
		eyeshot_angle = eyeshot_angle_middle;
	}
	else
	{
		eyeshot_angle = eyeshot_angle_short;
	}
	
	std::cout << relative_length << " : " << eyeshot_angle << " : " << get_front().dot( ( to - from ).normalize() ) << std::endl;

	// �^�[�Q�b�g�Ƃ̋����ɉ���������p�ɓ����Ă��Ȃ���΁A�ڎ��ł��Ȃ�
	if ( get_front().dot( ( to - from ).normalize() ) < eyeshot_angle )
	{
		return false;
	}

	// ��Q������
	ClosestNotMeAndHim ray_callback( from, to, get_rigid_body(), player_->get_rigid_body(), false );
	ray_callback.m_closestHitFraction = 1.0;
	
	get_dynamics_world()->rayTest( from, to, ray_callback );
	
	// �^�[�Q�b�g�Ƃ̊Ԃɏ�Q�����Ȃ���Ζڎ��ł��Ă���
	if ( ! ray_callback.hasHit() )
	{
		return true;
	}

	return false;
}

/// @todo �폜
void Robot::add_drawing_model( const DrawingModel* m )
{
	drawing_model_list_.push_back( m );
	set_drawing_model( drawing_model_list_.front() );
}

} // namespace blue_sky

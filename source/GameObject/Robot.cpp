#include "Robot.h"
#include "Player.h"
#include "Stone.h"

#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/GameMain.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/Texture.h>

#include <common/math.h>

#include <iostream>

namespace blue_sky
{

Robot::Robot()
	: player_( 0 )
	, texture_( 0 )
	, mode_( Mode::STAND )
	, mode_backup_( Mode::STAND )
	, timer_( 0 )
{
	texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );

	GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
	GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-error" );

	// この時点ではまだ get_model() は nullptr を返す
	// get_model()->set_shader_at( 0, GameMain::get_instance()->get_graphics_manager()->clone_shader( get_model()->get_shader_at( 0 ) ) );
}

void Robot::restart()
{
	ActiveObject::restart();

	set_angular_factor( 0.f );
	set_friction( 0.f );
	set_mass( 50.f );

	mode_ = Mode::STAND;
	timer_ = 0;

	texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );
	play_animation( "Stand", false, true );

	patrol_point_list_.clear();
	current_patrol_point_index_ = 0;

	add_patrol_point( get_location() );
	add_patrol_point( get_location() + ( get_front() * 5.f ) );
}

void Robot::update()
{
	get_rigid_body()->setActivationState( true );

	if ( mode_ == Mode::CHASE )
	{
		// ターゲットの方を向く ( 表示上の向き )
		chase_direction_to( player_->get_location(), 2.f );
		
		// ターゲットとの距離が遠い場合は、ターゲットの進行方向を予測して進む
		Vector relative_position = player_->get_location() - get_location();
		relative_position.set_y( 0 );

		Vector player_xz_velocity = player_->get_velocity();
		player_xz_velocity.set_y( 0 );

		if ( relative_position.length() > 3.f && player_xz_velocity.length() > 1.f )
		{
			relative_position = ( player_->get_location() + player_->get_front() * 3.f ) - get_location();
			relative_position.set_y( 0 );
		}

		// 進行方向を設定
		Vector forward = relative_position;
		forward.normalize();

		set_velocity( Vector( forward.x() * 3.f, get_velocity().y(), forward.z() * 3.f ) );
		// get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.8f, 0, 0, -0.25f ) );
		play_animation( "Walk", false, true );

		if ( is_visible_in_blink( 2.f ) )
		{
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-error" );
		}
		else
		{
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
		}

		timer_ += get_frame_elapsed_time();

		if ( timer_ >= 10.f )
		{
			if ( ! caluclate_target_visible() )
			{
				mode_ = Mode::ROTATION;
				timer_ = 0.f;
				play_sound( "robot-shutdown" );
				stop_sound( "robot-chase" );

				texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );
			}
		}
		else
		{
			play_sound( "robot-chase", false, false );
		}
	}
	else if ( mode_ == Mode::STAND || mode_ == Mode::ROTATION )
	{
		set_velocity( Vector( 0.f, 0.f, 0.f ) );

		if ( mode_ == Mode::ROTATION )
		{
			set_direction_degree( get_direction_degree() + 0.25f );
		}

		play_animation( "Stand", false, true );
		
		if ( caluclate_target_visible() )
		{
			mode_backup_ = mode_;
			mode_ = Mode::ATTENTION;
			timer_ = 0.f;

			play_sound( "robot-found", false, false );
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
		}
	}
	else if ( mode_ == Mode::PATROL )
	{
		update_patrol();
	}
	else if ( mode_ == Mode::ATTENTION )
	{
		set_velocity( Vector( 0.f, 0.f, 0.f ) );

		// ターゲットの方を向く
		Vector relative_position = player_->get_location() - get_location();
		relative_position.set_y( 0 );

		// 表示上の向きを設定
		float_t target_direction_degree = math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) );
		chase_direction_degree( target_direction_degree, 2.f );

		play_animation( "Attention", false, true );

		timer_ += get_frame_elapsed_time();

		if ( timer_ >= 3.f )
		{
			if ( caluclate_target_visible() )
			{
				mode_ = Mode::CHASE;
				timer_ = 0.f;
				play_sound( "robot-chase-start", false, false );
			}
			else
			{
				mode_ = mode_backup_;
				texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );

				timer_ = 0.f;
				play_sound( "robot-shutdown" );
			}
		}
	}
	else if ( mode_ == Mode::SHUTDOWN )
	{
		set_velocity( get_velocity() * 0.5f );
		texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );

		if ( get_animation_player()->get_current_frame() >= 60 )
		{
			set_no_contact_response( true );
		}
	}
	else if ( mode_ == Mode::FLOAT )
	{
		update_velocity_by_flicker( get_start_location(), get_flicker_scale() );

		chase_direction_to( player_->get_location(), 0.5f );

		get_animation_player()->set_speed( 0.25f );
		play_animation( "Float", false, true );
		texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-tender" );
	}

	get_model()->get_shader_at( 0 )->set_texture_at( 0, texture_ );
}

void Robot::update_patrol()
{
	/*
	if ( current_patrol_point_index_ == patrol_point_list_.end() && ! patrol_point_list_.empty() )
	{
		current_patrol_point_ = patrol_point_list_.begin();
	}

	if ( current_patrol_point_ == patrol_point_list_.end() )
	{
		mode_ = Mode::STAND;
		return;
	}

	auto next_patrol_point = current_patrol_point_;

	return;

	if ( next_patrol_point == patrol_point_list_.end() )
	{
		update_velocity_by_target_location( * current_patrol_point_, 1 );
	}
	else
	{
		update_velocity_by_target_location( * next_patrol_point, 1 );
	}
	*/

	if ( current_patrol_point_index_ < patrol_point_list_.size() )
	{
		const Vector& point = patrol_point_list_[ current_patrol_point_index_ ];

		update_velocity_by_target_location( point, 0.1f );

		if ( ( point - get_location() ).length() < 0.1f )
		{
			// current_patrol_point_index_++;
			// current_patrol_point_index_ = current_patrol_point_index_ % patrol_point_list_.size();
		}

		play_animation( "Walk", false, true );
	}

	
}

void Robot::action( const string_t& s )
{
	ActiveObject::action( s );

	if ( s == "rotation" )
	{
		mode_ = Mode::ROTATION;
	}
	else if ( s == "patrol" )
	{
		mode_ = Mode::PATROL;
	}
	else if ( s == "float" )
	{
		mode_ = Mode::FLOAT;
	}
}

/**
 * ターゲット ( プレイヤー ) を見失ったかを計算する
 *
 * @param ターゲットを見失った場合は、true を、ターゲットを補足中の場合は false を返す
 */
bool Robot::caluclate_target_lost() const
{
	Vector relative_position = player_->get_location() - get_location();
	relative_position.set_y( 0 );

	auto relative_length = relative_position.length(); // ターゲットとの距離

	/// @todo ちゃんと実装
	if ( relative_length > 20.f )
	{
		return true;
	}

	return false;
}

/**
 * ターゲット ( プレイヤー ) を目視できるかを計算する
 *
 * @param ターゲット ( プレイヤー ) を目視できる場合は、true を、目視できない場合は false を返す
 */
bool Robot::caluclate_target_visible() const
{
	if ( ! player_ )
	{
		return false;
	}

	Vector relative_position = player_->get_location() - get_location();

	auto relative_length = relative_position.length(); // ターゲットとの距離
	
	/// 視野の距離
	const float_t eyeshot_length_short = 3.f;
	const float_t eyeshot_length_middle = 10.f;
	const float_t eyeshot_length_long = 20.f;

	/// 視野の距離に応じた視野角 ( -1.f .. 1.f ) ( 1.f : 正面 / 0.f : 真横 / -1.f : 真後ろ  ) 
	const float_t eyeshot_angle_short = -0.25f;
	const float_t eyeshot_angle_middle = 0.25f;
	const float_t eyeshot_angle_long = 0.75f;

	if ( relative_length > eyeshot_length_long )
	{
		return false;
	}

	// ロボットの目の位置
	Vector from = get_location();
	from.set_y( from.y() + 1.5f ); // 目線の高さを追加

	// ターゲットの目の位置
	Vector to = player_->get_location();
	to.set_y( to.y() + 1.5f ); // 目線の高さを追加
	
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
	
	// std::cout << relative_length << " : " << eyeshot_angle << " : " << get_front().dot( ( to - from ).normalize() ) << std::endl;

	// ターゲットとの距離に応じた視野角に入っていなければ、目視できない
	if ( get_front().dot( ( to - from ).normalize() ) < eyeshot_angle )
	{
		return false;
	}

	// 障害物判定
	ClosestNotMeAndHim ray_callback( from, to, get_rigid_body(), player_->get_rigid_body(), false );
	ray_callback.m_closestHitFraction = 1.0;
	
	GameMain::get_instance()->get_physics_manager()->get_dynamics_world()->rayTest( from, to, ray_callback );
	
	// ターゲットとの間に障害物がなければ目視できている
	if ( ! ray_callback.hasHit() )
	{
		std::cout << "*** target_visible ***" << std::endl;
		std::cout << "from : " << from.x() << ", " << from.y() << ", " << from.z() << std::endl;
		std::cout << "to : " << to.x() << ", " << to.y() << ", " << to.z() << std::endl;

		return true;
	}

	return false;
}

bool Robot::caluclate_collide_object_to_swtich_off( const GameObject* )
{
	return true;
}

void Robot::shutdown()
{
	mode_ = Mode::SHUTDOWN;

	stop_sound( "robot-found" );
	stop_sound( "robot-chase" );

	play_sound( "robot-shutdown", false, false );

	play_animation( "Shutdown", false, false );

	set_velocity( Vector::Zero );
}

void Robot::start_floating()
{
	mode_ = Mode::FLOAT;
}

void Robot::on_collide_with( Player* player )
{
	if ( mode_ != Mode::STAND && mode_ != Mode::ROTATION )
	{
		return;
	}

	if ( ! caluclate_collide_object_to_swtich_off( player ) )
	{
		return;
	}

	shutdown();
	play_sound( "switch-off", false, false );
}

void Robot::on_collide_with( Stone* stone )
{
	if ( mode_ != Mode::STAND && mode_ != Mode::ROTATION )
	{
		return;
	}

	if ( ! caluclate_collide_object_to_swtich_off( stone ) )
	{
		return;
	}

	if ( stone->is_moving_to( this ) )
	{
		shutdown();
		play_sound( "switch-off", false, false );
	}
}

/**
 * 巡回ポイントを追加する
 *
 * @param point 巡回ポイント
 */
void Robot::add_patrol_point( const Vector& point )
{
	patrol_point_list_.push_back( point );
}

} // namespace blue_sky

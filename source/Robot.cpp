#include "Robot.h"
#include "Player.h"
#include "Stone.h"

#include "AnimationPlayer.h"

#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"


#include "ActiveObjectPhysics.h"

#include "GameMain.h"
#include "GraphicsManager.h"

#include <game/Texture.h>

#include <common/math.h>

#include <iostream>

#include "memory.h"

namespace blue_sky
{

Robot::Robot()
	: player_( 0 )
	, texture_( 0 )
	, mode_( MODE_STAND )
	, timer_( 0 )
{
	texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );

	GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
	GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-error" );
}

void Robot::restart()
{
	ActiveObject::restart();

	get_rigid_body()->setAngularFactor( 0 );
	get_rigid_body()->setFriction( 0 );
	set_mass( 50.f );

	mode_ = MODE_STAND;
	timer_ = 0;

	texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );
}

void Robot::update()
{
	get_rigid_body()->setActivationState( true );

	if ( mode_ == MODE_CHASE )
	{
		// ターゲットの方を向く ( 表示上の向き )
		chase_direction_to( player_->get_location(), 2.f );
		
		// ターゲットとの距離が遠い場合は、ターゲットの進行方向を予測して進む
		Vector3 relative_position = player_->get_location() - get_location();
		relative_position.setY( 0 );

		Vector3 player_xz_velocity = player_->get_velocity();
		player_xz_velocity.setY( 0 );

		if ( relative_position.length() > 3.f && player_xz_velocity.length() > 1.f )
		{
			relative_position = ( player_->get_location() + player_->get_front() * 3.f ) - get_location();
			relative_position.setY( 0 );
		}

		// 進行方向を設定
		get_front() = relative_position;
		get_front().normalize();

		set_velocity( Vector3( get_front().x() * 3.f, get_velocity().y(), get_front().z() * 3.f ) );
		// get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0.8f, 0, 0, -0.25f ) );
		get_animation_player()->play( "Walk", false, true );

		if ( is_visible_in_blink( 2.f ) )
		{
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-error" );
		}
		else
		{
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
		}

		if ( caluclate_target_lost() )
		{
			mode_ = MODE_STAND;
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );
		}

		play_sound( "robot-chase", false, false );
	}
	else if ( mode_ == MODE_STAND )
	{
		set_velocity( Vector3( 0.f, 0.f, 0.f ) );
		// get_drawing_model()->get_line()->set_color( DrawingLine::Color( 0, 0, 0, 0 ) );
		// get_animation_player()->play( "Test", false, true );
		get_animation_player()->play( "Stand", false, true );
		
		if ( caluclate_target_visible() )
		{
			mode_ = MODE_ATTENTION;
			timer_ = 0.f;

			play_sound( "robot-found", false, false );
			texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-warn" );
		}
	}
	else if ( mode_ == MODE_ATTENTION )
	{
		set_velocity( Vector3( 0.f, 0.f, 0.f ) );

		// ターゲットの方を向く
		Vector3 relative_position = player_->get_location() - get_location();
		relative_position.setY( 0 );

		// 表示上の向きを設定
		float_t target_direction_degree = math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) );
		chase_direction_degree( target_direction_degree, 2.f );

		get_animation_player()->play( "Attention", false, true );

		timer_ += get_frame_elapsed_time();

		if ( timer_ >= 3.f )
		{
			mode_ = MODE_CHASE;

			play_sound( "robot-found", false, false );
		}
	}
	else if ( mode_ == MODE_SHUTDOWN )
	{
		set_velocity( get_velocity() * 0.5f );
		texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot" );
	}
	else if ( mode_ == MODE_FLOAT )
	{
		update_velocity_by_flicker( get_start_location(), get_flicker_scale() );

		chase_direction_to( player_->get_location(), 0.5f );

		get_animation_player()->set_speed( 0.25f );
		get_animation_player()->play( "Float", false, true );
		texture_ = GameMain::get_instance()->get_graphics_manager()->get_texture( "robot-tender" );
	}
}

void Robot::action( const string_t& s )
{
	if ( s == "float" )
	{
		mode_ = MODE_FLOAT;
	}
}

/**
 * ターゲット ( プレイヤー ) を見失ったかを計算する
 *
 * @param ターゲットを見失った場合は、true を、ターゲットを補足中の場合は false を返す
 */
bool Robot::caluclate_target_lost() const
{
	Vector3 relative_position = player_->get_location() - get_location();
	relative_position.setY( 0 );

	Scalar relative_length = relative_position.length(); // ターゲットとの距離

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
	Vector3 relative_position = player_->get_location() - get_location();

	Scalar relative_length = relative_position.length(); // ターゲットとの距離
	
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
	Vector3 from = get_location();
	from.setY( from.y() + 1.5f ); // 目線の高さを追加

	// ターゲットの目の位置
	Vector3 to = player_->get_location();
	to.setY( to.y() + 1.5f ); // 目線の高さを追加
	
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
	
	get_dynamics_world()->rayTest( from, to, ray_callback );
	
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

void Robot::shutdown()
{
	mode_ = MODE_SHUTDOWN;

	stop_sound( "robot-found" );
	stop_sound( "robot-chase" );

	play_sound( "robot-shutdown", false, false );

	get_animation_player()->play( "Shutdown", false, false );

	set_velocity( Vector3( 0.f, 0.f, 0.f ) );
}

void Robot::start_floating()
{
	mode_ = MODE_FLOAT;
}

void Robot::on_collide_with( Player* )
{
	if ( mode_ != MODE_STAND )
	{
		return;
	}

	shutdown();
	play_sound( "switch-off", false, false );
}

void Robot::on_collide_with( Stone* stone )
{
	if ( mode_ != MODE_STAND )
	{
		return;
	}

	if ( stone->is_moving_to( this ) )
	{
		shutdown();
		play_sound( "switch-off", false, false );
	}
}

void Robot::render_material_at( uint_t material_index ) const
{
	texture_->bind_to_ps( 0 );

	game::Material* material = get_drawing_model()->get_mesh()->get_material_at( material_index );
	material->bind_to_ia();
	material->render();
}

} // namespace blue_sky

#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include <blue_sky/GameMain.h>

/// @todo ActiveObjectManager へ移動
#include <GameObject/Player.h>
#include <GameObject/Goal.h>
#include <GameObject/Balloon.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Rectangle.h>
#include <blue_sky/graphics/Model.h>

#include <core/graphics/ShadowMap.h>

#include <core/sound/SoundManager.h>

/// @todo 抽象化する
#include <core/graphics/Direct3D11/Sprite.h>

#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/Input.h>

#include <blue_sky/ActiveObjectManager.h>

#include <core/graphics/OculusRift.h>

#include <blue_sky/DelayedCommand.h>

#include <core/math.h>

#include <game/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/math.h>

namespace blue_sky
{

GamePlayScene::GamePlayScene()
	: stage_config_( new Config() )
	, debug_texture_shader_( get_graphics_manager()->get_shader( "debug_shadow_map_texture" ) )
	, camera_( new Camera() )
	, action_bgm_after_timer_( 0.f )
	, bpm_( 120.f )
	, drawing_accent_scale_( 0.f )
	, light_position_( Vector3( 0.25f, 1.f, -1.f ), 0.1f )
	, ambient_color_( Color( 1.f, 1.f, 1.f, 1.f ), 0.02f )
	, shadow_color_( Color( 0.f, 0.f, 0.f, 1.f ), 0.02f )
	, shadow_paper_color_( Color( 0.9f, 0.9f, 0.9f, 1.f ), 0.02f )
	, shading_enabled_( true )
	, balloon_sound_type_( BalloonSoundType::MIX )
	, is_blackout_( false )
	, blackout_timer_( 0.f )
{
	// ShadowMap
	if ( get_config()->get( "graphics.shadow-map-enabled", 1 ) != 0 && stage_config_->get( "graphics.shadow-map-enabled", true ) )
	{
		get_graphics_manager()->setup_shadow_map( get_config()->get( "graphics.shadow-map-cascade-levels", 3 ), get_config()->get( "graphics.shadow-map-size", 1024 ) );
	}

	// Physics
	get_active_object_physics()->add_ground_rigid_body( Vector( 1000, 1, 1000 ) );

	// Texture
	ui_texture_ = get_graphics_manager()->load_named_texture( "ui", "media/image/item.png" );

	// Sound
	load_sound_all( get_stage_name() == "2-3" );


	// Player
	player_ = new Player();
	player_->set_model( get_graphics_manager()->load_model( "player" ) );
	get_active_object_manager()->add_active_object( player_ );
	get_active_object_manager()->name_active_object( "player", player_ );

	// Goal
	goal_ = static_cast< Goal* >( GameMain::get_instance()->create_object( "goal" ) );
	get_active_object_manager()->name_active_object( "goal", goal_ );

	// Camera
	camera_->set_fov_default( get_config()->get( "camera.fov", 90.f ) );
	camera_->reset_fov();
	camera_->set_aspect( static_cast< float >( get_width() ) / static_cast< float >( get_height() ) );

	setup_command();

	if ( get_stage_name().empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "stage name is empty" );
	}

	std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );
	load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );

	if ( ! get_graphics_manager()->is_sky_box_set() )
	{
		get_graphics_manager()->set_sky_box( "sky-box-3" );
	}

	/// @todo 直す
	// far_billboards_ = get_graphics_manager()->load_model( ( get_stage_name() + "-far-billboards" ).c_str() );
	far_billboards_ = get_graphics_manager()->load_model( "far-billboards-1" );

	/// @todo 直す
	rectangle_ = get_graphics_manager()->create_named_model( "rectangle" );
	rectangle_->set_mesh( get_graphics_manager()->create_named_mesh< Rectangle >( "rectangle", Rectangle::Buffer::Type::DEFAULT ) );

	scope_mesh_ = get_graphics_manager()->load_model( "scope" );

	bgm_ = get_sound_manager()->get_sound( "bgm" );

	if ( bgm_ )
	{
		bgm_->play( stage_config_->get( "bgm.loop", true ) );
	}

	get_graphics_manager()->load_paper_textures();

	restart();
}

GamePlayScene::~GamePlayScene()
{
	get_graphics_manager()->unset_render_target();

	get_active_object_manager()->clear();

	get_graphics_manager()->unset_sky_box();
	get_graphics_manager()->unset_ground();
	get_graphics_manager()->unset_shadow_map();

	get_active_object_physics()->clear();

	clear_delayed_command();

	get_graphics_manager()->get_sprite()->set_ortho_offset( 0.f );
}

void GamePlayScene::clear_delayed_command()
{
	delayed_command_list_.clear();
}

/**
 * ステージの設定に応じて、ステージを準備する
 *
 */
void GamePlayScene::setup_stage()
{
	for ( auto i = stage_setup_command_call_list_.begin(); i != stage_setup_command_call_list_.end(); ++i )
	{
		( *i )();
	}
}

void GamePlayScene::restart()
{
	get_graphics_manager()->set_fade_color( Color::White );
	get_graphics_manager()->fade_out();

	is_cleared_ = false;
	action_bgm_after_timer_ = 0.f;

	get_graphics_manager()->set_paper_texture_type( 0 );

	camera_->restart();

	get_active_object_manager()->restart();

	clear_delayed_command();
	setup_stage();

	play_sound( "restart" );
}

void GamePlayScene::load_sound_all( bool is_final_stage )
{
	if ( is_final_stage )
	{
		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load( "fin", "girl-see-you" );
	}
	else
	{
		get_sound_manager()->load( "walk" );
		get_sound_manager()->load( "run" );
		get_sound_manager()->load( "clamber" );
		get_sound_manager()->load( "collision-wall" );
		get_sound_manager()->load( "jump" );
		get_sound_manager()->load( "short-breath-jump" );
		get_sound_manager()->load( "land" );
		get_sound_manager()->load( "short-breath" );

		get_sound_manager()->load( "fall" );
		get_sound_manager()->load( "damage-1" );
		get_sound_manager()->load( "dead" );

		get_sound_manager()->load( "balloon" );
		get_sound_manager()->load( "balloon-1" );
		get_sound_manager()->load( "balloon-2" );
		get_sound_manager()->load( "balloon-3" );
		get_sound_manager()->load( "balloon-4" );
		get_sound_manager()->load( "balloon-5" );
		get_sound_manager()->load( "balloon-6" );
		get_sound_manager()->load( "balloon-7" );
		get_sound_manager()->load( "balloon-burst" );

		get_sound_manager()->load( "rocket-get" );
		get_sound_manager()->load( "rocket" );
		get_sound_manager()->load( "rocket-burst" );

		get_sound_manager()->load( "umbrella-get" );
		get_sound_manager()->load( "umbrella-open" );

		get_sound_manager()->load( "stone-get" );
		get_sound_manager()->load( "stone-throw" );

		get_sound_manager()->load( "switch-on" );
		get_sound_manager()->load( "switch-off" );

		get_sound_manager()->load( "ladder-contact" );
		get_sound_manager()->load( "ladder-step" );

		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load_3d_sound( "soda-can-long-1" );
		get_sound_manager()->load_3d_sound( "soda-can-long-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-1" );
		get_sound_manager()->load_3d_sound( "soda-can-short-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-3" );

		get_sound_manager()->load_3d_sound( "robot-chase" );
		get_sound_manager()->load_3d_sound( "robot-found" );
		get_sound_manager()->load_3d_sound( "robot-shutdown" );

		get_sound_manager()->load( "fin" );
		get_sound_manager()->load( "door" );
	}
}

/**
 * メインループ処理
 *
 */
void GamePlayScene::update()
{
	Scene::update();

	if ( is_blackout_ )
	{
		update_blackout();
	}
	else if ( is_cleared_ )
	{
		update_clear();
	}
	else
	{
		update_main();
	}

	get_active_object_manager()->update();

	if ( ! is_cleared_ )
	{
		get_graphics_manager()->clear_debug_bullet();
		get_active_object_physics()->update( get_elapsed_time() );

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			( *i )->update_transform();
		}
	}

	camera_->update_with_player( player_ );
	camera_->update();


	// collision_check
	get_active_object_physics()->check_collision_with( player_ );

	if ( get_active_object_physics()->is_collision( player_, goal_ ) )
	{
		on_goal();
	}

	get_active_object_physics()->check_collision_all();

	get_sound_manager()->set_listener_position( camera_->position().xyz() );
	get_sound_manager()->set_listener_orientation( camera_->front().xyz(), camera_->up().xyz() );
	get_sound_manager()->commit();

	update_delayed_command();
	update_balloon_sound();
	update_shadow();

	light_position_.chase();
	ambient_color_.chase();
	shadow_color_.chase();
	shadow_paper_color_.chase();

	get_graphics_manager()->set_ambient_color( ambient_color_.value() );

	// tess test
	{
		if ( get_input()->press( Input::Button::L2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor -= 1 * get_elapsed_time();
		}
		if ( get_input()->press( Input::Button::R2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor += 1 * get_elapsed_time();
		}

		get_graphics_manager()->get_frame_render_data()->data().tess_factor = math::clamp( get_graphics_manager()->get_frame_render_data()->data().tess_factor, 1.f, 8.f );
	}
}

/**
 * 通常時の更新処理
 *
 */
void GamePlayScene::update_main()
{
	const float_t rotation_speed_rate = camera_->fov() / camera_->get_fov_default();

	if ( ! get_oculus_rift() )
	{
		camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * get_mouse_rotation_speed() * rotation_speed_rate, 0.f, 0.f, 0.f );
		camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -get_mouse_rotation_speed(), +get_mouse_rotation_speed() ) );
	}

	player_->set_pitch( -camera_->rotate_degree_target().x() / get_mouse_rotation_speed() );

	float eye_depth_add = 0.f;

	if ( player_->is_on_ladder() )
	{
		eye_depth_add = -0.05f;
	}
	else if ( camera_->rotate_degree_target().x() > 0.f )
	{
		if ( ! get_oculus_rift() )
		{
			eye_depth_add = get_input()->get_mouse_dy();
		}
	}

	player_->add_eye_depth( eye_depth_add );

	if ( ! player_->is_dead() )
	{
		bool is_moving = false;

		if ( ! player_->is_ladder_step_only() )
		{
			if ( get_input()->press( Input::Button::LEFT ) )
			{
				player_->side_step( -1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::RIGHT ) )
			{
				player_->side_step( +1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::UP ) )
			{
				player_->step( +1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::DOWN ) )
			{
				player_->step( -1.f );
				is_moving = true;
			}
		}

		bool is_moving_on_ladder = false;

		if ( player_->is_on_ladder() )
		{
			if ( get_input()->press( Input::Button::UP ) )
			{
				player_->ladder_step( +1.f );
				is_moving_on_ladder = true;
			}
			if ( get_input()->press( Input::Button::DOWN ) )
			{
				player_->ladder_step( -1.f );
				is_moving_on_ladder = true;
			}
		}
		else
		{
			player_->release_ladder();
		}

		if ( ! is_moving )
		{
			player_->stop();
		}

		if ( ! is_moving_on_ladder )
		{
			player_->stop_ladder_step();
		}

		if ( get_input()->push( Input::Button::A ) )
		{
			switch ( player_->get_selected_item_type() )
			{
				case Player::ItemType::NONE: player_->jump(); break;
				case Player::ItemType::ROCKET: player_->rocket( camera_->front() ); break;
				case Player::ItemType::STONE: player_->throw_stone( camera_->front() ); break;
					case Player::ItemType::SCOPE:
				{
					player_->switch_scope_mode();

					if ( player_->get_action_mode() == Player::ActionMode::SCOPE )
					{
						camera_->set_fov_target( camera_->get_fov_default() * get_scope_zoom_factor() );
						camera_->set_fov( camera_->get_fov_default() * get_scope_zoom_factor() );
					}
					break;
				}
			}
		}
		else if ( get_input()->push( Input::Button::JUMP ) )
		{
			player_->jump();
		}
		else if ( get_input()->press( Input::Button::A ) || get_input()->press( Input::Button::JUMP ) )
		{
			player_->clamber();
		}
		else
		{
			player_->stop_clamber();
		}

		int wheel = get_input()->pop_mouse_wheel_queue();

		if ( player_->get_action_mode() == Player::ActionMode::SCOPE )
		{
			if ( wheel > 0 )
			{
				camera_->set_fov_target( std::max( camera_->get_fov_target() * get_scope_zoom_factor(), get_min_scope_fov() ) );
			}
			else if ( wheel < 0 )
			{
				camera_->set_fov_target( std::min( camera_->get_fov_target() / get_scope_zoom_factor(), camera_->get_fov_default() * get_scope_zoom_factor() ) );
			}
		}
		else
		{
			if ( wheel > 0 )
			{
				player_->select_next_item();
			}
			else if ( wheel < 0 )
			{
				player_->select_prev_item();
			}

			camera_->reset_fov();
		}

		float_t add_direction_degree_by_mouse = get_input()->get_mouse_dx() * get_mouse_rotation_speed() * rotation_speed_rate;
		float_t add_direction_degree_by_hmd   = get_oculus_rift() ? math::radian_to_degree( get_oculus_rift()->get_delta_yaw() ) : 0.f;

		player_->add_direction_degree( add_direction_degree_by_mouse + add_direction_degree_by_hmd );
		camera_->rotate_degree_target().set_y( player_->get_direction_degree() );

		if ( player_->is_falling_to_die() )
		{
			// 落ちて死のうとしている場合は白くフェードアウトする

			get_graphics_manager()->set_fade_color( Color( 1.f, 1.f, 1.f, 0.5f ) );
			get_graphics_manager()->fade_out( get_fade_speed_falling() );
		}
		else
		{
			get_graphics_manager()->fade_in( get_fade_speed_normal() );
		}
	}
	else
	{
		if ( get_input()->push( Input::Button::A ) )
		{
			restart();
		}
		else
		{
			get_graphics_manager()->set_fade_color( Color( 0.25f, 0.f, 0.f, 0.75f ) );
			get_graphics_manager()->fade_out( get_fade_speed_dead() );
		}
	}
}

void GamePlayScene::update_blackout()
{
	blackout_timer_ += get_elapsed_time();

	get_graphics_manager()->set_fade_color( Color::Black );
	get_graphics_manager()->fade_out();

	if ( blackout_timer_ >= get_blackout_timeout() )
	{
		go_to_next_scene();
	}
}

void GamePlayScene::update_delayed_command()
{
	for ( auto i = delayed_command_list_.begin(); i != delayed_command_list_.end(); )
	{
		if ( ( *i )->update( get_elapsed_time() ) )
		{
			exec_command( ( *i )->get_command() );
		}

		if ( ( *i )->is_over() )
		{
			i = delayed_command_list_.erase( i );
		}
		else
		{
			++i;
		}
	}
}

void GamePlayScene::update_balloon_sound()
{
	int_t balloon_sound_request = player_->pop_balloon_sound_request();

	if ( balloon_sound_request >= 1 )
	{
		if ( balloon_sound_type_ == BalloonSoundType::MIX || balloon_sound_type_ == BalloonSoundType::SOLO )
		{
			Sound* sound = get_sound_manager()->get_sound( "balloon" );

			if ( sound )
			{
				sound->play( false );
			}
		}
		else if ( balloon_sound_type_ == BalloonSoundType::SCALE )
		{
			int r = math::clamp( balloon_sound_request, 1, 7 );

			for ( int n = 1; n <= 7; n++ )
			{
				if ( n != r )
				{
					stop_sound( ( std::string( "balloon-" ) + common::serialize( n ) ).c_str() );
				}
			}

			play_sound( ( std::string( "balloon-" ) + common::serialize( r ) ).c_str() );
		}
	}

	if ( player_->get_action_mode() == Player::ActionMode::BALLOON && ( balloon_sound_type_ == BalloonSoundType::SOLO || balloon_sound_type_ == BalloonSoundType::SCALE ) )
	{
		action_bgm_after_timer_ = get_action_bgm_fade_delay();
	}
	else
	{
		action_bgm_after_timer_ -= get_elapsed_time();
	}

	if ( action_bgm_after_timer_ > 0.f )
	{
		if ( bgm_ )
		{
			bgm_->fade_out();
		}
	}
	else
	{
		if ( bgm_ )
		{
			bgm_->fade_in();
		}
	}

	/// @todo 整理する
	// ぶれるのでここでやる。
	if ( player_->get_balloon() )
	{
		const_cast< Balloon* >( player_->get_balloon() )->update();
	}
}

/**
 * 影関連の情報を更新する
 *
 */
void GamePlayScene::update_shadow()
{
	if ( get_graphics_manager()->is_shadow_enabled() )
	{
		get_graphics_manager()->get_shadow_map()->set_light_position( Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z(), 1.f ) );
		get_graphics_manager()->get_shadow_map()->set_eye_position( camera_->position() );
	}
}

void GamePlayScene::on_goal()
{
	if ( is_cleared_ )
	{
		return;
	}

	is_cleared_ = true;

	get_sound_manager()->stop_all();

	if ( Sound* fin = get_sound_manager()->get_sound( "fin" ) )
	{
		fin->play( false );
	}
}

void GamePlayScene::update_clear()
{
	Vector target_position = goal_->get_location();
	target_position.set_z( target_position.z() - get_goal_camera_z_offset() + get_sound_manager()->get_sound( "fin" )->get_current_position() * get_goal_camera_z_speed() );

	player_->set_location( player_->get_location() * ( 1.f - get_goal_position_lerp() ) + target_position * get_goal_position_lerp() );
	player_->set_direction_degree( 0.f );

	camera_->rotate_degree_target().set( 0.f, player_->get_direction_degree(), 0.f, 0.f );
	camera_->set_rotate_chase_speed( get_camera_chase_speed_on_clear() );

	get_graphics_manager()->fade_out( get_fade_speed_clear() );

	Sound* fin_sound = get_sound_manager()->get_sound( "fin" );

	if ( fin_sound )
	{
		if (
			fin_sound->get_current_position() >= get_door_sound_start() &&
			fin_sound->get_current_position() <= get_door_sound_end() &&
			get_sound_manager()->get_sound( "door" ) && ! get_sound_manager()->get_sound( "door" )->is_playing() )
		{
			get_sound_manager()->get_sound( "door" )->play( false );
		}

		if ( ! fin_sound->is_playing() )
		{
			go_to_next_scene();
		}
	}
}

void GamePlayScene::go_to_next_scene()
{
	std::string save_param_name = StageSelectScene::get_stage_prefix_by_page( get_save_data()->get( "stage-select.page", 0 ) ) + "." + get_stage_name();

	get_save_data()->set( save_param_name.c_str(), std::max( player_->has_medal() ? 2 : 1, get_save_data()->get( save_param_name.c_str(), 0 ) ) );

	if ( get_stage_name() == "2-3" )
	{
		set_next_scene( "ending" );
	}
	else
	{
		set_next_scene( "stage_outro" );
	}
}

} // namespace blue_sky

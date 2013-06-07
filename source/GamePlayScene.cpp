#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include "GameMain.h"

#include "App.h"

#include "Player.h"
#include "Goal.h"
#include "Robot.h"
#include "Balloon.h"
#include "Rocket.h"
#include "Umbrella.h"
#include "Medal.h"
#include "Camera.h"
#include "Stage.h"

#include "DrawingModelManager.h"
#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include "AnimationPlayer.h"

#include "Direct3D11.h"
#include "Direct3D11SkyBox.h"
#include "Direct3D11ShadowMap.h"
#include "Direct3D11Rectangle.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11FarBillboardsMesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Sprite.h"
#include "DirectWrite.h"
#include "DirectX.h"

#include "ActiveObjectPhysics.h"
#include "Direct3D11BulletDebugDraw.h"

#include "ConstantBuffer.h"

#include "Input.h"
#include "SoundManager.h"
#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "StaticObject.h"
#include "DynamicObject.h"
#include "TranslationObject.h"

#include "Direct3D11.h"

#include <game/Sound.h>
#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

#include <list>

#include <fstream>
#include <sstream>

namespace blue_sky
{

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, ui_texture_( 0 )
	, is_cleared_( false )
	, action_bgm_after_timer_( 0.f )
	, bpm_( 120.f )
{
	object_detail_level_0_length_ = get_config()->get( "video.object-detail-level-0-length", 500.f );
	object_detail_level_1_length_ = get_config()->get( "video.object-detail-level-1-length", 250.f );
	object_detail_level_2_length_ = get_config()->get( "video.object-detail-level-2-length", 100.f );

	get_physics()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	// Texture
	ui_texture_ = get_direct_3d()->getTextureManager()->load( "ui", "media/image/item.png" );

	// Sound
	{
		get_sound_manager()->stop_all();
		get_sound_manager()->unload_all();

		balloon_bgm_ = get_sound_manager()->load_music( "balloon" );

		get_sound_manager()->load_3d_sound( "enemy" );

		get_sound_manager()->load( "click" );

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

		get_sound_manager()->load( "balloon-get" );
		get_sound_manager()->load( "balloon-burst" );
		get_sound_manager()->load( "rocket-get" );
		get_sound_manager()->load( "rocket" );
		get_sound_manager()->load( "rocket-burst" );
		get_sound_manager()->load( "umbrella-get" );
		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load_3d_sound( "soda-can-long-1" );
		get_sound_manager()->load_3d_sound( "soda-can-long-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-1" );
		get_sound_manager()->load_3d_sound( "soda-can-short-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-3" );

		get_sound_manager()->load( "fin" );
		get_sound_manager()->load( "door" );
	}

	// Player
	player_ = new Player();
	player_->set_drawing_model( get_drawing_model_manager()->load( "player" ) );

	// Goal
	goal_ = new Goal();
	goal_->set_drawing_model( get_drawing_model_manager()->load( "goal" ) );

	// Camera
	camera_ = new Camera();
	camera_->set_fov_default( get_config()->get( "camera.fov", 90.f ) );
	camera_->set_aspect( static_cast< float >( get_width() ) / static_cast< float >( get_height() ) );
	camera_->reset_fov();

	// balloon_model_ = get_drawing_model_manager()->load( "balloon" );

	if ( get_stage_name().empty() )
	{
		generate_random_stage();
	}
	else
	{
		std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );

		load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );
	}

	if ( get_config()->get( "video.shadow-map-enabled", 1 ) != 0 )
	{
		shadow_map_ = new ShadowMap( get_direct_3d(), get_config()->get( "video.shadow-map-cascade-levels", 3 ), get_config()->get( "video.shadow-map-size", 1024 ) );
	}

	if ( ! sky_box_ )
	{
		sky_box_ = new SkyBox( get_direct_3d(), "sky-box-3" );
	}

	if ( ! ground_ )
	{
		ground_ = new Mesh( get_direct_3d() );
		ground_->load_obj( "media/model/ground.obj" );
	}

	if ( ! far_billboards_ )
	{
		far_billboards_ = new FarBillboardsMesh( get_direct_3d() );
		far_billboards_->load_obj( ( std::string( "media/model/stage-" ) + get_stage_name() + "-far-billboards.obj" ).c_str() );
	}

	rectangle_ = new Rectangle( get_direct_3d() );

	bgm_ = get_sound_manager()->get_sound( "balloon" );

	if ( bgm_ )
	{
		bgm_->play( true );
	}
	
	update_render_data_for_game();

	restart();
}

GamePlayScene::~GamePlayScene()
{
	get_drawing_model_manager()->clear();
	get_active_object_manager()->clear();

	get_direct_3d()->getMeshManager()->unload_all();
	get_direct_3d()->getTextureManager()->unload_all();

	get_sound_manager()->stop_all();
	get_sound_manager()->unload_all();

	get_physics()->clear();
}

void GamePlayScene::restart()
{
	get_direct_3d()->getFader()->set_color( Direct3D::Color::White );
	get_direct_3d()->getFader()->full_out();

	is_cleared_ = false;
	action_bgm_after_timer_ = 0.f;

	player_->restart();
	goal_->restart();

	camera_->restart();

	for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
	{
		ActiveObject* active_object = *i;
		active_object->restart();
	}

	play_sound( "restart" );
}

void GamePlayScene::generate_random_stage()
{
	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "floor" );

		for ( int n = 0; n < 10; n++ )
		{
			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( 0.f, 0.f, n * 20.f );

			get_active_object_manager()->add_active_object( static_object );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "soda-can" );

		for ( int n = 0; n < 5; n++ )
		{
			DynamicObject* object = new DynamicObject( 0.07f, 0.12f, 0.07f );
			object->set_drawing_model( drawing_model );
			object->set_location( 5.f + n * 0.5f, 20.f, 0.f );

			get_active_object_manager()->add_active_object( object );

			// object->set_rigid_body( get_physics()->add_active_object( object ) );
			object->set_rigid_body( get_physics()->add_active_object_as_cylinder( object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "wall-1" );

		for ( int n = 0; n < 4; n++ )
		{
			StaticObject* static_object = new StaticObject( 4.f, 1.75f, 0.1f );
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( n * 5.f, 0, 0.f );

			get_active_object_manager()->add_active_object( static_object );

			static_object->set_rigid_body( get_physics()->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "outdoor-unit" );

		for ( int n = 0; n < 10; n++ )
		{
			StaticObject* static_object = new StaticObject( 0.7f, 0.6f, 0.24f );
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( 4.f, 0, -n * 2.f );

			get_active_object_manager()->add_active_object( static_object );

			static_object->set_rigid_body( get_physics()->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "building-20" );

		for ( int n = 0; n < 10; n++ )
		{
			StaticObject* static_object = new StaticObject( 10, 20, 10 );
			static_object->set_drawing_model( drawing_model );
			static_object->set_start_location( 10.f, 0, n * 12.f );
			static_object->set_start_rotation( 15.f, 0, 0 );

			get_active_object_manager()->add_active_object( static_object );

			static_object->set_rigid_body( get_physics()->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "building-200" );

		for ( int n = 0; n < 5; n++ )
		{
			StaticObject* static_object = new StaticObject( 80, 200, 60 );
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( -40, 0, n * 70.f );

			get_active_object_manager()->add_active_object( static_object );

			static_object->set_rigid_body( get_physics()->add_active_object( static_object ) );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "tree-1" );

		for ( int n = 0; n < 20; n++ )
		{
			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( -5.f + rand() % 3, 0, n * 5.f + rand() % 2 );

			get_active_object_manager()->add_active_object( static_object );
		}
	}

	{
		DrawingModel* drawing_model = get_drawing_model_manager()->load( "rocket" );

		StaticObject* static_object = new StaticObject();
		static_object->set_drawing_model( drawing_model );
		static_object->set_location( 0, 0, 0 );

		get_active_object_manager()->add_active_object( static_object );
	}

	{
		for ( int n = 0; n < 10; n++ )
		{
			DrawingModel* drawing_model = get_drawing_model_manager()->load( "balloon" );

			StaticObject* static_object = new StaticObject();
			static_object->set_drawing_model( drawing_model );
			static_object->set_location( float_t( rand() % 3 ), float_t( 3 + n * 3 ), float_t( n * 3 ) );

			get_active_object_manager()->add_active_object( static_object );
		}
	}
}

/**
 * ステージファイルを読み込む
 *
 * @param file_name ステージファイル名
 */
void GamePlayScene::load_stage_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load stage file \"" ) + file_name + "\" failed." );
	}

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		std::stringstream ss;
		
		std::string name;
		std::string value;
		
		ss << line;

		ss >> name;

		ActiveObject* active_object = 0;

	
		if ( name == "bgm" )
		{
			std::string name;
			ss >> name;

			get_sound_manager()->load_music( "bgm", name.c_str() );

			if ( ss.good() )
			{
				float_t bpm = get_bpm();

				ss >> bpm;

				set_bpm( bpm );
			}
		}
		/*
		else if ( name == "ambient" )
		{
			ss >> ambient_color_[ 0 ] >> ambient_color_[ 1 ] >> ambient_color_[ 2 ];
		}
		*/
		else if ( name == "player" )
		{
			float_t x = 0, y = 0, z = 0;

			ss >> x >> y >> z;

			player_->set_start_location( x, y, z );
			player_->set_rigid_body( get_physics()->add_active_object( player_.get() ) );

			if ( ! ss.eof() )
			{
				ss >> camera_->rotate_degree().x();
				camera_->rotate_degree_target().x() = camera_->rotate_degree().x();
			}
		}
		else if ( name == "goal" )
		{
			float_t x = 0, y = 0, z = 0;

			ss >> x >> y >> z;

			goal_->set_start_location( x, y, z );
			goal_->set_rigid_body( get_physics()->add_active_object( goal_.get() ) );
		}
		else if ( name == "sky-box" )
		{
			std::string sky_box_name;
			std::string sky_box_ext;

			ss >> sky_box_name >> sky_box_ext;

			sky_box_.release();

			if ( sky_box_name != "none" )
			{
				sky_box_ = new SkyBox( get_direct_3d(), sky_box_name.c_str(), sky_box_ext.c_str() );
			}
		}
		else if ( name == "far-billboards" )
		{
			std::string far_billboards_name;

			ss >> far_billboards_name;

			far_billboards_ = new FarBillboardsMesh( get_direct_3d() );
			far_billboards_->load_obj( ( std::string( "media/model/" ) + far_billboards_name + ".obj" ).c_str() );
		}
		else if ( name == "object" || name == "static-object" || name == "dynamic-object" )
		{
			std::string object_name;
			float x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;

			ss >> object_name >> x >> y >> z >> rx >> ry >> rz;

			DrawingModel* drawing_model = get_drawing_model_manager()->load( object_name.c_str() );

			std::map< string_t, ActiveObject::Vector3 > size_map;
			size_map[ "soda-can-1"   ] = ActiveObject::Vector3(  0.07f, 0.12f, 0.07f );
			size_map[ "wall-1"       ] = ActiveObject::Vector3(  4.f,   1.75f, 0.2f  );
			size_map[ "wall-2"       ] = ActiveObject::Vector3(  8.f,   2.5f,  0.2f  );
			size_map[ "outdoor-unit" ] = ActiveObject::Vector3(  0.7f,  0.6f,  0.24f );
			size_map[ "building-20"  ] = ActiveObject::Vector3( 10.f,  20.f,  10.f   );
			size_map[ "building-200" ] = ActiveObject::Vector3( 80.f, 200.f,  60.f   );
			size_map[ "board-1"      ] = ActiveObject::Vector3(  4.f,   0.2f,  0.8f  );

			size_map[ "box-5x5x5"    ] = ActiveObject::Vector3(  5.f,  5.f,  5.f );

			std::map< string_t, float_t > mass_map;
			mass_map[ "soda-can-1"   ] = 0.001f;
			mass_map[ "board-1"      ] = 20.f;

			float w = 0.f, h = 0.f, d = 0.f, mass = 0.f;
			
			{
				auto i = size_map.find( object_name );

				if ( i != size_map.end() )
				{
					w = i->second.x();
					h = i->second.y();
					d = i->second.z();
				}
			}

			{
				auto i = mass_map.find( object_name );

				if ( i != mass_map.end() )
				{
					mass = i->second;
				}
			}

			ActiveObject* object = 0;

			if ( name == "dynamic-object" )
			{
				DynamicObject* dynamic_object = new DynamicObject( w, h, d );

				if ( object_name == "soda-can-1" )
				{
					dynamic_object->set_collision_sound_name( "soda-can-long-1" );
				}

				object = dynamic_object;
			}
			else
			{
				object = new StaticObject( w, h, d );
			}
			
			object->set_drawing_model( drawing_model );
			object->setup_animation_player();

			/*** @todo ちゃんとする */
			if ( object->get_animation_player() )
			{
				object->get_animation_player()->play( "Walk", true, false );
			}

			object->set_start_location( x, y, z );
			object->set_start_rotation( rx, ry, rz );

			if ( object_name == "soda-can-1" )
			{
				object->set_rigid_body( get_physics()->add_active_object_as_cylinder( object ) );
			}
			else
			{
				object->set_rigid_body( get_physics()->add_active_object( object ) );
			}

			object->set_mass( mass );
			get_active_object_manager()->add_active_object( object );
		}
		else if ( name == "translation-object" )
		{
			float_t x = 0, y = 0, z = 0, tw = 0, th = 0, td = 0, s = 0.01f;
			ss >> x >> y >> z >> tw >> th >> td >> s;

			TranslationObject* object = new TranslationObject( 5, 5, 5, tw, th, td, s );

			DrawingModel* drawing_model = get_drawing_model_manager()->load( "box-5x5x5" );
			
			object->set_drawing_model( drawing_model );
			object->set_start_location( x, y, z );

			object->set_rigid_body( get_physics()->add_active_object( object ) );
			get_active_object_manager()->add_active_object( object );
		}
		else if ( name == "robot" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			Robot* robot = new Robot();
			robot->set_player( player_.get() );

			robot->add_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot-l" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot-r" ) );

			robot->set_start_location( x, y, z );

			get_active_object_manager()->add_active_object( robot );
			robot->set_rigid_body( get_physics()->add_active_object( robot ) );
		}
		else if ( name == "balloon" )
		{
			active_object = new Balloon();
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
		}
		else if ( name == "medal" )
		{
			active_object = new Medal();
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
		}
		/*
		else if ( name == "rocket" )
		{
			active_object = new Rocket();
		}
		else if ( name == "umbrella" )
		{
			active_object = new Umbrella();
		}
		
		else if ( name == "poison" )
		{
			active_object = new Poison();
		}
		*/

		if ( active_object )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			DrawingModel* drawing_model = get_drawing_model_manager()->load( name.c_str() );

			active_object->set_drawing_model( drawing_model );
			active_object->setup_animation_player();

			active_object->set_start_location( x, y, z );
			active_object->set_direction_degree( r );
			
			get_active_object_manager()->add_active_object( active_object );
		}
	}
}

void GamePlayScene::save_stage_file( const char* file_name ) const
{

}

/**
 * メインループ処理
 *
 */
void GamePlayScene::update()
{
	Scene::update();

	if ( is_cleared_ )
	{
		update_clear();
	}
	else
	{
		update_main();
		player_->update();
	}

	get_active_object_manager()->update();

	if ( ! is_cleared_ )
	{
		get_physics()->update( 1.f / get_main_loop()->get_fps() );
		// get_physics()->update( get_elapsed_time() );

		player_->update_transform();

		for ( ActiveObjectManager::ActiveObjectList::iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			( *i )->update_transform();
		}
	}

	camera_->update_with_player( player_.get() );
	camera_->update();
	

	// collision_check
	get_physics()->check_collision_with( player_.get() );

	if ( get_physics()->is_collision( player_.get(), goal_.get() ) )
	{
		on_goal();
	}
	
	get_physics()->check_collision_dynamic_object();

	get_sound_manager()->set_listener_position( camera_->position() );
	// get_sound_manager()->set_listener_velocity( player_->get_velocity() );
	get_sound_manager()->set_listener_orientation( camera_->front(), camera_->up() );
	get_sound_manager()->commit();

	if ( player_->get_action_mode() == Player::ACTION_MODE_BALLOON )
	{
		action_bgm_after_timer_ = 2.f;
	}
	else
	{
		action_bgm_after_timer_ -= get_elapsed_time();
	}

	if ( action_bgm_after_timer_ > 0.f )
	{
		bgm_->fade_out();

		if ( balloon_bgm_->is_playing() )
		{
			balloon_bgm_->fade_in( Sound::VOLUME_FADE_SPEED_FAST );
		}
		else
		{
			balloon_bgm_->play( true );
			balloon_bgm_->set_volume( Sound::VOLUME_MAX );
		}
	}
	else
	{
		balloon_bgm_->fade_out();

		bgm_->play( true, false );
		bgm_->fade_in();
	}
}

void GamePlayScene::update_main()
{
	if ( ! player_->is_dead() )
	{
		bool is_moving = false;

		if ( get_input()->press( Input::LEFT ) )
		{
			player_->side_step( -1.f );
			is_moving = true;
		}
		if ( get_input()->press( Input::RIGHT ) )
		{
			player_->side_step( +1.f );
			is_moving = true;
		}
		if ( get_input()->press( Input::UP ) )
		{
			player_->step( +1.f );
			is_moving = true;
		}
		if ( get_input()->press( Input::DOWN ) )
		{
			player_->step( -1.f );
			is_moving = true;
		}

		if ( ! is_moving )
		{
			player_->stop();
		}

		if ( get_input()->push( Input::A ) )
		{
			player_->jump();
		}

		if ( get_input()->push( Input::B ) )
		{
			player_->super_jump();
		}

		player_->add_direction_degree( get_input()->get_mouse_dx() * 90.f );

		camera_->rotate_degree_target().y() = player_->get_direction_degree();

		get_direct_3d()->getFader()->fade_in( 0.05f );
	}
	else
	{
		if ( get_input()->push( Input::A ) )
		{
			restart();
		}
		else
		{
			get_direct_3d()->getFader()->set_color( Direct3D::Color( 0.25f, 0.f, 0.f, 0.75f ) );
			get_direct_3d()->getFader()->fade_out( 0.05f );
		}
	}

	camera_->rotate_degree_target().x() += get_input()->get_mouse_dy() * 90.f;
	camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );

	float eye_depth_add = 0.f;

	if ( camera_->rotate_degree_target().x() > 0.f )
	{
		eye_depth_add = get_input()->get_mouse_dy();
	}

	player_->add_eye_depth( eye_depth_add );
}

void GamePlayScene::on_goal()
{
	if ( is_cleared_ )
	{
		return;
	}

	is_cleared_ = true;

	get_sound_manager()->stop_all();
	get_sound_manager()->get_sound( "fin" )->play( false );
}

void GamePlayScene::update_clear()
{
	ActiveObject::Vector3 target_position = goal_->get_location();
	target_position.setZ( target_position.z() - 4.f + get_sound_manager()->get_sound( "fin" )->get_current_position() * 0.5f );

	player_->set_location( player_->get_location() * 0.95f + target_position * 0.05f );
	player_->set_direction_degree( 0.f );

	camera_->rotate_degree_target().x() = 0.f;
	camera_->rotate_degree_target().y() = player_->get_direction_degree();
	camera_->rotate_degree_target().z() = 0.f;
	camera_->set_rotate_chase_speed( 0.1f );

	get_direct_3d()->getFader()->fade_out( 0.0025f );

	if (
		get_sound_manager()->get_sound( "fin" )->get_current_position() >= 6.f &&
		get_sound_manager()->get_sound( "fin" )->get_current_position() <= 8.f &&
		! get_sound_manager()->get_sound( "door" )->is_playing() )
	{
		get_sound_manager()->get_sound( "door" )->play( false );
	}
	
	if ( get_sound_manager()->get_sound( "fin" )->get_current_position() >= 10.f )
	{
		std::string save_param_name = StageSelectScene::get_stage_prefix_by_page( get_save_data()->get( "stage-select.page", 0 ) ) + "." + get_stage_name();

		get_save_data()->set( save_param_name.c_str(), std::max( player_->has_medal() ? 2 : 1, get_save_data()->get( save_param_name.c_str(), 0 ) ) );
		set_next_scene( "stage_outro" );
	}
}

/**
 * 描画
 *
 */
void GamePlayScene::render()
{
	get_direct_3d()->setInputLayout( "main" );
	
	// render_text()
	{
		get_direct_3d()->begin2D();
		get_direct_3d()->getFont()->begin();

		render_text();

		get_direct_3d()->getFont()->end();
		get_direct_3d()->end2D();
	}

	// render_3d()
	{
		get_direct_3d()->begin3D();

		get_direct_3d()->clear();

		update_render_data_for_frame();
		update_render_data_for_object();

		render_shadow_map();

		get_direct_3d()->set_default_render_target();
		get_direct_3d()->set_default_viewport();

		render_sky_box();
		render_far_billboards();

		get_direct_3d()->setInputLayout( "skin" );

		render_object_skin_mesh();

		get_direct_3d()->setInputLayout( "main" );

		render_object_mesh();

		get_direct_3d()->setInputLayout( "line" );

		render_object_line();

		render_bullet_debug();

		get_direct_3d()->setInputLayout( "main" );

		get_direct_3d()->renderText();
		
		// render_shadow_map_debug_window();

		render_sprite();

		render_fader();

		get_direct_3d()->end3D();
	}
}

void GamePlayScene::render_text() const
{
	std::wstringstream ss;
	ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

	if ( get_game_main()->is_display_fps() )
	{
		ss << L"FPS : " << get_main_loop()->get_last_fps() << std::endl;
		ss << L"BPM : " << get_bpm() << std::endl;
		ss << L"POS : " << player_->get_transform().getOrigin().x() << ", " << player_->get_transform().getOrigin().y() << ", " << player_->get_transform().getOrigin().z() << std::endl;
		ss << L"step speed : " << player_->get_step_speed() << std::endl;
		ss << L"last footing height : " << player_->get_last_footing_height() << std::endl;
		ss << L"DX : " << player_->get_rigid_body()->getLinearVelocity().x() << std::endl;
		ss << L"DY : " << player_->get_rigid_body()->getLinearVelocity().y() << std::endl;
		ss << L"DZ : " << player_->get_rigid_body()->getLinearVelocity().z() << std::endl;
		ss << L"Objects : " << get_active_object_manager()->active_object_list().size() << std::endl;

		ss << L"mouse.dx : " << get_input()->get_mouse_dx() << std::endl;
		ss << L"mouse.dy : " << get_input()->get_mouse_dy() << std::endl;
	}

	get_direct_3d()->getFont()->draw_text( 10.f, 10.f, get_app()->get_width() - 10.f, get_app()->get_height() - 10.f, ss.str().c_str(), Direct3D::Color( 1.f, 0.95f, 0.95f, 1.f ) );
}

/**
 * ゲーム毎に更新する必要のある描画用の定数バッファを更新する
 *
 */
void GamePlayScene::update_render_data_for_game() const
{
	GameConstantBufferData constant_buffer_data;

	constant_buffer_data.projection = XMMatrixPerspectiveFovLH( math::degree_to_radian( camera_->fov() ), camera_->aspect(), 0.05f, 3000.f );
	constant_buffer_data.projection = XMMatrixTranspose( constant_buffer_data.projection );
	constant_buffer_data.screen_width = static_cast< float_t >( get_width() );
	constant_buffer_data.screen_height = static_cast< float_t >( get_height() );
		
	get_game_main()->get_game_constant_buffer()->update( & constant_buffer_data );
}

/**
 * フレーム毎に更新する必要のある描画用の定数バッファを更新する
 *
 */
void GamePlayScene::update_render_data_for_frame() const
{
	{
		XMVECTOR eye = XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 );
		XMVECTOR at = XMVectorSet( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 0.f );
		XMVECTOR up = XMVectorSet( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.f );

		FrameConstantBufferData frame_constant_buffer_data;

		frame_constant_buffer_data.view = XMMatrixLookAtLH( eye, at, up );
		frame_constant_buffer_data.view = XMMatrixTranspose( frame_constant_buffer_data.view );
		frame_constant_buffer_data.time = get_total_elapsed_time();
		frame_constant_buffer_data.time_beat = static_cast< uint_t >( get_total_elapsed_time() * ( get_bpm() / 60.f ) );

		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}

	{
		FrameDrawingConstantBufferData frame_drawing_constant_buffer_data;

		frame_drawing_constant_buffer_data.accent = bgm_->get_current_peak_level();

		get_game_main()->get_frame_drawing_constant_buffer()->update( & frame_drawing_constant_buffer_data );
	}
}

/**
 * 全てのオブジェクトの描画用の定数バッファを更新する
 *
 */
void GamePlayScene::update_render_data_for_object() const
{
	for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
	{
		update_render_data_for_active_object( *i );
	}

	update_render_data_for_active_object( player_.get() );
	update_render_data_for_active_object( goal_.get() );
}

/**
 * 指定した ActiveObject の描画用の定数バッファを更新する
 *
 * @param active_object ActiveObject
 */
void GamePlayScene::update_render_data_for_active_object( const ActiveObject* active_object ) const
{
	if ( active_object->is_dead() )
	{
		return;
	}

	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBufferData buffer_data;

	buffer_data.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer_data.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer_data.world = XMMatrixTranspose( buffer_data.world );
	buffer_data.color = active_object->get_drawing_model()->get_line()->get_color();

	active_object->get_object_constant_buffer()->update( & buffer_data );

	if ( active_object->get_animation_player() )
	{
		active_object->get_animation_player()->update_render_data();
	}
}

/**
 * ゲーム毎の定数バッファをバインドする
 *
 */
void GamePlayScene::bind_game_constant_buffer() const
{
	get_game_main()->get_game_constant_buffer()->bind_to_vs();
	get_game_main()->get_game_constant_buffer()->bind_to_gs();
}

/**
 * フレーム毎の定数バッファをバインドする
 *
 */
void GamePlayScene::bind_frame_constant_buffer() const
{
	get_game_main()->get_frame_constant_buffer()->bind_to_all();
}

/**
 * オブジェクト毎の定数バッファをバインドする
 *
 */
void GamePlayScene::bind_object_constant_buffer() const
{
	get_game_main()->get_object_constant_buffer()->bind_to_vs();
	get_game_main()->get_object_constant_buffer()->bind_to_ps();
}

/**
 * シャドウマップを描画する
 *
 */
void GamePlayScene::render_shadow_map() const
{
	if ( ! shadow_map_ )
	{
		return;
	}

	/*
	static float a = 0.f;

	a += 0.01f;

	const XMVECTOR light_origin = XMVectorSet( 50.f, 100.f, -25.f, 0.f );
	XMVECTOR light = light_origin + XMVectorSet( cos( a ) * 10.f, 0.f, sin( a ) * 10.f, 0.f );	

	shadow_map_->setLightPosition( light );
	*/

	shadow_map_->setEyePosition( XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 ) );
	shadow_map_->ready_to_render_shadow_map();

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|shadow_map" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		for ( int n = 0; n < shadow_map_->get_cascade_levels(); n++ )
		{
			shadow_map_->ready_to_render_shadow_map_with_cascade_level( n );

			bind_game_constant_buffer();
			bind_frame_constant_buffer();

			for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
			{
				render_active_object_mesh( *i );
			}

			render_active_object_mesh( player_.get() );
			render_active_object_mesh( goal_.get() );
		}
	}

	// shadow_map_->finish_to_render_shadow_map();
}

/**
 * スカイボックスを描画する
 *
 */
void GamePlayScene::render_sky_box() const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sky_box" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		bind_object_constant_buffer();

		ObjectConstantBufferData object_constant_buffer_data;

		// sky box
		{
			object_constant_buffer_data.world = XMMatrixTranslationFromVector( XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 ) );
			object_constant_buffer_data.world = XMMatrixTranspose( object_constant_buffer_data.world );

			get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );

			sky_box_->render();
		}

		// ground
		{
			object_constant_buffer_data.world = XMMatrixIdentity();

			get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
	
			ground_->render();
		}
	}
}

/**
 * 遠景ビルボードメッシュを描画する
 *
 */
void GamePlayScene::render_far_billboards() const
{
	if ( ! far_billboards_ )
	{
		return;
	}

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|billboard" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		{
			ObjectConstantBufferData buffer;
			buffer.world = XMMatrixIdentity();
					
			get_game_main()->get_object_constant_buffer()->update( & buffer );
		}

		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		bind_object_constant_buffer();

		far_billboards_->render();
	}
}

/**
 * オブジェクトのスキンメッシュを描画する
 *
 */
void GamePlayScene::render_object_skin_mesh() const
{
	/// @todo skin_with_shadow
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( shadow_map_ ? "|skin" : "|skin" );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
		
		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs();

		if ( shadow_map_ )
		{
			shadow_map_->ready_to_render_scene();
		}		

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			if ( ( *i )->get_drawing_model()->is_skin_mesh() )
			{
				render_active_object_mesh( *i );
			}
		}
	}
}

/**
 * オブジェクトのメッシュを描画する
 *
 */
void GamePlayScene::render_object_mesh() const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( shadow_map_ ? "|main_with_shadow" : "|main" );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs();

		if ( shadow_map_ )
		{
			shadow_map_->ready_to_render_scene();
		}		

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			if ( ! ( *i )->get_drawing_model()->is_skin_mesh() )
			{
				render_active_object_mesh( *i );
			}
		}

		render_active_object_mesh( goal_.get() );
	}
}

/**
 * オブジェクトの線を描画する
 *
 */
void GamePlayScene::render_object_line() const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|drawing_line" );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs();

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			render_active_object_line( *i );
		}

		render_active_object_line( player_.get() );
	}
}

/**
 * ActiveObject のメッシュを描画する
 *
 * @param active_object ActiveObject
 */
void GamePlayScene::render_active_object_mesh( const ActiveObject* active_object ) const
{
	if ( active_object->is_dead() )
	{
		return;
	}
	
	active_object->get_object_constant_buffer()->bind_to_vs();
	active_object->get_object_constant_buffer()->bind_to_ps();
	
	if ( active_object->get_animation_player() )
	{
		active_object->get_animation_player()->bind_render_data();
	}

	active_object->get_drawing_model()->get_mesh()->render();
}

/**
 * ActiveObject の線を描画する
 *
 * @param active_object ActiveObject
 */
void GamePlayScene::render_active_object_line( const ActiveObject* active_object ) const
{
	if ( active_object->is_dead() )
	{
		return;
	}

	active_object->get_object_constant_buffer()->bind_to_vs();
	active_object->get_object_constant_buffer()->bind_to_ps();

	active_object->get_drawing_model()->get_line()->render(); // 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
}

/**
 * 2D スプライトを描画する
 *
 */
void GamePlayScene::render_sprite()
{
	get_direct_3d()->getSprite()->begin();

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

	for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		if ( player_->has_medal() )
		{
			win::Rect src_rect = win::Rect::Size( 384, 0, 64, 64 );
			win::Point dst_point( 5, get_height() - src_rect.height() - 5 );

			get_direct_3d()->getSprite()->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
		}
	}

	get_direct_3d()->getSprite()->end();
}

/**
 * Bullet のデバッグ表示を描画する
 *
 */
void GamePlayScene::render_bullet_debug() const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|bullet" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		bind_game_constant_buffer();
		bind_frame_constant_buffer();

		get_game_main()->get_bullet_debug_draw()->render();
	}
}

/**
 * シャドウマップのデバッグウィンドウを描画する
 *
 */
void GamePlayScene::render_shadow_map_debug_window() const
{
	if ( ! shadow_map_ )
	{
		return;
	}

	get_direct_3d()->setDebugViewport( 0.f, 0, get_width() / 4.f * shadow_map_->get_cascade_levels(), get_height() / 4.f );

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		ObjectConstantBufferData buffer_data;
				
		buffer_data.world = XMMatrixIdentity();
		buffer_data.color = Color( 0.5f, 0.f, 0.f, 0.f );

		get_game_main()->get_object_constant_buffer()->update( & buffer_data );
		
		bind_object_constant_buffer();

		( * rectangle_->get_material_list().begin() )->set_shader_resource_view( shadow_map_->getShaderResourceView() );
		rectangle_->render();
	}
}

} // namespace blue_sky
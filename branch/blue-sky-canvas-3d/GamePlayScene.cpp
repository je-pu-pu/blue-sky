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
#include "DrawingLine.h"

#include "Direct3D11.h"
#include "Direct3D11SkyBox.h"
#include "Direct3D11ShadowMap.h"
#include "Direct3D11Rectangle.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11Material.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Effect.h"
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

static const bool is_render_2d_enabled = true;
static FrameConstantBuffer frame_constant_buffer; /// @todo メンバにする

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, ui_texture_( 0 )
	, is_cleared_( false )
	, action_bgm_after_timer_( 0.f )
{
	object_detail_level_0_length_ = get_config()->get( "video.object-detail-level-0-length", 500.f );
	object_detail_level_1_length_ = get_config()->get( "video.object-detail-level-1-length", 250.f );
	object_detail_level_2_length_ = get_config()->get( "video.object-detail-level-2-length", 100.f );

	get_physics()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	// Texture
	// ui_texture_ = get_direct_3d()->getTextureManager()->load( "ui", "media/image/item.png" );

	// Sound
	{
		get_sound_manager()->stop_all();
		get_sound_manager()->unload_all();

		balloon_bgm_ = get_sound_manager()->load_music( "balloon" );

		get_sound_manager()->load_3d_sound( "enemy" );
		// sound_manager()->load_3d_sound( "vending-machine" );

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

		get_sound_manager()->load_3d_sound( "balloon-get" );
		get_sound_manager()->load_3d_sound( "balloon-burst" );
		get_sound_manager()->load_3d_sound( "rocket-get" );
		get_sound_manager()->load_3d_sound( "rocket" );
		get_sound_manager()->load_3d_sound( "rocket-burst" );
		get_sound_manager()->load_3d_sound( "umbrella-get" );
		get_sound_manager()->load_3d_sound( "medal-get" );

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

	{
		GameConstantBuffer constant_buffer;
		constant_buffer.projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, get_app()->get_width() / ( FLOAT ) get_app()->get_height(), 0.05f, 3000.f );
		constant_buffer.projection = XMMatrixTranspose( constant_buffer.projection );
		
		get_game_main()->get_game_constant_buffer()->update( & constant_buffer );
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

	rectangle_ = new Rectangle( get_direct_3d() );

	bgm_ = get_sound_manager()->get_sound( "bgm" );
	if ( bgm_ )
	{
		bgm_->play( true );
	}

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
		else if ( name == "object" || name == "static-object" || name == "dynamic-object" )
		{
			std::string object_name;
			float x = 0, y = 0, z = 0, r = 0;

			ss >> object_name >> x >> y >> z >> r;

			DrawingModel* drawing_model = get_drawing_model_manager()->load( object_name.c_str() );

			std::map< string_t, ActiveObject::Vector3 > size_map;
			size_map[ "soda-can-1"   ] = ActiveObject::Vector3(  0.07f, 0.12f, 0.07f );
			size_map[ "wall-1"       ] = ActiveObject::Vector3(  4.f,   1.75f, 0.2f  );
			size_map[ "wall-2"       ] = ActiveObject::Vector3(  8.f,   2.5f,  0.2f  );
			size_map[ "outdoor-unit" ] = ActiveObject::Vector3(  0.7f,  0.6f,  0.24f );
			size_map[ "building-20"  ] = ActiveObject::Vector3( 10.f,  20.f,  10.f   );
			size_map[ "building-200" ] = ActiveObject::Vector3( 80.f, 200.f,  60.f   );
			size_map[ "board-1"      ] = ActiveObject::Vector3(  4.f,   0.2f,  0.8f  );

			std::map< string_t, float_t > mass_map;
			mass_map[ "soda-can-1"   ] = 0.1f;
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
				object = new DynamicObject( w, h, d );
			}
			else
			{
				object = new StaticObject( w, h, d );
			}
			
			object->set_drawing_model( drawing_model );
			object->set_start_location( x, y, z );
			object->set_start_rotation( r, 0, 0 );

			if ( object_name == "soda-can" )
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
		/*
		else if ( name == "enemy" )
		{
			Enemy* enemy = new Enemy();
			enemy->set_player( player_.get() );
			active_object = enemy;
		}
		*/
		else if ( name == "balloon" )
		{
			active_object = new Balloon();
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
		else if ( name == "medal" )
		{
			active_object = new Medal();
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
	}

	player_->update();

	get_active_object_manager()->update();

	if ( ! is_cleared_ )
	{
		get_physics()->update( get_elapsed_time() );

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

		if ( get_input()->push( Input::X ) )
		{
			get_physics()->setConstraint();
		}
		if ( get_input()->push( Input::Y ) )
		{
			player_->damage( Player::Vector3( 0, 10, 0.f ) );
		}
		if ( get_input()->push( Input::A ) )
		{
			player_->jump();
		}

		if ( get_input()->push( Input::B ) )
		{
			Robot* robot = new Robot();
			robot->set_player( player_.get() );

			robot->add_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot-l" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->add_drawing_model( get_drawing_model_manager()->load( "robot-r" ) );

			robot->set_start_location( player_->get_transform().getOrigin().getX(), player_->get_transform().getOrigin().getY() + 20, player_->get_transform().getOrigin().getZ() + 5 );

			get_active_object_manager()->add_active_object( robot );
			robot->set_rigid_body( get_physics()->add_active_object( robot ) );
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

	camera_->rotate_degree_target().y() += get_input()->get_mouse_dx() * 90.f;
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
 */
void GamePlayScene::render()
{
	get_direct_3d()->setInputLayout( "main" );

	// render_2d()
	if ( is_render_2d_enabled )
	{
		get_direct_3d()->begin2D();

		get_direct_3d()->getFont()->begin();

		std::wstringstream ss;
		ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

		ss << L"まぁがくれた月" << std::endl;
		ss << L"FPS : " << get_main_loop()->get_last_fps() << std::endl;
		ss << L"POS : " << player_->get_transform().getOrigin().x() << ", " << player_->get_transform().getOrigin().y() << ", " << player_->get_transform().getOrigin().z() << std::endl;
		ss << L"step speed : " << player_->get_step_speed() << std::endl;
		ss << L"last footing height : " << player_->get_last_footing_height() << std::endl;
		ss << L"DX : " << player_->get_rigid_body()->getLinearVelocity().x() << std::endl;
		ss << L"DY : " << player_->get_rigid_body()->getLinearVelocity().y() << std::endl;
		ss << L"DZ : " << player_->get_rigid_body()->getLinearVelocity().z() << std::endl;
		ss << L"Objects : " << get_active_object_manager()->active_object_list().size() << std::endl;

		get_direct_3d()->getFont()->draw_text( 10.f, 10.f, get_app()->get_width() - 10.f, get_app()->get_height() - 10.f, ss.str().c_str(), Direct3D::Color( 1.f, 0.95f, 0.95f, 1.f ) );

		get_direct_3d()->getFont()->end();

		get_direct_3d()->end2D();
	}

	// render_3d()
	{
		if ( is_render_2d_enabled )
		{
			get_direct_3d()->begin3D();
		}

		XMVECTOR eye = XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 );
#if 1
		// render_object_for_shadow()
		// if ( rand() % 4 == 0 )
		if ( shadow_map_ )
		{
			/*
			static float a = 0.f;

			a += 0.01f;

			const XMVECTOR light_origin = XMVectorSet( 50.f, 100.f, -25.f, 0.f );
			XMVECTOR light = light_origin + XMVectorSet( cos( a ) * 10.f, 0.f, sin( a ) * 10.f, 0.f );	

			shadow_map_->setLightPosition( light );
			*/

			shadow_map_->setEyePosition( eye );
			shadow_map_->ready_to_render_shadow_map();

			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|shadow_map" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				for ( int n = 0; n < shadow_map_->get_cascade_levels(); n++ )
				{
					shadow_map_->ready_to_render_shadow_map_with_cascade_level( n );

					{
						get_game_main()->get_game_constant_buffer()->render();
						get_game_main()->get_frame_constant_buffer()->render();
					}

					for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
					{
						if ( ( *i )->is_dead() )
						{
							continue;
						}
						
						render( *i );
					}

					render( player_.get() );
					render( goal_.get() );
				}
			}

			// shadow_map_->finish_to_render_shadow_map();
		}
#endif

#if 1
		get_direct_3d()->clear();

		// update_view()
		{
			XMVECTOR at = XMVectorSet( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 0.0f );
			XMVECTOR up = XMVectorSet( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.0f );

			frame_constant_buffer.view = XMMatrixLookAtLH( eye, at, up );
			frame_constant_buffer.view = XMMatrixTranspose( frame_constant_buffer.view );
			frame_constant_buffer.time = get_total_elapsed_time();

			get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer );
		}

		// render_sky_box()
		{
			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sky_box" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				{
					get_game_main()->get_game_constant_buffer()->render();
					get_game_main()->get_frame_constant_buffer()->render();

					ObjectConstantBuffer buffer;
					buffer.world = XMMatrixTranslationFromVector( eye );
					buffer.world = XMMatrixTranspose( buffer.world );
					get_game_main()->get_object_constant_buffer()->update( & buffer );
					get_game_main()->get_object_constant_buffer()->render();
				}

				sky_box_->render();

				{
					ObjectConstantBuffer buffer;
					buffer.world = XMMatrixIdentity();

					get_game_main()->get_object_constant_buffer()->update( & buffer );
					get_game_main()->get_object_constant_buffer()->render();

					ground_->render();
				}
			}
		}

		// render_far_billboards()
		{
			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|billboard" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				get_game_main()->get_game_constant_buffer()->render();
				get_game_main()->get_frame_constant_buffer()->render();

				{
					ObjectConstantBuffer buffer;
					buffer.world = XMMatrixIdentity();
					
					get_game_main()->get_object_constant_buffer()->update( & buffer );
					get_game_main()->get_object_constant_buffer()->render();

					// 
					DrawingModel* drawing_model = get_drawing_model_manager()->load( "far-billboards-1" );
					drawing_model->get_mesh()->render();
				}
			}
		}

		// render_object();
		if ( shadow_map_ )
		{
			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|main_with_shadow" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				shadow_map_->ready_to_render_scene();

				{
					get_game_main()->get_game_constant_buffer()->render();
					get_game_main()->get_frame_constant_buffer()->render();
				}

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
				{
					if ( ( *i )->is_dead() )
					{
						continue;
					}

					render( *i );
				}

				render( goal_.get() );
			}
		}
		else
		{
			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|main" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				{
					get_game_main()->get_game_constant_buffer()->render();
					get_game_main()->get_frame_constant_buffer()->render();
				}

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
				{
					if ( ( *i )->is_dead() )
					{
						continue;
					}

					render( *i );
				}

				render( goal_.get() );
			}
		}

#if 1
		// render_object_line();
		{
			get_direct_3d()->setInputLayout( "line" );

			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|drawing_line" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();

				{
					get_game_main()->get_game_constant_buffer()->render();
					get_game_main()->get_frame_constant_buffer()->render();
				}

				for ( ActiveObjectManager::ActiveObjectList::const_iterator i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
				{
					if ( ( *i )->is_dead() )
					{
						continue;
					}

					render_line( *i );
				}

				render_line( player_.get() );
			}

			get_direct_3d()->setInputLayout( "main" );
		}
#endif

#if 1
		// render_bullet_debug();
		{
			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|bullet" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				get_game_main()->get_game_constant_buffer()->render();
				get_game_main()->get_frame_constant_buffer()->render();

				get_game_main()->get_bullet_debug_draw()->render();
			}
		}
#endif

		// render_text();
		if ( is_render_2d_enabled )
		{
			get_direct_3d()->renderText();
		}

#endif

#if 0
		// render_debug_shadow_map()
		{
			get_direct_3d()->setDebugViewport( 0.f, 0, get_width() / 4.f * shadow_map_->get_cascade_levels(), get_height() / 4.f );

			Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

			for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
			{
				( *i )->apply();
				
				( * rectangle_->get_material_list().begin() )->set_shader_resource_view( shadow_map_->getShaderResourceView() );
				rectangle_->render();
			}
		}
#endif

		render_fader();

		if ( is_render_2d_enabled )
		{
			get_direct_3d()->end3D();
		}
	}
}

void GamePlayScene::render( const ActiveObject* active_object )
{
	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBuffer buffer;

	buffer.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer.world = XMMatrixTranspose( buffer.world );

	get_game_main()->get_object_constant_buffer()->update( & buffer );
	get_game_main()->get_object_constant_buffer()->render();

	active_object->get_drawing_model()->get_mesh()->render();
}

void GamePlayScene::render_line( const ActiveObject* active_object )
{
	// @todo 効率化
	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBuffer buffer;

	buffer.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer.world = XMMatrixTranspose( buffer.world );
	buffer.color = active_object->get_drawing_model()->get_line()->get_color();

	get_game_main()->get_object_constant_buffer()->update( & buffer );
	get_game_main()->get_object_constant_buffer()->render();

	active_object->get_drawing_model()->get_line()->render(); // 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
}

} // namespace blue_sky
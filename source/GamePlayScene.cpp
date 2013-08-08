#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include "GameMain.h"

#include "App.h"

#include "Player.h"
#include "Girl.h"
#include "Goal.h"
#include "Robot.h"
#include "Balloon.h"
#include "Rocket.h"
#include "Umbrella.h"
#include "Medal.h"
#include "Stone.h"
#include "Switch.h"
#include "AreaSwitch.h"
#include "Ladder.h"
#include "Camera.h"


#include "DrawingModelManager.h"
#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include "AnimationPlayer.h"

#include "Direct3D11.h"
#include "Direct3D11SkyBox.h"
#include "Direct3D11ShadowMap.h"
#include "Direct3D11Rectangle.h"
#include "Direct3D11Axis.h"
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
#include "GraphicsManager.h"
#include "SoundManager.h"

#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "TranslationObject.h"

#include "Direct3D11.h"

#include "math.h"

#include <game/Sound.h>
#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <fstream>
#include <sstream>

#include "memory.h"

namespace blue_sky
{

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, ui_texture_( 0 )
	, is_cleared_( false )
	, action_bgm_after_timer_( 0.f )
	, bpm_( 120.f )
	, drawing_accent_scale_( 0.f )
	, light_position_( vector3( 0.25f, 1.f, -1.f ), 0.1f )
	, ambient_color_( Color( 1.f, 1.f, 1.f, 1.f ), 0.02f )
	, shadow_color_( Color( 0.f, 0.f, 0.f, 1.f ), 0.02f )
	, shadow_paper_color_( Color( 0.9f, 0.9f, 0.9f, 1.f ), 0.02f )
	, shading_enabled_( true )
	, balloon_sound_type_( BALLOON_SOUND_TYPE_MIX )
{
	stage_config_ = new Config();

	// Physics
	get_physics()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	// Texture
	ui_texture_ = get_direct_3d()->getTextureManager()->load( "ui", "media/image/item.png" );

	// Sound
	load_sound_all( get_stage_name() == "2-3" );

	get_graphics_manager()->setup_loader();

	// Player
	player_ = new Player();
	player_->set_drawing_model( get_drawing_model_manager()->load( "player" ) );
	get_active_object_manager()->name_active_object( "player", player_.get() );

	// Goal
	goal_ = new Goal();
	goal_->set_drawing_model( get_drawing_model_manager()->load( "goal" ) );
	get_active_object_manager()->name_active_object( "goal", goal_.get() );

	// Camera
	camera_ = new Camera();
	camera_->set_fov_default( get_config()->get( "camera.fov", 90.f ) );
	camera_->set_aspect( static_cast< float >( get_width() ) / static_cast< float >( get_height() ) );
	camera_->reset_fov();

	setup_command();

	if ( get_stage_name().empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "stage name is empty" );
	}

	std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );
	load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );

	if ( get_config()->get( "video.shadow-map-enabled", 1 ) != 0 && stage_config_->get( "video.shadow-map-enabled", true ) )
	{
		shadow_map_ = new ShadowMap( get_direct_3d(), get_config()->get( "video.shadow-map-cascade-levels", 3 ), get_config()->get( "video.shadow-map-size", 1024 ) );
	}

	if ( ! sky_box_ )
	{
		sky_box_ = new SkyBox( get_direct_3d(), "sky-box-3" );
	}

	if ( ! far_billboards_ )
	{
		far_billboards_ = new FarBillboardsMesh( get_direct_3d() );
		far_billboards_->load_obj( ( std::string( "media/model/stage-" ) + get_stage_name() + "-far-billboards.obj" ).c_str() );
	}

	rectangle_ = new Rectangle( get_direct_3d() );

	debug_axis_ = new Axis( get_direct_3d() );

	scope_mesh_ = get_graphics_manager()->create_mesh();
	scope_mesh_->load_obj( "media/model/scope.obj" );


	get_graphics_manager()->cleanup_loader();

	bgm_ = get_sound_manager()->get_sound( "bgm" );

	if ( bgm_ )
	{
		bgm_->play( stage_config_->get( "bgm.loop", true ) );
	}

	paper_texture_list_.push_back( get_direct_3d()->getTextureManager()->load( "paper-0", "media/texture/pencil-face-1.png" ) );
	paper_texture_list_.push_back( get_direct_3d()->getTextureManager()->load( "paper-1", "media/texture/pen-face-1-loop.png" ) );
	paper_texture_list_.push_back( get_direct_3d()->getTextureManager()->load( "paper-2", "media/texture/pen-face-2-loop.png" ) );
	paper_texture_list_.push_back( get_direct_3d()->getTextureManager()->load( "paper-3", "media/texture/dot-face-1.png" ) );
	paper_texture_list_.push_back( get_direct_3d()->getTextureManager()->load( "paper-4", "media/texture/brush-face-1.png" ) );
	paper_texture_ = paper_texture_list_.front();

	update_render_data_for_game();

	restart();
}

GamePlayScene::~GamePlayScene()
{
	get_drawing_model_manager()->clear();
	get_active_object_manager()->clear();

	get_direct_3d()->getMeshManager()->unload_all();
	get_direct_3d()->getTextureManager()->unload_all();

	get_physics()->clear();
}

/**
 * コマンドを準備する
 *
 */
void GamePlayScene::setup_command()
{
	auto set_color = [] ( Color& color, const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		float_t r = 0.f, g = 0.f, b = 0.f, a = 0.f;
		ss >> color.r() >> color.g() >> color.b() >> color.a();
	};

	command_map_[ "set_line_type" ] = [ & ] ( const string_t& s )
	{
		drawing_line_type_index_ = common::deserialize< int >( s );
		drawing_line_type_index_ = math::clamp< int >( drawing_line_type_index_, 0, DrawingLine::LINE_TYPE_MAX - 1 );
	};
	command_map_[ "set_paper_type" ] = [ & ] ( const string_t& s )
	{
		int paper_texture_index = common::deserialize< int >( s );
		paper_texture_index = math::clamp< int >( paper_texture_index, 0, paper_texture_list_.size() - 1 );
		paper_texture_ = paper_texture_list_[ paper_texture_index ];
	};
	command_map_[ "set_balloon_sound_type" ] = [ & ] ( const string_t& s )
	{
		balloon_sound_type_ = static_cast< BalloonSoundType >( common::deserialize< int_t >( s ) );
	};
	command_map_[ "set_drawing_accent_scale" ] = [ & ] ( const string_t& s )
	{
		drawing_accent_scale_ = common::deserialize< float_t >( s );
	};
	command_map_[ "set_shading_enabled" ] = [ & ] ( const string_t& s )
	{
		shading_enabled_ = common::deserialize< bool >( s );
	};
	command_map_[ "set_ambient_color_target" ] = [ & ] ( const string_t& s )
	{
		set_color( ambient_color_.target_value(), s );
	};
	command_map_[ "set_ambient_color" ] = [ & ] ( const string_t& s )
	{
		set_color( ambient_color_.target_value(), s );
		ambient_color_.chase_full();
	};
	command_map_[ "set_shadow_color_target" ] = [ & ] ( const string_t& s )
	{
		set_color( shadow_color_.target_value(), s );
	};
	command_map_[ "set_shadow_color" ] = [ & ] ( const string_t& s )
	{
		set_color( shadow_color_.target_value(), s );
		shadow_color_.chase_full();
	};
	command_map_[ "set_shadow_paper_color_target" ] = [ & ] ( const string_t& s )
	{
		set_color( shadow_paper_color_.target_value(), s );
	};
	command_map_[ "set_shadow_paper_color" ] = [ & ] ( const string_t& s )
	{
		set_color( shadow_paper_color_.target_value(), s );
		shadow_paper_color_.chase_full();
	};
	command_map_[ "set_light_position_target" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		float_t x = 0.f, y = 0.f, z = 0.f;
		ss >> x >> y >> z;
		light_position_.target_value().set( x, y, z );
	};
	command_map_[ "set_light_position" ] = [ & ] ( const string_t& s )
	{
		command_map_[ "set_light_position_target" ]( s );
		light_position_.value() = light_position_.target_value();
	};
	command_map_[ "change_sky_box" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t sky_box_name;
		string_t sky_box_ext;

		ss >> sky_box_name >> sky_box_ext;

		sky_box_.release();

		if ( sky_box_name != "none" )
		{
			sky_box_ = new SkyBox( get_direct_3d(), ( string_t( "sky-box-" ) + sky_box_name ).c_str(), sky_box_ext.c_str() );
		}
	};
	command_map_[ "change_bgm" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		bgm_ = get_sound_manager()->load_music( "bgm", s.c_str(), true );
		bgm_->play( stage_config_->get( "bgm.loop", true ) );
	};
	command_map_[ "change_bpm" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		float_t bpm;
		float_t beat = 1;

		ss >> bpm >> beat;

		set_bpm( bpm * beat );
	};
	command_map_[ "load_sound" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t sound_name, sound_file_name;

		ss >> sound_name >> sound_file_name;

		Sound* sound = get_sound_manager()->load( sound_name.c_str(), ( sound_file_name.empty() ? sound_name : sound_file_name ).c_str() );
	};
	command_map_[ "play_sound" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t sound_name;
		bool force = true, loop = false;

		ss >> sound_name >> force >> loop;

		Sound* sound = get_sound_manager()->load( sound_name.c_str() );

		if ( sound )
		{
			sound->play( loop, force );
		}
	};
	command_map_[ "stop_sound" ] = [ & ] ( const string_t& s )
	{
		Sound* sound = get_sound_manager()->load( s.c_str() );

		if ( sound )
		{
			sound->stop();
		}
	};
	command_map_[ "game_object.create" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		get_active_object_manager()->create_object( ss, get_drawing_model_manager(), get_physics() );
	};
	command_map_[ "game_object.set_mass" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t mass = 0.f;

		ss >> object_name >> mass;

		GameObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->set_mass( mass );
		}
	};
	command_map_[ "game_object.set_gravity" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t x = 0.f, y = 0.f, z = 0.f;

		ss >> object_name >> x >> y >> z;

		GameObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->set_gravity( GameObject::Vector3( x, y, z  ) );
		}
	};
	command_map_[ "game_object.set_angular_factor" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t x = 0.f, y = 0.f, z = 0.f;

		ss >> object_name >> x >> y >> z;

		GameObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->set_angular_factor( GameObject::Vector3( x, y, z  ) );
		}
	};
	command_map_[ "game_object.set_target_location" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t x = 0.f, y = 0.f, z = 0.f, speed = 0.f;
		ss >> object_name >> x >> y >> z >> speed;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );

		if ( o )
		{
			get_active_object_manager()->set_target_location( o, GameObject::Vector3( x, y, z ), speed );
		}
	};
	command_map_[ "game_object.set_target_direction" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t r = 0.f, speed = 0.f;

		ss >> object_name >> r >> speed;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );

		if ( o )
		{
			get_active_object_manager()->set_target_direction( o, r, speed );
		}
	};
	command_map_[ "game_object.set_target_direction_object" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name, target_object_name;
		float_t speed = 0.f;

		ss >> object_name >> target_object_name >> speed;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );
		ActiveObject* to = get_active_object_manager()->get_active_object( target_object_name );

		if ( o && to )
		{
			get_active_object_manager()->set_target_direction_object( o, to, speed );
		}
	};
	command_map_[ "game_object.set_flicker_scale" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t scale = 1.f;

		ss >> object_name >> scale;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );

		if ( o  )
		{
			o->set_flicker_scale( scale );
		}
	};
	command_map_[ "game_object.play_animation" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name, animation_name;
		bool force = true, loop = false;

		ss >> object_name >> animation_name >> force >> loop;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->play_animation( animation_name.c_str(), force, loop );
		}
	};
	command_map_[ "game_object.set_animation_speed" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		float_t speed;

		ss >> object_name >> speed;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o && o->get_animation_player() )
		{
			o->get_animation_player()->set_speed( speed );
		}
	};
	command_map_[ "game_object.action" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name, action;
		ss >> object_name >> action;

		ActiveObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->action( action );
		}
	};
	command_map_[ "player.start_flickering" ] = [ & ] ( const string_t& )
	{
		player_->start_flickering();
	};
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
	get_direct_3d()->getFader()->set_color( Direct3D::Color::White );
	get_direct_3d()->getFader()->full_out();

	is_cleared_ = false;
	action_bgm_after_timer_ = 0.f;

	drawing_line_type_index_ = 0;
	paper_texture_ = paper_texture_list_[ 0 ];

	player_->restart();
	goal_->restart();

	camera_->restart();

	get_active_object_manager()->restart();

	setup_stage();

	play_sound( "restart" );
}

void GamePlayScene::load_sound_all( bool is_final_stage )
{
	if ( is_final_stage )
	{
		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load( "fin", "girl-see-you" );
		// get_sound_manager()->load( "door" );
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

	ActiveObject* last_object = 0;
	BaseSwitch* last_base_switch = 0;

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
			bool loop = true;

			ss >> name;
			ss >> loop;


			get_sound_manager()->load_music( "bgm", name.c_str() );

			if ( ss.good() )
			{
				float_t bpm = get_bpm();

				ss >> bpm;

				set_bpm( bpm );
			}
		}
		else if ( name == "collision" )
		{
			std::string file_name;

			ss >> file_name;

			get_physics()->load_obj( ( std::string( "media/stage/" ) + file_name ).c_str() );
		}
		else if ( name == "player" )
		{
			float_t x = 0, y = 0, z = 0;

			ss >> x >> y >> z;

			player_->set_start_location( x, y, z );
			player_->set_rigid_body( get_physics()->add_active_object_as_capsule( player_.get() ) );

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
		else if ( name == "ground" )
		{
			string_t ground_name;
			ss >> ground_name;

			if ( ! ground_ )
			{
				ground_ = get_graphics_manager()->create_mesh();				
			}
			
			ground_->load_obj( ( string_t( "media/model/" ) + ground_name + ".obj" ).c_str() );
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
			last_object = get_active_object_manager()->create_object( ss, get_drawing_model_manager(), get_physics() );
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

			last_object = object;
		}
		else if ( name == "girl" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			girl_ = new Girl();
			girl_->set_player( player_.get() );
			girl_->set_drawing_model( get_drawing_model_manager()->load( "girl" ) );
			girl_->set_rigid_body( get_physics()->add_active_object( girl_.get() ) );
			girl_->set_start_location( x, y, z );
			girl_->set_start_direction_degree( r );
			girl_->setup_animation_player();

			get_active_object_manager()->add_active_object( girl_.get() );
			get_active_object_manager()->name_active_object( "girl", girl_.get() );

			last_object = girl_.get();
		}
		else if ( name == "robot" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			Robot* robot = new Robot();
			robot->set_player( player_.get() );

			robot->set_drawing_model( get_drawing_model_manager()->load( "robot" ) );
			robot->set_rigid_body( get_physics()->add_active_object( robot ) );
			robot->set_start_location( x, y, z );
			robot->set_start_direction_degree( r );
			robot->setup_animation_player();

			get_active_object_manager()->add_active_object( robot );

			last_object = robot;
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
		else if ( name == "ladder" )
		{
			active_object = new Ladder();
			active_object->set_drawing_model( get_drawing_model_manager()->load( "ladder-5" ) );
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
			active_object->set_mass( 0 );
		}
		else if ( name == "rocket" )
		{
			active_object = new Rocket();
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
		}
		else if ( name == "umbrella" )
		{
			active_object = new Umbrella();
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
		}
		else if ( name == "stone" )
		{
			active_object = new Stone();
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );
		}
		else if ( name == "switch" )
		{
			Switch* s = new Switch();

			active_object = s;
			active_object->set_rigid_body( get_physics()->add_active_object( active_object ) );

			last_base_switch = s;
			last_object = s;
		}
		else if ( name == "area-switch" )
		{
			float_t x = 0.f, y = 0.f, z = 0.f;
			float_t w = 0.f, h = 0.f, d = 0.f;
			float_t r = 0.f;

			ss >> x >> y >> z >> w >> h >> d >> r;

			AreaSwitch* s = new AreaSwitch( w, h, d );
			DrawingModel* drawing_model = get_drawing_model_manager()->load( name.c_str() );
			s->set_drawing_model( drawing_model );

			s->set_rigid_body( get_physics()->add_active_object( s ) );
			s->set_start_location( x, y, z );
			s->set_start_direction_degree( r );

			get_active_object_manager()->add_active_object( s );

			last_base_switch = s;
			last_object = s;
		}
		else if ( name == "event" || name == "exec" )
		{
			string_t event_name;
			string_t event_handler_name;
			string_t event_handler_params;

			if ( name == "event" )
			{
				ss >> event_name;
			}

			ss >> event_handler_name;

			while ( ss.good() )
			{
				string_t s;
				ss >> s;

				if ( ! event_handler_params.empty() )
				{
					event_handler_params += " ";
				}

				event_handler_params += s;
			}

			auto i = command_map_.find( event_handler_name );

			if ( i == command_map_.end() )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( ( "event handler not found. ( " ) + event_handler_name + " )" );
			}

			auto command_call = [ = ] { i->second( event_handler_params ); };

			if ( name == "event" )
			{
				last_base_switch->get_event_handler( event_name ).push_back( command_call );
			}
			else
			{
				stage_setup_command_call_list_.push_back( command_call );
			}
		}
		else if ( name == "name" )
		{
			if ( ! last_object )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( "naming object not found." );
			}

			string_t object_name;

			ss >> object_name;
			get_active_object_manager()->name_active_object( object_name, last_object );
		}
		else
		{
			/// @todo コマンドに置き換える？
			stage_config_->read_line( line );
		}

		if ( active_object )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			if ( ! active_object->get_drawing_model() )
			{
				DrawingModel* drawing_model = get_drawing_model_manager()->load( name.c_str() );
				active_object->set_drawing_model( drawing_model );
			}

			active_object->setup_animation_player();

			active_object->set_start_location( x, y, z );
			active_object->set_start_direction_degree( r );
			
			get_active_object_manager()->add_active_object( active_object );

			last_object = active_object;
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
		// get_physics()->update( 1.f / get_main_loop()->get_fps() );
		get_physics()->update( get_elapsed_time() );

		player_->update_transform();
		goal_->update_transform();

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
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
	
	get_physics()->check_collision_all();

	get_sound_manager()->set_listener_position( camera_->position() );
	// get_sound_manager()->set_listener_velocity( player_->get_velocity() );
	get_sound_manager()->set_listener_orientation( camera_->front(), camera_->up() );
	get_sound_manager()->commit();

	update_balloon_sound();
	update_shadow();

	light_position_.chase();
	ambient_color_.chase();
	shadow_color_.chase();
	shadow_paper_color_.chase();
}

void GamePlayScene::update_main()
{
	const float_t rotation_speed_rate = camera_->fov() / camera_->get_fov_default();

	camera_->rotate_degree_target().x() += get_input()->get_mouse_dy() * 90.f * rotation_speed_rate;
	camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );
	player_->set_pitch( -camera_->rotate_degree_target().x() / 90.f );

	float eye_depth_add = 0.f;

	if ( player_->is_on_ladder() )
	{
		eye_depth_add = -0.05f;
	}
	else if ( camera_->rotate_degree_target().x() > 0.f )
	{
		eye_depth_add = get_input()->get_mouse_dy();
	}

	player_->add_eye_depth( eye_depth_add );

	if ( ! player_->is_dead() )
	{
		bool is_moving = false;

		if ( ! player_->is_ladder_step_only() )
		{
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
		}

		bool is_moving_on_ladder = false;

		if ( player_->is_on_ladder() )
		{
			if ( get_input()->press( Input::UP ) )
			{
				player_->ladder_step( +1.f );
				is_moving_on_ladder = true;
			}
			if ( get_input()->press( Input::DOWN ) )
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

		if ( get_input()->push( Input::A ) )
		{
			switch ( player_->get_selected_item_type() )
			{
				case Player::ITEM_TYPE_NONE: player_->jump(); break;
				case Player::ITEM_TYPE_ROCKET: player_->rocket( Player::Vector3( camera_->front().x(), camera_->front().y(), camera_->front().z() ) ); break;
				case Player::ITEM_TYPE_STONE: player_->throw_stone( Player::Vector3( camera_->front().x(), camera_->front().y(), camera_->front().z() ) ); break;
				// case Player::ITEM_TYPE_UMBRELLA: player_->start_umbrella_mode(); player_->jump(); break;
				case Player::ITEM_TYPE_SCOPE:
				{
					player_->switch_scope_mode();

					if ( player_->get_action_mode() != Player::ACTION_MODE_SCOPE )
					{
						camera_->reset_fov();
					}
					else
					{
						camera_->set_fov_target( camera_->get_fov_default() * 0.5f );
						camera_->set_fov( camera_->get_fov_default() * 0.5f );
					}
					break;
				}
			}
		}
		else if ( get_input()->push( Input::JUMP ) )
		{
			player_->jump();
		}
		else if ( get_input()->press( Input::A ) || get_input()->press( Input::JUMP ) )
		{
			player_->clamber();
		}
		else
		{
			player_->stop_clamber();
		}

		int wheel = get_input()->pop_mouse_wheel_queue();

		if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
		{
			if ( wheel > 0 )
			{
				camera_->set_fov_target( std::max( camera_->get_fov_target() * 0.5f, 5.f ) );
			}
			else if ( wheel < 0 )
			{
				camera_->set_fov_target( std::min( camera_->get_fov_target() * 2.f, camera_->get_fov_default() * 0.5f ) );
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
		}

		player_->add_direction_degree( get_input()->get_mouse_dx() * 90.f * rotation_speed_rate);
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
			get_direct_3d()->getFader()->fade_out( 0.25f );
		}
	}
}

void GamePlayScene::update_balloon_sound()
{
	int_t balloon_sound_request = player_->pop_balloon_sound_request();

	if ( balloon_sound_request >= 1 )
	{
		if ( balloon_sound_type_ == BALLOON_SOUND_TYPE_MIX || balloon_sound_type_ == BALLOON_SOUND_TYPE_SOLO )
		{
			Sound* sound = get_sound_manager()->get_sound( "balloon" );

			if ( sound )
			{
				sound->play( false );
			}
		}
		else if ( balloon_sound_type_ == BALLOON_SOUND_TYPE_SCALE )
		{
			// int r = math::clamp( balloon_sound_request, 1, 7 );
			int r = balloon_sound_request % 7 + 1;

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

	if ( player_->get_action_mode() == Player::ACTION_MODE_BALLOON && balloon_sound_type_ == BALLOON_SOUND_TYPE_SOLO || balloon_sound_type_ == BALLOON_SOUND_TYPE_SCALE )
	{
		action_bgm_after_timer_ = 2.f;
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

		/*
		if ( balloon_bgm_->is_playing() )
		{
			balloon_bgm_->fade_in( Sound::VOLUME_FADE_SPEED_FAST );
		}
		else
		{
			balloon_bgm_->play( true );
			balloon_bgm_->set_volume( Sound::VOLUME_MAX );
		}
		*/
	}
	else
	{
		if ( balloon_bgm_ )
		{
			balloon_bgm_->fade_out();
		}

		if ( bgm_ )
		{
			/*
			if ( bgm_->is_fader_full_out() && stage_config_->get( "bgm.loop", true ) )
			{
				bgm_->play( true, false );
			}
			*/

			bgm_->fade_in();
		}
	}

	// 関数分ける
	if ( player_->get_balloon() )
	{
		// かくかくするのでここでやる
		const_cast< Balloon* >( player_->get_balloon() )->set_location( player_->get_location() + player_->get_front() * 0.25f - player_->get_right() * 0.25f + GameObject::Vector3( 0, 1.75f, 0 ) );
		const_cast< Balloon* >( player_->get_balloon() )->set_direction_degree( 0 );
	}
}

/**
 * 影関連の情報を更新する
 *
 */
void GamePlayScene::update_shadow()
{
	if ( shadow_map_ )
	{
		if ( false )
		{
			static float a = 0.1f;

			a += 0.0025f;

			const Vector light_origin( 0.f, 100.f, 0.f );
			const Vector v = light_origin + Vector( cos( a ) * 50.f, 0.f, sin( a ) * 50.f, 0.f );	

			light_position_.value().set( v.x(), v.y(), v.z() );
			light_position_.target_value() = light_position_.value();
		}

		shadow_map_->set_light_position( Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z() ) );
		shadow_map_->set_eye_position( Vector( camera_->position().x(), camera_->position().y(), camera_->position().z() ) );
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
		get_sound_manager()->get_sound( "door" ) && ! get_sound_manager()->get_sound( "door" )->is_playing() )
	{
		get_sound_manager()->get_sound( "door" )->play( false );
	}
	
	if ( ! get_sound_manager()->get_sound( "fin" )->is_playing() )
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
}

/**
 * 描画
 *
 */
void GamePlayScene::render()
{
	get_direct_3d()->setInputLayout( "main" );
	
	// render_text()
	if ( get_direct_3d()->getFont() )
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

		get_direct_3d()->setInputLayout( "main" );

		render_sky_box();
		render_far_billboards();

		render_object_mesh();

		get_direct_3d()->setInputLayout( "skin" );
		render_object_skin_mesh();

		get_direct_3d()->setInputLayout( "line" );

		render_object_line();

		// render_debug_axis();

		render_debug_bullet();

		get_direct_3d()->setInputLayout( "main" );

		render_sprite();

		get_direct_3d()->renderText();

		render_fader();
		
		// render_debug_shadow_map_window();

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
		ss << L"DX : " << player_->get_velocity().x() << std::endl;
		ss << L"DY : " << player_->get_velocity().y() << std::endl;
		ss << L"DZ : " << player_->get_velocity().z() << std::endl;
		ss << L"VELOCITY : " << player_->get_velocity().length() << std::endl;
		ss << L"Objects : " << get_active_object_manager()->active_object_list().size() << std::endl;

		ss << L"mouse.dx : " << get_input()->get_mouse_dx() << std::endl;
		ss << L"mouse.dy : " << get_input()->get_mouse_dy() << std::endl;

		ss << L"IS JUMPING : " << player_->is_jumping() << std::endl;
		ss << L"ON FOOTING : " << player_->is_on_footing() << std::endl;
		ss << L"ON LADDER : " << player_->is_on_ladder() << std::endl;
		ss << L"IS FACING TO BLOCK : " << player_->is_facing_to_block() << std::endl;
		ss << L"CAN CLAMBER : " << player_->can_clamber() << std::endl;
		ss << L"CAN PEER DOWN : " << player_->can_peer_down() << std::endl;
		ss << L"CAN THROW : " << player_->can_throw() << std::endl;
		ss << L"IS CLAMBERING : " << player_->is_clambering() << std::endl;
		ss << L"IS FALLING TO DIE : " << player_->is_falling_to_die() << std::endl;
		ss << L"IS FALLING TO SAFE : " << player_->is_falling_to_safe() << std::endl;

		ss << L"IS LADDER STEP ONLY : " << player_->is_ladder_step_only() << std::endl;

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
		frame_constant_buffer_data.projection = XMMatrixPerspectiveFovLH( math::degree_to_radian( camera_->fov() ), camera_->aspect(), 0.05f, 3000.f );
		frame_constant_buffer_data.projection = XMMatrixTranspose( frame_constant_buffer_data.projection );
		frame_constant_buffer_data.light = -Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z() );
		frame_constant_buffer_data.light.normalize();
		frame_constant_buffer_data.time = get_total_elapsed_time();
		frame_constant_buffer_data.time_beat = static_cast< uint_t >( get_total_elapsed_time() * ( get_bpm() / 60.f ) );

		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}

	/// @todo 整理する
	{
		FrameDrawingConstantBufferData frame_drawing_constant_buffer_data;

		frame_drawing_constant_buffer_data.accent = bgm_ ? bgm_->get_current_peak_level() * drawing_accent_scale_ : 0.f;
		frame_drawing_constant_buffer_data.line_type = drawing_line_type_index_;
		frame_drawing_constant_buffer_data.shadow_color = shadow_color_.value();
		frame_drawing_constant_buffer_data.shadow_paper_color = shadow_paper_color_.value();

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
	if ( ! active_object->is_visible() )
	{
		return;
	}

	const btTransform& trans = active_object->get_transform();

	XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

	ObjectConstantBufferData buffer_data;

	buffer_data.world = XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
	buffer_data.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
	buffer_data.world = XMMatrixTranspose( buffer_data.world );

	if ( active_object->get_drawing_model()->get_line() )
	{
		buffer_data.color = active_object->get_drawing_model()->get_line()->get_color();
	}

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
	get_game_main()->get_game_constant_buffer()->bind_to_ps();
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

	shadow_map_->ready_to_render_shadow_map();

	get_direct_3d()->setInputLayout( "main" );
	render_shadow_map( "|shadow_map", false );

	// @todo 最適化・高速化
	get_direct_3d()->setInputLayout( "skin" );
	render_shadow_map( "|shadow_map_skin", true );

	// shadow_map_->finish_to_render_shadow_map();
}

/**
 * シャドウマップを描画する
 *
 * @param technique_name テクニック名
 * @param is_skin_mesh スキンメッシュフラグ
 */
void GamePlayScene::render_shadow_map( const char* technique_name, bool is_skin_mesh ) const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( technique_name );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		for ( int n = 0; n < shadow_map_->get_cascade_levels(); n++ )
		{
			shadow_map_->ready_to_render_shadow_map_with_cascade_level( n );

			bind_game_constant_buffer();
			bind_frame_constant_buffer();

			get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs(); // for line

			for ( auto j = get_active_object_manager()->active_object_list().begin(); j != get_active_object_manager()->active_object_list().end(); ++j )
			{
				if ( ( *j )->get_drawing_model()->is_skin_mesh() == is_skin_mesh )
				{
					render_active_object_mesh( *j );

					if ( ( *j )->get_drawing_model()->get_line() && ( *j )->get_drawing_model()->get_line()->is_cast_shadow() )
					{
						render_active_object_line( *j );
					}
				}
			}

			if ( ! is_skin_mesh )
			{
				render_active_object_mesh( player_.get() );
				render_active_object_mesh( goal_.get() );
			}
		}
	}
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
		if ( sky_box_ )
		{
			object_constant_buffer_data.world = XMMatrixTranslationFromVector( XMVectorSet( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1 ) );
			object_constant_buffer_data.world = XMMatrixTranspose( object_constant_buffer_data.world );
			object_constant_buffer_data.color = ambient_color_.value();

			get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );

			sky_box_->render();
		}

		// ground
		if ( ground_ )
		{
			object_constant_buffer_data.world = XMMatrixIdentity();
			object_constant_buffer_data.color = ambient_color_.value();

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
	const char* technique_name = "|skin";

	if ( shading_enabled_ )
	{
		if ( shadow_map_ )
		{
			technique_name = "|skin_with_shadow";
		}
	}
	else
	{
		technique_name ="|skin_flat";
	}

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( technique_name );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
		
		bind_game_constant_buffer();
		bind_frame_constant_buffer();

		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs();
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_ps();
		get_direct_3d()->bind_texture_to_ps( 2, paper_texture_ );

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
	const char* technique_name = "|main";

	if ( shading_enabled_ )
	{
		if ( shadow_map_ )
		{
			technique_name = "|main_with_shadow";
		}
	}
	else
	{
		technique_name ="|main_flat";
	}

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( technique_name );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		bind_game_constant_buffer();
		bind_frame_constant_buffer();
		
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_gs();
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_ps();
		get_direct_3d()->bind_texture_to_ps( 2, paper_texture_ );

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
	active_object->render_mesh();
}

/**
 * ActiveObject の線を描画する
 *
 * @param active_object ActiveObject
 */
void GamePlayScene::render_active_object_line( const ActiveObject* active_object ) const
{
	if ( ! active_object->is_visible() )
	{
		return;
	}

	if ( ! active_object->get_drawing_model()->get_line() )
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
	if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
	{
		get_direct_3d()->setInputLayout( "main" );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = XMMatrixOrthographicLH( camera_->aspect() * 2.f, 2.f, 0.f, 1.f );
		buffer_data.world = XMMatrixTranspose( buffer_data.world );

		get_game_main()->get_object_constant_buffer()->update( & buffer_data );

		auto technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

		for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
		{
			( *i )->apply();
			
			get_game_main()->get_object_constant_buffer()->bind_to_vs();
			get_game_main()->get_object_constant_buffer()->bind_to_ps();

			scope_mesh_->render();
		}
	}

	get_direct_3d()->getSprite()->begin();

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

	for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		if ( player_->get_selected_item_type() == Player::ITEM_TYPE_ROCKET )
		{
			for ( int n = 0; n < player_->get_item_count( Player::ITEM_TYPE_ROCKET ); n++ )
			{
				const int offset = n * 20;

				win::Rect src_rect = win::Rect::Size( 0, 0, 202, 200 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - offset - 5 );

				get_direct_3d()->getSprite()->draw( dst_point, ui_texture_, src_rect.get_rect() );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ITEM_TYPE_UMBRELLA )
		{
			for ( int n = 0; n < player_->get_item_count( Player::ITEM_TYPE_UMBRELLA ); n++ )
			{
				const float offset = n * 50.f;

				win::Rect src_rect = win::Rect::Size( 0, 256, 186, 220 );
				Vector center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

				Matrix t;
				t.set_translation( get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );

				get_direct_3d()->getSprite()->set_transform( t );
				get_direct_3d()->getSprite()->draw( ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ITEM_TYPE_STONE )
		{
			for ( int n = player_->get_item_count( Player::ITEM_TYPE_STONE ) - 1; n >= 0; --n )
			{
				const int offset = n * 50;

				win::Rect src_rect = win::Rect::Size( 256, 96, 128, 96 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 - offset );
				
				get_direct_3d()->getSprite()->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ITEM_TYPE_SCOPE )
		{
			win::Rect src_rect = win::Rect::Size( 256, 256, 192, 140 );
			win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 );

			get_direct_3d()->getSprite()->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
		}

		if ( player_->get_selected_item_type() == Player::ITEM_TYPE_ROCKET || ( player_->get_selected_item_type() == Player::ITEM_TYPE_STONE && player_->can_throw() ) )
		{
			// aim
			win::Rect src_rect = win::Rect::Size( 256, 0, 76, 80 );
			win::Point dst_point( ( get_width() - src_rect.width() ) / 2, ( get_height() - src_rect.height() ) / 2 );
			
			get_direct_3d()->getSprite()->draw( ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.5f ) );
		}

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
 * デバッグ用の軸を表示する
 *
 */
void GamePlayScene::render_debug_axis() const
{
	/*
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|simple_line" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		bind_game_constant_buffer();
		bind_frame_constant_buffer();

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			( *i )->get_object_constant_buffer()->bind_to_vs();
			( *i )->get_object_constant_buffer()->bind_to_ps();

			debug_axis_->render();
		}
	}
	*/

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|simple_line" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		bind_game_constant_buffer();
		bind_frame_constant_buffer();

		for ( auto i = get_active_object_manager()->active_object_list().begin(); i != get_active_object_manager()->active_object_list().end(); ++i )
		{
			const ActiveObject* active_object = *i;

			if ( active_object->is_dead() )
			{
				continue;
			}

			if ( ! ( *i )->get_animation_player() )
			{
				continue;
			}

			( *i )->get_object_constant_buffer()->bind_to_vs();
			( *i )->get_animation_player()->bind_render_data();

			AnimationPlayer::BoneConstantBuffer::Data bone_constant_buffer_data;
			active_object->get_animation_player()->calculate_bone_matrix_recursive( bone_constant_buffer_data, 0, Matrix::identity() );

			for ( uint_t n = 0; n < ( *i )->get_animation_player()->get_skinning_animation_set()->get_bone_count(); ++n )
			{
				const Matrix& bone_offset_matrix = ( *i )->get_animation_player()->get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( n );
				const Matrix& bone_matrix = bone_offset_matrix * bone_constant_buffer_data.bone_matrix[ n ];
				// const Matrix& bone_matrix = bone_offset_matrix;

				const btTransform& trans = active_object->get_transform();

				XMFLOAT4 q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

				ObjectConstantBufferData buffer_data;

				buffer_data.world = XMMatrixScaling( 0.1f, 0.1f, 0.1f );
				buffer_data.world *= bone_matrix;
				buffer_data.world *= XMMatrixRotationQuaternion( XMLoadFloat4( & q ) );
				buffer_data.world *= XMMatrixTranslation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
				buffer_data.world = XMMatrixTranspose( buffer_data.world );

				if ( active_object->get_drawing_model()->get_line() )
				{
					buffer_data.color = active_object->get_drawing_model()->get_line()->get_color();
				}

				active_object->get_object_constant_buffer()->update( & buffer_data );

				debug_axis_->render();
			}
		}
	}
}

/**
 * Bullet のデバッグ表示を描画する
 *
 */
void GamePlayScene::render_debug_bullet() const
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
void GamePlayScene::render_debug_shadow_map_window() const
{
	if ( ! shadow_map_ )
	{
		return;
	}

	get_direct_3d()->setDebugViewport( 0.f, 0, get_width() / 4.f * shadow_map_->get_cascade_levels(), get_height() / 4.f );

	ObjectConstantBufferData buffer_data;
	buffer_data.world = XMMatrixIdentity();
	buffer_data.color = Color( 0.5f, 0.f, 0.f, 0.f );
	get_game_main()->get_object_constant_buffer()->update( & buffer_data );

	auto technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
		
		bind_object_constant_buffer();

		Texture t( get_direct_3d(), shadow_map_->getShaderResourceView() );
		rectangle_->get_material_list().front()->set_texture( & t );
		rectangle_->render();
	}
}

} // namespace blue_sky

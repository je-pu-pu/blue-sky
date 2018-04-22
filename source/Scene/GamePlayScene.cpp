#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include "GameMain.h"

#include "App.h"

/// @todo ActiveObjectManager �ֈړ�
#include <GameObject/Player.h>
#include <GameObject/Girl.h>
#include <GameObject/Goal.h>
#include <GameObject/Robot.h>
#include <GameObject/Balloon.h>
#include <GameObject/AreaSwitch.h>
#include <GameObject/TranslationObject.h>

/// @todo �����ȃN���X�𐮗�����
#include <Camera.h>
#include <AnimationPlayer.h>

/// @todo GraphicsManager ? �Ɉڍs����
#include "DrawingModelManager.h"
#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

/// @todo ���ۉ�����
#include <core/graphics/Direct3D11/Direct3D11ShadowMap.h>
#include <core/graphics/Direct3D11/Direct3D11SkyBox.h>
#include <core/graphics/Direct3D11/Direct3D11FarBillboardsMesh.h>
#include <core/graphics/Direct3D11/Direct3D11Rectangle.h>
#include <core/graphics/Direct3D11/Direct3D11Sprite.h>
#include <core/graphics/Direct3D11/Direct3D11Material.h>
#include <core/graphics/Direct3D11/Direct3D11BulletDebugDraw.h>

#include "ActiveObjectPhysics.h"

#include "ConstantBuffer.h"

#include "Input.h"
#include "GraphicsManager.h"
#include "SoundManager.h"

#include "ScriptManager.h"

#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "OculusRift.h"

#include "DelayedCommand.h"

#include <core/math.h>

#include <game/Sound.h>
#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <iostream>
#include <iomanip>

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
	, drawing_accent_scale_( 0.f )
	, light_position_( Vector3( 0.25f, 1.f, -1.f ), 0.1f )
	, ambient_color_( Color( 1.f, 1.f, 1.f, 1.f ), 0.02f )
	, shadow_color_( Color( 0.f, 0.f, 0.f, 1.f ), 0.02f )
	, shadow_paper_color_( Color( 0.9f, 0.9f, 0.9f, 1.f ), 0.02f )
	, shading_enabled_( true )
	, balloon_sound_type_( BALLOON_SOUND_TYPE_MIX )
	, is_blackout_( false )
	, blackout_timer_( 0.f )
{
	stage_config_ = new Config();

	// Physics
	get_physics()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	// Texture
	ui_texture_ = get_graphics_manager()->load_texture( "ui", "media/image/item.png" );

	// Sound
	load_sound_all( get_stage_name() == "2-3" );

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
	camera_->reset_fov();
	camera_->set_aspect( static_cast< float >( get_width() ) / static_cast< float >( get_height() ) );

	setup_command();

	if ( get_stage_name().empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "stage name is empty" );
	}

	std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );
	load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );

	if ( get_config()->get( "graphics.shadow-map-enabled", 1 ) != 0 && stage_config_->get( "graphics.shadow-map-enabled", true ) )
	{
		shadow_map_ = new ShadowMap( get_direct_3d(), get_config()->get( "graphics.shadow-map-cascade-levels", 3 ), get_config()->get( "graphics.shadow-map-size", 1024 ) );
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

	scope_mesh_ = get_graphics_manager()->create_mesh();
	scope_mesh_->load_obj( "media/model/scope.obj" );

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
	get_direct_3d()->unset_render_target();

	get_drawing_model_manager()->clear();
	get_active_object_manager()->clear();

	get_graphics_manager()->unload_texture_all();

	get_physics()->clear();
	
	clear_delayed_command();

	get_direct_3d()->getSprite()->set_ortho_offset( 0.f );
}

void GamePlayScene::clear_delayed_command()
{
	for ( auto i = delayed_command_list_.begin(); i != delayed_command_list_.end(); ++i )
	{
		delete *i;
	}

	delayed_command_list_.clear();
}

/**
 * �I�u�W�F�N�g�𐶐�����
 *
 * @param class_name �N���X��
 * @return ���������I�u�W�F�N�g
 * @todo ��������B loc, rot �̎w����ǂ����邩�H StaticObject �̐������ǂ����邩�H
 */
ActiveObject* GamePlayScene::create_object_at_player_front( const char_t* class_name )
{
	// get_script_manager()->exec( string_t( "active_object_ = create_object( '" ) + class_name + "' )" );
	// ActiveObject* active_object = get_script_manager()->get< ActiveObject* >( "active_object_" );

	ActiveObject* active_object = GameMain::get_instance()->create_object( class_name );

	if ( ! active_object )
	{
		return 0;
	}

	active_object->set_start_location( player_->get_location().x(), player_->get_location().y(), player_->get_location().z() + 3.f );
	active_object->set_start_direction_degree( 0.f );

	active_object->restart();

	return active_object;
}

/**
 * �R�}���h����������
 *
 * @todo Script �Ɉڍs����
 */
void GamePlayScene::setup_command()
{
	auto set_color = [] ( Color& color, const string_t& s )
	{
		std::stringstream ss;
		ss << s;
		ss >> color.r() >> color.g() >> color.b() >> color.a();
	};

	auto set_color_target = [] ( common::chase_value< Color, float_t >& color, const string_t& s )
	{
		std::stringstream ss;
		ss << s;
		ss >> color.target_value().r() >> color.target_value().g() >> color.target_value().b() >> color.target_value().a() >> color.speed();
	};

	get_script_manager()->set_function( "create_object_at_player_front", [this] ( const char_t* name ) { return create_object_at_player_front( name ); } );

	command_map_[ "set_line_type" ] = [ this ] ( const string_t& s )
	{
		get_graphics_manager()->set_drawing_line_type( common::deserialize< int >( s ) );
	};
	command_map_[ "set_paper_type" ] = [ & ] ( const string_t& s )
	{
		get_graphics_manager()->set_paper_texture_type( common::deserialize< int >( s ) );
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
		set_color_target( ambient_color_, s );
	};
	command_map_[ "set_ambient_color" ] = [ & ] ( const string_t& s )
	{
		set_color( ambient_color_.target_value(), s );
		ambient_color_.chase_full();
	};
	command_map_[ "set_shadow_color_target" ] = [ & ] ( const string_t& s )
	{
		set_color_target( shadow_color_, s );
	};
	command_map_[ "set_shadow_color" ] = [ & ] ( const string_t& s )
	{
		set_color( shadow_color_.target_value(), s );
		shadow_color_.chase_full();
	};
	command_map_[ "set_shadow_paper_color_target" ] = [ & ] ( const string_t& s )
	{
		set_color_target( shadow_paper_color_, s );
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
		bool replace = false;

		ss >> sound_name >> sound_file_name >> replace;

		Sound* current_sound = get_sound_manager()->load( sound_name.c_str() );

		if ( replace && current_sound )
		{
			get_sound_manager()->unload( sound_name.c_str() );
		}

		get_sound_manager()->load( sound_name.c_str(), ( sound_file_name.empty() ? sound_name : sound_file_name ).c_str() );
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
	command_map_[ "stop_all_sound" ] = [ & ] ( const string_t& )
	{
		get_sound_manager()->stop_all();
	};
	command_map_[ "game_object.create" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		get_active_object_manager()->create_static_object( ss, get_drawing_model_manager(), get_physics() );
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
	command_map_[ "game_object.set_kinematic" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		bool is_kinematic = false;

		ss >> object_name >> is_kinematic;

		GameObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->set_kinematic( is_kinematic );
		}
	};
	command_map_[ "game_object.set_no_contact_response" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		string_t object_name;
		bool is_no_contact_response = false;

		ss >> object_name >> is_no_contact_response;

		GameObject* o = get_active_object_manager()->get_active_object( object_name );
		
		if ( o )
		{
			o->set_no_contact_response( is_no_contact_response );
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
		ss >> object_name;

		while ( ss.good() )
		{
			string_t p;
			ss >> p;

			if ( ! action.empty() )
			{
				action += " ";
			}

			action += p;
		}

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
	command_map_[ "timer" ] = [ & ] ( const string_t& s )
	{
		std::stringstream ss;
		ss << s;

		float_t interval = 1.f;
		int_t count = 1;
		string_t command;
		
		ss >> interval >> count;

		while ( ss.good() )
		{
			string_t p;
			ss >> p;

			if ( ! command.empty() )
			{
				command+= " ";
			}

			command += p;
		}

		delayed_command_list_.push_back( new DelayedCommand( interval, count, command ) );
	};
	command_map_[ "end_by_last_switch" ] = [ & ] ( const string_t& )
	{
		bgm_->stop();
		get_sound_manager()->stop_all();

		Sound* sound = get_sound_manager()->get_sound( "switch-off" );
		sound->play( false, true );

		is_blackout_ = true;
	};
}

/**
 * �X�e�[�W�̐ݒ�ɉ����āA�X�e�[�W����������
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

	player_->restart();
	goal_->restart();

	camera_->restart();

	get_active_object_manager()->restart();

	clear_delayed_command();
	setup_stage();

	play_sound( "restart" );

	get_script_manager()->exec( "a = 0" );
	get_script_manager()->exec( "set_drawing_line_type( a )" );
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
 * �X�e�[�W�t�@�C����ǂݍ���
 *
 * @param file_name �X�e�[�W�t�@�C����
 * @todo ��������
 */
void GamePlayScene::load_stage_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load stage file \"" ) + file_name + "\" failed." );
	}

	ActiveObject* last_object = 0;

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		std::stringstream ss;
		
		std::string name;
		std::string value;
		
		ss << line;

		ss >> name;

		if ( name == "#" )
		{
			//
		}
		else if ( name == "bgm" )
		{
			std::string bgm_name;
			bool loop = true;

			ss >> bgm_name;
			ss >> loop;


			get_sound_manager()->load_music( "bgm", bgm_name.c_str() );

			if ( ss.good() )
			{
				float_t bpm = get_bpm();

				ss >> bpm;

				set_bpm( bpm );
			}
		}
		else if ( name == "collision" )
		{
			std::string collision_file_name;

			ss >> collision_file_name;

			get_physics()->load_obj( ( StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) ) + collision_file_name ).c_str() );
		}
		else if ( name == "player" )
		{
			float_t x = 0, y = 0, z = 0;

			ss >> x >> y >> z;

			player_->set_start_location( x, y, z );
			player_->set_rigid_body( get_physics()->add_active_object_as_capsule( player_.get() ) );

			if ( ! ss.eof() )
			{
				float r = 0.f;

				ss >> r;
				
				camera_->rotate_degree_target().set_x( r );
				camera_->rotate_degree().set_x( r );
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
			last_object = get_active_object_manager()->create_static_object( ss, get_drawing_model_manager(), get_physics() );
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
		else if ( name == "balloon" || name == "medal" || name == "ladder" || name == "rocket" || name == "umbrella" || name == "stone" || name == "switch" )
		{
			ActiveObject* active_object = GameMain::get_instance()->create_object( name.c_str() );
			
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			active_object->set_start_location( x, y, z );
			active_object->set_start_direction_degree( r );

			last_object = active_object;
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
				last_object->add_event_handler( event_name.c_str(), command_call );
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
			/// @todo �R�}���h�ɒu��������H
			stage_config_->read_line( line );
		}
	}
}

void GamePlayScene::exec_command( const string_t& command )
{
	std::stringstream ss;
	ss << command;

	string_t command_name;
	string_t command_params;

	ss >> command_name;

	while ( ss.good() )
	{
		string_t s;
		ss >> s;

		if ( ! command_params.empty() )
		{
			command_params += " ";
		}

		command_params += s;
	}

	auto i = command_map_.find( command_name );

	if ( i == command_map_.end() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( ( "command not found. ( " ) + command_name + " )" );
	}

	i->second( command_params );
}

void GamePlayScene::save_stage_file( const char* ) const
{

}

/**
 * ���C�����[�v����
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

	update_delayed_command();
	update_balloon_sound();
	update_shadow();

	light_position_.chase();
	ambient_color_.chase();
	shadow_color_.chase();
	shadow_paper_color_.chase();
}

/**
 * �ʏ펞�̍X�V����
 *
 */
void GamePlayScene::update_main()
{
	if ( get_input()->push( Input::A ) )
	{
		get_script_manager()->exec( "a = a + 1 set_drawing_line_type( a )" );
	}
	if ( get_input()->push( Input::B ) )
	{
		get_script_manager()->exec( "a = a - 1 set_drawing_line_type( a )" );
	}

	const float_t rotation_speed_rate = camera_->fov() / camera_->get_fov_default();

	if ( get_oculus_rift() )
	{
		// camera_->rotate_degree_target().x() = math::radian_to_degree( get_oculus_rift()->get_pitch() );
		// camera_->rotate_degree_target().y() = math::radian_to_degree( get_oculus_rift()->get_yaw() );
		// camera_->rotate_degree_target().z() = math::radian_to_degree( get_oculus_rift()->get_roll() );
	}
	else
	{
		camera_->rotate_degree_target().x() += get_input()->get_mouse_dy() * 90.f * rotation_speed_rate;
		camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );
	}

	player_->set_pitch( -camera_->rotate_degree_target().x() / 90.f );

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

					if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
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

			camera_->reset_fov();
		}

		float_t add_direction_degree_by_mouse = get_input()->get_mouse_dx() * 90.f * rotation_speed_rate;
		float_t add_direction_degree_by_hmd   = get_oculus_rift() ? math::radian_to_degree( get_oculus_rift()->get_delta_yaw() ) : 0.f;
		
		player_->add_direction_degree( add_direction_degree_by_mouse + add_direction_degree_by_hmd );
		camera_->rotate_degree_target().y() = player_->get_direction_degree();

		if ( player_->is_falling_to_die() )
		{
			// �����Ď��̂��Ƃ��Ă���ꍇ�͔����t�F�[�h�A�E�g����

			get_graphics_manager()->set_fade_color( Color( 1.f, 1.f, 1.f, 0.5f ) );
			get_graphics_manager()->fade_out( 0.01f );
		}
		else
		{
			get_graphics_manager()->fade_in( 0.05f );
		}
	}
	else
	{
		if ( get_input()->push( Input::A ) )
		{
			restart();
		}
		else
		{
			get_graphics_manager()->set_fade_color( Color( 0.25f, 0.f, 0.f, 0.75f ) );
			get_graphics_manager()->fade_out( 0.1f );
		}
	}
}

void GamePlayScene::update_blackout()
{
	blackout_timer_ += get_elapsed_time();

	get_graphics_manager()->set_fade_color( Color::Black );
	get_graphics_manager()->fade_out();

	if ( blackout_timer_ >= 6.f )
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
			delete *i;
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
			int r = math::clamp( balloon_sound_request, 1, 7 );
			// int r = ( balloon_sound_request - 1 ) % 7 + 1;

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

	if ( player_->get_action_mode() == Player::ACTION_MODE_BALLOON && ( balloon_sound_type_ == BALLOON_SOUND_TYPE_SOLO || balloon_sound_type_ == BALLOON_SOUND_TYPE_SCALE ) )
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

	/// @todo ��������
	// �Ԃ��̂ł����ł��B
	if ( player_->get_balloon() )
	{
		const_cast< Balloon* >( player_->get_balloon() )->update();
	}
}

/**
 * �e�֘A�̏����X�V����
 *
 */
void GamePlayScene::update_shadow()
{
	if ( shadow_map_ )
	{
		if ( true )
		{
			static float a = 0.1f;

			a += 0.0025f;

			const Vector light_origin( 0.f, 50.f, 0.f );
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

	get_graphics_manager()->fade_out( 0.0025f );

	if (
		get_sound_manager()->get_sound( "fin" )->get_current_position() >= 6.f &&
		get_sound_manager()->get_sound( "fin" )->get_current_position() <= 8.f &&
		get_sound_manager()->get_sound( "door" ) && ! get_sound_manager()->get_sound( "door" )->is_playing() )
	{
		get_sound_manager()->get_sound( "door" )->play( false );
	}
	
	if ( ! get_sound_manager()->get_sound( "fin" )->is_playing() )
	{
		go_to_next_scene();
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

/**
 * �`��
 *
 */
void GamePlayScene::render()
{
	render_to_oculus_vr();
	render_to_display();

	render_text();
}

/**
 * Oculus Rift �ɑ΂��ĕ`����s��
 *
 */
void GamePlayScene::render_to_oculus_vr() const
{
	if ( ! get_oculus_rift() )
	{
		return;
	}

	update_render_data_for_frame_drawing();
	update_render_data_for_object();

	render_shadow_map();

	get_oculus_rift()->setup_rendering();

	get_oculus_rift()->setup_rendering_for_left_eye();
	update_render_data_for_frame_for_eye( 0 );
			
	render_for_eye();

	get_oculus_rift()->setup_rendering_for_right_eye();
	update_render_data_for_frame_for_eye( 1 );

	render_for_eye();

	get_oculus_rift()->finish_rendering();
}

/**
 * �ʏ�̃f�B�X�v���C�ɑ΂��ĕ`����s��
 *
 */
void GamePlayScene::render_to_display() const
{
	update_render_data_for_frame();
	update_render_data_for_frame_drawing();
	update_render_data_for_object();

	render_shadow_map();

	get_direct_3d()->clear_default_view();

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();
	render_for_eye();
}

/**
 * �e�ڂɑ΂��ĕ`����s��
 *
 * @parma ortho_offset UI �̗��ڊԃI�t�Z�b�g
 */
void GamePlayScene::render_for_eye( float_t ortho_offset ) const
{
	get_graphics_manager()->set_input_layout( "main" );

	render_sky_box();
	render_far_billboards();

	render_object_mesh();

	get_graphics_manager()->set_input_layout( "skin" );

	render_object_skin_mesh();

	// debug
	// get_direct_3d()->clear_default_view();

	get_graphics_manager()->set_input_layout( "line" );

	render_object_line();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	get_graphics_manager()->set_input_layout( "main" );

	render_sprite( ortho_offset );

	render_fader();
		
	// render_debug_shadow_map_window();
}

void GamePlayScene::render_text() const
{
	std::stringstream ss;
	ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

	if ( get_game_main()->is_display_fps() )
	{
		ss << "FPS : " << get_main_loop()->get_last_fps() << std::endl;
		ss << "BPM : " << get_bpm() << std::endl;
		ss << "POS : " << player_->get_transform().getOrigin().x() << ", " << player_->get_transform().getOrigin().y() << ", " << player_->get_transform().getOrigin().z() << std::endl;
		ss << "step speed : " << player_->get_step_speed() << std::endl;
		ss << "last footing height : " << player_->get_last_footing_height() << std::endl;
		ss << "DX : " << player_->get_velocity().x() << std::endl;
		ss << "DY : " << player_->get_velocity().y() << std::endl;
		ss << "DZ : " << player_->get_velocity().z() << std::endl;
		ss << "VELOCITY : " << player_->get_velocity().length() << std::endl;
		ss << "Objects : " << get_active_object_manager()->active_object_list().size() << std::endl;

		ss << "mouse.dx : " << get_input()->get_mouse_dx() << std::endl;
		ss << "mouse.dy : " << get_input()->get_mouse_dy() << std::endl;

		/*
		ss << "IS JUMPING : " << player_->is_jumping() << std::endl;
		ss << "ON FOOTING : " << player_->is_on_footing() << std::endl;
		ss << "ON LADDER : " << player_->is_on_ladder() << std::endl;
		ss << "IS FACING TO BLOCK : " << player_->is_facing_to_block() << std::endl;
		ss << "CAN CLAMBER : " << player_->can_clamber() << std::endl;
		ss << "CAN PEER DOWN : " << player_->can_peer_down() << std::endl;
		ss << "CAN THROW : " << player_->can_throw() << std::endl;
		ss << "IS CLAMBERING : " << player_->is_clambering() << std::endl;
		ss << "IS FALLING TO DIE : " << player_->is_falling_to_die() << std::endl;
		ss << "IS FALLING TO SAFE : " << player_->is_falling_to_safe() << std::endl;

		ss << "IS LADDER STEP ONLY : " << player_->is_ladder_step_only() << std::endl;

		ss << "BALLOON : " << ( player_->get_balloon() != nullptr ) << std::endl;

		if ( player_->get_balloon() )
		{
			ss << "BALLOON : " << player_->get_balloon()->is_visible() << std::endl;
			ss << "BALLOON : " << player_->get_balloon()->is_mesh_visible() << std::endl;
			ss << "BALLOON : " << player_->get_balloon()->is_line_visible() << std::endl;
		}
		*/

		if ( get_oculus_rift() )
		{
			ss << "YAW : " << get_oculus_rift()->get_yaw() << std::endl;
			ss << "PITCH : " << get_oculus_rift()->get_pitch() << std::endl;
			ss << "ROLL : " << get_oculus_rift()->get_roll() << std::endl;

			ss << "DELTA YAW : " << get_oculus_rift()->get_delta_yaw() << std::endl;
		}
	}

	get_graphics_manager()->draw_text( 10.f, 10.f, get_direct_3d()->get_width() - 10.f, get_direct_3d()->get_height() - 10.f, ss.str().c_str(), Direct3D::Color( 1.f, 0.95f, 0.95f, 1.f ) );
}

/**
 * �t���[�����ɍX�V����K�v�̂���`��p�̒萔�o�b�t�@���X�V����
 *
 * @todo GraphicsManager �� ActiveObjectManager �� Scenegraph �Ɉړ�����

 */
void GamePlayScene::update_render_data_for_frame() const
{
	// get_graphic_manager()->update_render_data_for_frame( camera_ );

	FrameConstantBufferData frame_constant_buffer_data;
	update_frame_constant_buffer_data_sub( frame_constant_buffer_data );

	Vector eye( camera_->position().x(), camera_->position().y(), camera_->position().z() );
	Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z() );
	Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z() );

	frame_constant_buffer_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_constant_buffer_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() ).transpose();

	get_graphics_manager()->get_frame_render_data()->update( & frame_constant_buffer_data );
}

/**
 * �t���[�����ɍX�V����K�v�̂���`��p�̒萔�o�b�t�@���X�V����
 *
 * @param int eye_index 0 : ���� / 1 : �E��
 */
void GamePlayScene::update_render_data_for_frame_for_eye( int eye_index ) const
{
	FrameConstantBufferData frame_constant_buffer_data;
	update_frame_constant_buffer_data_sub( frame_constant_buffer_data );

	Matrix camera_rot;
	camera_rot.set_rotation_xyz(
		math::degree_to_radian( camera_->rotate_degree().x() ),
		math::degree_to_radian( camera_->rotate_degree().y() ),
		math::degree_to_radian( camera_->rotate_degree().z() ) );

	Vector eye_offset = ( get_oculus_rift()->get_eye_position( eye_index ) ) * camera_rot;

	// std::cout << std::fixed << std::setprecision( 8 );
	// std::cout << "eye" << eye_index << " offset : " << eye_offset.x() << ", " << eye_offset.y() << ", " << eye_offset.z() << std::endl;

	Matrix r = get_oculus_rift()->get_eye_rotation( eye_index ) * camera_rot;
	Vector eye = Vector( camera_->position().x(), camera_->position().y(), camera_->position().z() ) + eye_offset;
	// Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z() );
	// Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z() );
	Vector at = eye + Vector( 0.f, 0.f, 1.f ) * r;
	Vector up = Vector( 0.f, 1.f, 0.f ) * r;

	frame_constant_buffer_data.view = Matrix().set_look_at( eye, at, up ).transpose();
	frame_constant_buffer_data.projection = get_oculus_rift()->get_projection_matrix( eye_index, camera_->near_clip(), camera_->far_clip() ).transpose();

	get_graphics_manager()->get_frame_render_data()->update( & frame_constant_buffer_data );
}

/**
 * �t���[�����ɍX�V����K�v�̂���`��p�̒萔�o�b�t�@�p�f�[�^�̂����A�}�g���b�N�X�ȊO�̃f�[�^�����X�V����
 *
 */
void GamePlayScene::update_frame_constant_buffer_data_sub( FrameConstantBufferData& frame_constant_buffer_data ) const
{
	frame_constant_buffer_data.light = -Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z() );
	frame_constant_buffer_data.light.normalize();
	frame_constant_buffer_data.time = get_total_elapsed_time();
	frame_constant_buffer_data.time_beat = static_cast< uint_t >( get_total_elapsed_time() * ( get_bpm() / 60.f ) );
}
/**
 * �t���[�����ɍX�V����K�v�̂���`��p�̒萔�o�b�t�@�̂����A�菑�����`��Ɋւ���萔�o�b�t�@�X�V����
 *
 */
void GamePlayScene::update_render_data_for_frame_drawing() const
{
	/// @todo GraphicsManager ���Ǘ�����
	get_graphics_manager()->set_shadow_color( shadow_color_.value() );
	get_graphics_manager()->set_shadow_paper_color( shadow_paper_color_.value() );
	get_graphics_manager()->set_drawing_accent( bgm_ ? bgm_->get_current_peak_level() * drawing_accent_scale_ : 0.f );

	get_graphics_manager()->get_frame_drawing_render_data()->update();
}

/**
 * �S�ẴI�u�W�F�N�g�̕`��p�̒萔�o�b�t�@���X�V����
 *
 */
void GamePlayScene::update_render_data_for_object() const
{
	for ( const auto& active_object : get_active_object_manager()->active_object_list() )
	{
		active_object->update_render_data();
	}

	player_->update_render_data();
	goal_->update_render_data();
}

/**
 * �V���h�E�}�b�v��`�悷��
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

	// @todo �œK���E������
	get_direct_3d()->setInputLayout( "skin" );
	render_shadow_map( "|shadow_map_skin", true );

	// shadow_map_->finish_render_shadow_map();
}

/**
 * �V���h�E�}�b�v��`�悷��
 *
 * @param technique_name �e�N�j�b�N��
 * @param is_skin_mesh �X�L�����b�V���t���O
 */
void GamePlayScene::render_shadow_map( const char* technique_name, bool is_skin_mesh ) const
{
	render_technique( technique_name, [this, is_skin_mesh]
	{
		for ( int n = 0; n < shadow_map_->get_cascade_levels(); n++ )
		{
			shadow_map_->ready_to_render_shadow_map_with_cascade_level( n );

			bind_game_render_data();
			bind_frame_render_data();

			get_graphics_manager()->get_frame_drawing_render_data()->bind_to_gs(); // for line

			for ( const auto* active_object : get_active_object_manager()->active_object_list() )
			{
				if ( active_object->get_drawing_model()->is_skin_mesh() == is_skin_mesh )
				{
					active_object->render_mesh();

					if ( active_object->get_drawing_model()->get_line() && active_object->get_drawing_model()->get_line()->is_cast_shadow() )
					{
						active_object->render_line();
					}
				}
			}

			if ( ! is_skin_mesh )
			{
				player_->render_mesh();
				goal_->render_mesh();
			}
		}
	} );
}


/**
 * �X�J�C�{�b�N�X��`�悷��
 *
 */
void GamePlayScene::render_sky_box() const
{
	render_technique( "|sky_box", [this]
	{
		bind_game_render_data();
		bind_frame_render_data();
		bind_shared_object_render_data();

		ObjectConstantBufferData object_constant_buffer_data;

		// sky box
		if ( sky_box_ )
		{
			object_constant_buffer_data.world = Matrix().set_translation( camera_->position().x(), camera_->position().y(), camera_->position().z() ).transpose();
			object_constant_buffer_data.color = ambient_color_.value();

			get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );

			sky_box_->render();
		}

		// ground
		if ( ground_ )
		{
			object_constant_buffer_data.world.set_identity();
			object_constant_buffer_data.color = ambient_color_.value();

			get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
	
			ground_->render();
		}
	} );
}

/**
 * ���i�r���{�[�h���b�V����`�悷��
 *
 */
void GamePlayScene::render_far_billboards() const
{
	if ( ! far_billboards_ )
	{
		return;
	}

	render_technique( "|billboard", [this]
	{
		{
			ObjectConstantBufferData buffer;
			buffer.color = Color::White;
			buffer.world.set_identity();
					
			get_graphics_manager()->get_shared_object_render_data()->update( & buffer );
		}

		bind_game_render_data();
		bind_frame_render_data();
		bind_shared_object_render_data();

		far_billboards_->render();
	} );
}

/**
 * �I�u�W�F�N�g�̃X�L�����b�V����`�悷��
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

	render_technique( technique_name, [this]
	{
		bind_game_render_data();
		bind_frame_render_data();

		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_gs();
		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_ps();
		get_graphics_manager()->bind_paper_texture();

		if ( shadow_map_ )
		{
			shadow_map_->ready_to_render_scene();
		}

		for ( const auto* active_object : get_active_object_manager()->active_object_list() )
		{
			if ( active_object->get_drawing_model()->is_skin_mesh() )
			{
				active_object->render_mesh();
			}
		}
	} );
}

/**
 * �I�u�W�F�N�g�̃��b�V����`�悷��
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

	render_technique( technique_name, [this]
	{
		bind_game_render_data();
		bind_frame_render_data();
		
		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_gs();
		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_ps();
		get_graphics_manager()->bind_paper_texture();

		if ( shadow_map_ )
		{
			shadow_map_->ready_to_render_scene();
		}

		for ( const auto* active_object : get_active_object_manager()->active_object_list() )
		{
			if ( ! active_object->get_drawing_model()->is_skin_mesh() )
			{
				active_object->render_mesh();
			}
		}

		goal_->render_mesh();
	} );
}

/**
 * �I�u�W�F�N�g�̐���`�悷��
 *
 */
void GamePlayScene::render_object_line() const
{
	render_technique( "|drawing_line", [this]
	{
		bind_game_render_data();
		bind_frame_render_data();
		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_gs();

		for ( const auto* active_object : get_active_object_manager()->active_object_list() )
		{
			active_object->render_line();
		}

		player_->render_line();
	} );
}

/**
 * 2D �X�v���C�g��`�悷��
 *
 */
void GamePlayScene::render_sprite( float_t ortho_offset ) const
{
	if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
	{
		get_direct_3d()->setInputLayout( "main" );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = Matrix().set_orthographic( camera_->aspect() * 2.f, 2.f, 0.f, 1.f );
		buffer_data.world *= Matrix().set_translation( ortho_offset, 0.f, 0.f );
		buffer_data.world = buffer_data.world.transpose();

		get_graphics_manager()->get_shared_object_render_data()->update( & buffer_data );

		render_technique( "|main2d", [this]
		{
			bind_shared_object_render_data();

			scope_mesh_->render();
		} );
	}

	get_direct_3d()->getSprite()->set_ortho_offset( ortho_offset * 20.f );

	get_direct_3d()->getSprite()->begin();

	render_technique( "|sprite", [this]
	{
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
	} );

	get_direct_3d()->getSprite()->end();
}

/**
 * �V���h�E�}�b�v�̃f�o�b�O�E�B���h�E��`�悷��
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
	buffer_data.world.set_identity();
	buffer_data.color = Color( 0.5f, 0.f, 0.f, 0.f );
	get_graphics_manager()->get_shared_object_render_data()->update( & buffer_data );

	render_technique( "|main2d", [this]
	{
		bind_shared_object_render_data();

		rectangle_->get_material_list().front()->set_texture( shadow_map_->getTexture() );
		rectangle_->render();
	} );
}

} // namespace blue_sky
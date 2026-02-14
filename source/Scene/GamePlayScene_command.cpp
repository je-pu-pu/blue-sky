#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include <blue_sky/GameMain.h>

/// @todo ActiveObjectManager へ移動
#include <GameObject/Player.h>
#include <GameObject/Girl.h>
#include <GameObject/Goal.h>
#include <GameObject/Robot.h>
#include <GameObject/Balloon.h>
#include <GameObject/AreaSwitch.h>
#include <GameObject/TranslationObject.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>

#include <core/animation/AnimationPlayer.h>
#include <core/sound/SoundManager.h>

#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ScriptManager.h>

#include <blue_sky/DelayedCommand.h>

#include <game/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/math.h>

#include <fstream>
#include <sstream>

namespace blue_sky
{

/**
 * オブジェクトを生成する
 *
 * @param class_name クラス名
 * @return 生成したオブジェクト
 * @todo 整理する。 loc, rot の指定をどうするか？ StaticObject の生成をどうするか？
 */
ActiveObject* GamePlayScene::create_object_at_player_front( const char_t* class_name )
{
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
 * コマンドを準備する
 *
 * @todo Script に移行する
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

	get_script_manager()->set_function( "create_object_at_player_front", [this] ( const char_t* class_name ) { return create_object_at_player_front( class_name ); } );

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

		assert( sky_box_ext != "png" );

		if ( sky_box_name == "none" )
		{
			get_graphics_manager()->unset_sky_box();
		}
		else
		{
			get_graphics_manager()->set_sky_box( ( string_t( "sky-box-" ) + sky_box_name ).c_str() );
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

		get_active_object_manager()->create_static_object( ss );
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
			o->set_gravity( Vector( x, y, z  ) );
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
			o->set_angular_factor( Vector( x, y, z  ) );
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
			get_active_object_manager()->set_target_location( o, Vector( x, y, z ), speed );
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

		delayed_command_list_.push_back( std::make_unique< DelayedCommand >( interval, count, command ) );
	};
	command_map_[ "end_by_last_switch" ] = [ & ] ( const string_t& )
	{
		if ( bgm_ )
		{
			bgm_->stop();
		}
		get_sound_manager()->stop_all();

		if ( Sound* sound = get_sound_manager()->get_sound( "switch-off" ) )
		{
			sound->play( false, true );
		}

		is_blackout_ = true;
	};
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
 * ステージファイルを読み込む
 *
 * @param file_name ステージファイル名
 * @todo 整理する
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
		std::string command;

		ss << line;
		ss >> command;

		if ( command == "#" )
		{
			//
		}
		else if ( command == "bgm" )
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
		else if ( command == "collision" )
		{
			std::string collision_file_name;

			ss >> collision_file_name;

			get_active_object_physics()->load_obj( ( StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) ) + collision_file_name ).c_str() );
		}
		else if ( command == "player" )
		{
			float_t x = 0, y = 0, z = 0;

			ss >> x >> y >> z;

			player_->set_start_location( x, y, z );
			player_->set_rigid_body( get_active_object_physics()->add_active_object_as_capsule( player_ ) );

			if ( ! ss.eof() )
			{
				float r = 0.f;

				ss >> r;

				camera_->rotate_degree_target().set_x( r );
				camera_->rotate_degree().set_x( r );
			}
		}
		else if ( command == "goal" )
		{
			float_t x = 0, y = 0, z = 0;
			ss >> x >> y >> z;

			goal_->set_start_location( x, y, z );
		}
		else if ( command == "ground" )
		{
			string_t ground_name;
			ss >> ground_name;

			get_graphics_manager()->set_ground( ground_name.c_str() );
		}
		else if ( command == "far-billboards" )
		{
			std::string far_billboards_name;

			ss >> far_billboards_name;

			/// @todo 直す
			far_billboards_ = get_graphics_manager()->load_model( far_billboards_name.c_str() );
		}
		else if ( command == "object" || command == "static-object" || command == "dynamic-object" )
		{
			last_object = get_active_object_manager()->create_static_object( ss );
		}
		else if ( command == "translation-object" )
		{
			float_t x = 0, y = 0, z = 0, tw = 0, th = 0, td = 0, s = 0.01f;
			ss >> x >> y >> z >> tw >> th >> td >> s;

			TranslationObject* object = new TranslationObject( 5, 5, 5, tw, th, td, s );

			Model* model = get_graphics_manager()->load_model( "box-5x5x5" );

			object->set_model( model );
			object->set_rigid_body( get_active_object_physics()->add_active_object_as_box( object ) );
			object->set_start_location( x, y, z );

			get_active_object_manager()->add_active_object( object );

			last_object = object;
		}
		else if ( command == "girl" || command == "robot" || command == "balloon" || command == "medal" || command == "ladder" || command == "rocket" || command == "umbrella" || command == "stone" || command == "switch" )
		{
			ActiveObject* active_object = GameMain::get_instance()->create_object( command.c_str() );

			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			active_object->set_start_location( x, y, z );
			active_object->set_start_direction_degree( r );

			if ( command == "girl" )
			{
				girl_ = static_cast< Girl* >( active_object );
				girl_->set_player( player_ );
				get_active_object_manager()->name_active_object( "girl", girl_ );
			}
			else if ( command == "robot" )
			{
				Robot* robot = static_cast< Robot* >( active_object );
				robot->set_player( player_ );
			}

			last_object = active_object;
		}
		else if ( command == "area-switch" )
		{
			float_t x = 0.f, y = 0.f, z = 0.f;
			float_t w = 0.f, h = 0.f, d = 0.f;
			float_t r = 0.f;

			ss >> x >> y >> z >> w >> h >> d >> r;

			AreaSwitch* s = new AreaSwitch( w, h, d );
			s->set_rigid_body( get_active_object_physics()->add_active_object_as_box( s ) );
			s->set_start_location( x, y, z );
			s->set_start_direction_degree( r );

			get_active_object_manager()->add_active_object( s );

			last_object = s;
		}
		else if ( command == "event" || command == "exec" )
		{
			string_t event_name;
			string_t event_handler_name;
			string_t event_handler_params;

			if ( command == "event" )
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

			if ( command == "event" )
			{
				last_object->add_event_handler( event_name.c_str(), command_call );
			}
			else
			{
				stage_setup_command_call_list_.push_back( command_call );
			}
		}
		else if ( command == "name" )
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
	}
}

} // namespace blue_sky

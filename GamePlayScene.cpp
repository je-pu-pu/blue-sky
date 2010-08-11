#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include "App.h"

#include "Player.h"
#include "Goal.h"
#include "Enemy.h"
#include "Balloon.h"
#include "Rocket.h"
#include "Umbrella.h"
#include "Medal.h"
#include "Poison.h"
#include "Camera.h"
#include "Stage.h"

#include "Direct3D9Font.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9SkyBox.h"
#include "Direct3D9Box.h"
#include "Direct3D9Rectangle.h"
#include "Direct3D9.h"
#include "Direct3D9MeshManager.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include "Input.h"
#include "SoundManager.h"
#include "GridDataManager.h"
#include "GridObjectManager.h"
#include "GridObject.h"
#include "GridCell.h"
#include "ActiveObjectManager.h"

#include "matrix4x4.h"
#include "vector3.h"

#include <game/Sound.h>
#include <game/Config.h>

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
	, back_buffer_texture_( 0 )
	, back_buffer_surface_( 0 )
	, depth_surface_( 0 )
	, ui_texture_( 0 )
	, brightness_( 1.f )
	, is_cleared_( false )
	, grid_object_visible_length_( 500 )
	, grid_object_lod_0_length_( 100 )
	, lens_type_( LENS_TYPE_NORMAL )
{
	ambient_color_[ 0 ] = 1.f;
	ambient_color_[ 1 ] = 1.f;
	ambient_color_[ 2 ] = 1.f;
	ambient_color_[ 3 ] = 1.f;

	grid_object_visible_length_ = config()->get( "video.grid-object-visible-length", 500.f );
	grid_object_lod_0_length_ = config()->get( "video.grid-object-lod-0-length", 100.f );

	// Texture
	ui_texture_ = direct_3d()->getTextureManager()->load( "ui", "media/image/item.png" );

	// Mesh
//	player_mesh_ = new Direct3D9Mesh( direct_3d() );
//	player_mesh_->load_x( "media/model/player.x" );

	goal_mesh_ = new Direct3D9Mesh( direct_3d() );
	goal_mesh_->load_x( "media/model/goal.x" );

	shadow_mesh_ = new Direct3D9Mesh( direct_3d() );
	shadow_mesh_->load_x( "media/model/shadow.x" );

	ground_mesh_ = new Direct3D9Mesh( direct_3d() );
	ground_mesh_->load_x( "media/model/ground.x" );

	scope_mesh_ = new Direct3D9Mesh( direct_3d() );
	scope_mesh_->load_x( "media/model/scope.x" );

	balloon_mesh_ = direct_3d()->getMeshManager()->load( "balloon", "media/model/balloon.x" );
	umbrella_mesh_ = direct_3d()->getMeshManager()->load( "umbrella", "media/model/umbrella.x" );
	rocket_mesh_ = direct_3d()->getMeshManager()->load( "rocket", "media/model/rocket.x" );
	aim_mesh_ = direct_3d()->getMeshManager()->load( "aim", "media/model/aim.x" );

	// SkyBox
	// sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box-3", "png" );

	// Box
	box_ = new Direct3D9Box( direct_3d(), 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Sound
	{
		sound_manager()->stop_all();
		sound_manager()->unload_all();

		sound_manager()->load_3d_sound( "enemy" );
		// sound_manager()->load_3d_sound( "vending-machine" );

		sound_manager()->load( "click" );

		sound_manager()->load( "walk" );
		sound_manager()->load( "run" );
		sound_manager()->load( "clamber" );
		sound_manager()->load( "collision-wall" );
		sound_manager()->load( "jump" );
		sound_manager()->load( "super-jump" );
		sound_manager()->load( "land" );
		sound_manager()->load( "short-breath" );
		sound_manager()->load( "dead" );

		sound_manager()->load( "balloon-get" );
		sound_manager()->load( "balloon-burst" );
		sound_manager()->load( "rocket-get" );
		sound_manager()->load( "rocket" );
		sound_manager()->load( "rocket-burst" );
		sound_manager()->load( "umbrella-get" );
		sound_manager()->load( "medal-get" );

		sound_manager()->load( "fin" );
		sound_manager()->load( "door" );
	}

	// Player
	player_ = new Player();
	player_->set_input( input() );
	player_->set_gravity( config()->get( "player.gravity", 0.01f ) );

	// Goal
	goal_ = new Goal();

	// Camera
	camera_ = new Camera();
	camera_->set_fov_default( config()->get( "camera.fov", 60.f ) );

	// Stage
	stage_ = new Stage( 1000, 1000 );
	player_->set_stage( stage_.get() );

	if ( get_stage_name().empty() )
	{
		generate_random_stage();
	}
	else
	{
		std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( save_data()->get( "stage-select.page", 0 ) );

		load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );
	}

	if ( ! sky_box_ )
	{
		sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box-3", "png" );
	}

	if ( lens_type_ != LENS_TYPE_NORMAL )
	{
		DIRECT_X_FAIL_CHECK( D3DXCreateTexture( direct_3d()->getDevice(), get_width(), get_height(), 1, D3DUSAGE_RENDERTARGET, direct_3d()->getPresentParameters().BackBufferFormat, D3DPOOL_DEFAULT, & back_buffer_texture_ ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->CreateDepthStencilSurface( get_width(), get_height(), direct_3d()->getPresentParameters().AutoDepthStencilFormat, D3DMULTISAMPLE_NONE, 0, TRUE, & depth_surface_, 0 ) );

		rectangle_ = new Direct3D9Rectangle( direct_3d() );
	}

	player_->restart();
	goal_->restart();

	for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
	{
		ActiveObject* active_object = *i;
		active_object->restart();
	}

	Sound* bgm = sound_manager()->get_sound( "bgm" );
	if ( bgm )
	{
		bgm->play( true );
	}

	direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_, 4 );
}

GamePlayScene::~GamePlayScene()
{
	// save_stage_file( "media/stage/quit" );

	player_mesh_.release();
	goal_mesh_.release();
	shadow_mesh_.release();
	ground_mesh_.release();
	scope_mesh_.release();

	stage_.release();
	camera_.release();
	goal_.release();
	player_.release();
	
	sky_box_.release();

	box_.release();
	rectangle_.release();

	DIRECT_X_RELEASE( depth_surface_ );
	DIRECT_X_RELEASE( back_buffer_surface_ );
	DIRECT_X_RELEASE( back_buffer_texture_ );

	grid_object_manager()->clear();
	grid_data_manager()->clear();

	active_object_manager()->clear();

	direct_3d()->getMeshManager()->unload_all();
	direct_3d()->getTextureManager()->unload_all();

	sound_manager()->stop_all();
	sound_manager()->unload_all();

	float ambient_color_none[] = { 1.f, 1.f, 1.f, 1.f };
	float object_color_none[] = { 1.f, 1.f, 1.f, 1.f };
	float add_color_none[] = { 0.f, 0.f, 0.f, 0.f };
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_none, 4 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color_none, 4 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color_none, 4 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
}

void GamePlayScene::generate_random_stage()
{
	grid_object_manager()->clear();

	GridData* building_a_grid = grid_data_manager()->load( "building-50" );
	GridData* building_b_grid = grid_data_manager()->load( "building-100" );
	GridData* building_c_grid = grid_data_manager()->load( "building-200" );
	GridData* house_a_grid = grid_data_manager()->load( "house-a" );
	
	GridData* road_grid = grid_data_manager()->load( "road" );
	GridData* road_curve_grid = grid_data_manager()->load( "road-curve" );

	// GridData* tel_box_grid = grid_data_manager()->load( "tel-box" );

	const int x_space = 0;
	const int z_space = 0;
	bool player_position_fixed = false;

	for ( int z = 0; z < stage_->depth(); z += 10 + z_space )
	{
		for ( int x = 0; x < stage_->width(); x += 10 + x_space )
		{
			GridData* grid_data = 0;
			bool tel_box = false;

			int y = 0;

			int random_value = common::random( 0, 100 );

			if ( random_value < 40 )
			{
				grid_data = building_a_grid;
			}
			else if ( random_value < 41 )
			{
				grid_data = building_b_grid;
				
				if ( ! player_position_fixed )
				{
					player_->start_position().set( static_cast< float >( x ), static_cast< float >( building_b_grid->cell( 0, 0 ).height() ), static_cast< float >( z ) );
					player_position_fixed = true;
				}
			}
			else if ( random_value < 42 )
			{
				grid_data = building_c_grid;
			}
			else if ( random_value < 60 )
			{
				grid_data = house_a_grid;
			}
			else
			{
				grid_data = road_grid;

				if ( common::random( 0, 5 ) == 0 )
				{
					tel_box = true;
				}
			}

			if ( grid_data )
			{
				int r = 0; // common::random( 0, 3 ) * 90;
				int dx = x;
				int dy = y;
				int dz = z;

				if ( stage_->put( dx, dy, dz, r, grid_data ) )
				{
					grid_object_manager()->add_grid_object( new GridObject( dx, dy, dz, r, grid_data ) );
				}
			}
		}
	}

	if ( ! player_position_fixed )
	{
		player_->start_position().set( common::random( 0.f, 100.f ), 3.f, common::random( 0.f, 100.f ) );
	}

	goal_->start_position() = player_->start_position();
	goal_->start_position().z() += 10.f;

	Enemy* enemy = new Enemy();
	enemy->set_mesh( direct_3d()->getMeshManager()->load( "enemy", "media/model/enemy-a.x" ) );
	enemy->set_stage( stage_.get() );
	enemy->set_player( player_.get() );
	enemy->position().set( player_->start_position().x(), 300.f, player_->start_position().z() + common::random( 0, 100 ) );
	enemy->set_direction_degree( 180 );

	active_object_manager()->add_active_object( enemy );

	for ( int n = 0; n < 30; n++ )
	{
		Balloon* balloon = new Balloon();
		balloon->set_mesh( direct_3d()->getMeshManager()->load( "balloon", "media/model/balloon.x" ) );
		balloon->set_stage( stage_.get() );
		balloon->position() = player_->start_position();
		balloon->position().x() += common::random( -50.f, +50.f );
		balloon->position().y() = common::random( 30.f, 150.f );
		balloon->position().z() += common::random( -50.f, +50.f );
		balloon->limit_position();
		balloon->start_position() = balloon->position();

		active_object_manager()->add_active_object( balloon );
	}
}

void GamePlayScene::load_stage_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load stage file \"" ) + file_name + "\" failed." );
	}

	/*
	const int OBJECT_COLOR_MAX = 3;
	float object_colors[ OBJECT_COLOR_MAX ][ 4 ] = {
		{ 1.f, 0.95f, 0.95f, 1.f },
		{ 0.95f, 1.f, 0.95f, 1.f },
		{ 0.95f, 0.95f, 1.f, 1.f },
	};
	*/

	/*
	float object_colors[ 3 ][ 4 ] = {
		{ 1.f, 0.5f, 0.5f, 1.f },
		{ 1.f, 1.f, 1.f, 1.f },
		{ 0.5f, 0.5f, 1.f, 1.f },
	};
	*/

	const int OBJECT_COLOR_MAX = 5;
	float object_colors[ OBJECT_COLOR_MAX ][ 4 ] = {
		{ 1.0f, 1.0f, 1.0f, 1.f },
		{ 0.9f, 0.9f, 0.9f, 1.f },
		{ 0.8f, 0.8f, 0.8f, 1.f },
		{ 0.7f, 0.7f, 0.7f, 1.f },
		{ 0.6f, 0.6f, 0.6f, 1.f },
	};

	int object_color_index = 0;

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

		if ( name == "lens" )
		{
			std::string name;
			ss >> name;

			if ( name == "fish-eye" )
			{
				lens_type_ = LENS_TYPE_FISH_EYE;
			}
			else if ( name == "crazy" )
			{
				lens_type_ = LENS_TYPE_CRAZY;
			}
		}
		else if ( name == "bgm" )
		{
			std::string name;
			ss >> name;

			sound_manager()->load_music( "bgm", name.c_str() );
		}
		else if ( name == "ambient" )
		{
			ss >> ambient_color_[ 0 ] >> ambient_color_[ 1 ] >> ambient_color_[ 2 ];
		}
		else if ( name == "player" )
		{
			ss >> player_->start_position().x() >> player_->start_position().y() >> player_->start_position().z();

			if ( ! ss.eof() )
			{
				ss >> camera_->rotate_degree().x();
				camera_->rotate_degree_target().x() = camera_->rotate_degree().x();
			}
		}
		else if ( name == "goal" )
		{
			ss >> goal_->start_position().x() >> goal_->start_position().y() >> goal_->start_position().z();
		}
		else if ( name == "sky-box" )
		{
			std::string sky_box_name;
			std::string sky_box_ext;

			ss >> sky_box_name >> sky_box_ext;

			sky_box_.release();

			if ( sky_box_name != "none" )
			{
				sky_box_ = new Direct3D9SkyBox( direct_3d(), sky_box_name.c_str(), sky_box_ext.c_str() );
			}
		}
		else if ( name == "object" )
		{
			std::string grid_data_name;
			int x = 0, y = 0, z = 0, r = 0;

			ss >> grid_data_name >> x >> y >> z >> r;

			GridData* grid_data = grid_data_manager()->load( grid_data_name.c_str() );

			if ( stage_->put( x, y, z, r, grid_data ) )
			{
				GridObject* grid_object = new GridObject( x, y, z, r, grid_data );
				
				if ( ss.eof() )
				{
					if ( grid_data->cell( 0, 0 ).height() > 0 )
					{
						grid_object->color()[ 0 ] = object_colors[ object_color_index ][ 0 ];
						grid_object->color()[ 1 ] = object_colors[ object_color_index ][ 1 ];
						grid_object->color()[ 2 ] = object_colors[ object_color_index ][ 2 ];
						grid_object->color()[ 3 ] = object_colors[ object_color_index ][ 3 ];
						object_color_index = ( object_color_index + 1 ) % OBJECT_COLOR_MAX;
					}
				}
				else
				{
					ss >> grid_object->color()[ 0 ] >> grid_object->color()[ 1 ] >> grid_object->color()[ 2 ];
				}

				grid_object_manager()->add_grid_object( grid_object );
			}
		}
		else if ( name == "enemy" )
		{
			Enemy* enemy = new Enemy();
			enemy->set_player( player_.get() );
			active_object = enemy;
		}
		else if ( name == "balloon" )
		{
			active_object = new Balloon();
		}
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

		if ( active_object )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			active_object->set_mesh( direct_3d()->getMeshManager()->load( name.c_str(), ( std::string( "media/model/" ) + name + ".x" ).c_str() ) );
			active_object->set_stage( stage_.get() );
			active_object->start_position().set( x, y, z );
			active_object->set_direction_degree( r );
			active_object_manager()->add_active_object( active_object );
		}
	}
}

void GamePlayScene::save_stage_file( const char* file_name ) const
{
	std::ofstream out( file_name );
	
	if ( ! out.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "save stage file \"" ) + file_name + "\" failed." );
	}

	out << "player " << player_->position().x() << " " << player_->position().z() << " " << player_->position().z() << std::endl;
	
	for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
	{
		GridObject* grid_object = *i;

		out << "object " << grid_object->grid_data()->get_name() << " " << grid_object->x() << " " << grid_object->y() << " " << grid_object->z() << std::endl;
	}
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void GamePlayScene::update()
{
	if ( ! player_->is_dead() && ! is_cleared_ )
	{
		bool step = false;

		if ( input()->press( Input::B ) || input()->press( Input::UP ) )
		{
			player_->step( +1 );
			step = true;
		}
		if ( input()->press( Input::DOWN ) )
		{
			player_->step( -1 );
			step = true;
		}
		if ( input()->press( Input::LEFT ) )
		{
			player_->side_step( -1 );
			step = true;
		}
		if ( input()->press( Input::RIGHT ) )
		{
			player_->side_step( +1 );
			step = true;
		}

		if ( ! step )
		{
			player_->stop();
		}
		
		int wheel = input()->pop_mouse_wheel_queue();

		/*
		if ( wheel )
		{
			sound_manager()->get_sound( "click" )->play( false );
		}
		*/

		if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
		{
			if ( wheel > 0 )
			{
				camera_->set_fov_target( camera_->fov() * 0.5f );
			}
			else if ( wheel < 0 )
			{
				camera_->set_fov_target( camera_->fov() * 2.f );
			}
		}
		else
		{
			if ( wheel > 0 )
			{
				player_->select_prev_item();
			}
			else if ( wheel < 0 )
			{
				player_->select_next_item();
			}
		}

		if ( input()->push( Input::A ) )
		{
			switch ( player_->get_selected_item_type() )
			{
				case Player::ITEM_TYPE_NONE: player_->jump(); break;
				case Player::ITEM_TYPE_ROCKET: player_->rocket( camera_->front() ); break;
				case Player::ITEM_TYPE_UMBRELLA: player_->start_umbrella_mode(); player_->jump(); break;
				case Player::ITEM_TYPE_SCOPE:
				{
					player_->switch_scope_mode();

					if ( player_->get_action_mode() != Player::ACTION_MODE_SCOPE )
					{
						camera_->reset_fov();
					}
					break;
				}
			}
		}
	}

	if ( ! is_cleared_ )
	{
		player_->add_direction_degree( input()->get_mouse_dx() * 90.f );

		camera_->rotate_degree_target().y() = player_->get_direction_degree();

		camera_->rotate_degree_target().x() += input()->get_mouse_dy() * 90.f;
		camera_->rotate_degree_target().x() = math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f );

		player_->update();

		active_object_manager()->update();
	}

	for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
	{
		ActiveObject* active_object = *i;

		if ( active_object->is_dead() )
		{
			continue;
		}

		if ( active_object->collision_detection( player_.get() ) )
		{
			// active_object->on_collision( player_ );
			// player_->on_collision( active_object );

			if ( dynamic_cast< Balloon* >( active_object ) )
			{
				player_->on_get_balloon();
				active_object->kill();
			}
			else if ( dynamic_cast< Rocket* >( active_object ) )
			{
				player_->on_get_rocket();
				active_object->kill();
			}
			else if ( dynamic_cast< Umbrella* >( active_object ) )
			{
				player_->on_get_umbrella();
				active_object->kill();
			}
			else if ( dynamic_cast< Medal* >( active_object ) )
			{
				player_->on_get_medal();
				active_object->kill();
			}
			else if ( dynamic_cast< Poison* >( active_object ) )
			{
				player_->kill();
				active_object->kill();
			}
			else
			{
				player_->kill();
			}
		}
	}

	if ( ! is_cleared_ && player_->collision_detection( goal_.get() ) )
	{
		player_->set_gravity( player_->get_gravity() * 0.1f );
		player_->velocity().set( 0.f, 0.5f, 0.f );

		is_cleared_ = true;

		sound_manager()->stop_all();
		sound_manager()->get_sound( "fin" )->play( false );
	}

	if ( is_cleared_ )
	{
		vector3 target_position = goal_->position();
		target_position.z() -= 4.f - sound_manager()->get_sound( "fin" )->get_current_position() * 0.5f;

		player_->position() = ( player_->position() * 0.95f + target_position * 0.05f );
		player_->set_direction_degree( 0.f );

		camera_->rotate_degree_target().x() = 0.f;
		camera_->rotate_degree_target().y() = player_->get_direction_degree();
		camera_->rotate_degree_target().z() = 0.f;
		camera_->set_rotate_chase_speed( 0.1f );

		brightness_ = math::chase( brightness_, 1.f, 0.002f );

		if ( sound_manager()->get_sound( "fin" )->get_current_position() >= 6.f && ! sound_manager()->get_sound( "door" )->is_playing() )
		{
			sound_manager()->get_sound( "door" )->play( false );
		}

		if ( sound_manager()->get_sound( "fin" )->get_current_position() >= 9.f )
		{
			std::string save_param_name = StageSelectScene::get_stage_prefix_by_page( save_data()->get( "stage-select.page", 0 ) ) + "." + get_stage_name();

			save_data()->set( save_param_name.c_str(), std::max( player_->has_medal() ? 2 : 1, save_data()->get( save_param_name.c_str(), 0 ) ) );
			set_next_scene( "stage_outro" );
		}
	}
	else if ( player_->is_dead() )
	{
		camera_->rotate_degree_target().z() = 90.f;
		brightness_ = math::chase( brightness_, -0.4f, 0.01f );

		if ( camera_->rotate_degree().z() == camera_->rotate_degree_target().z() && input()->push( Input::A ) )
		{
			camera_->rotate_degree().set( 0.f, 0.f, 0.f );
			camera_->rotate_degree_target().set( 0.f, 0.f, 0.f );
			camera_->reset_rotate_chase_speed();

			player_->rebirth();

			for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
			{
				ActiveObject* active_object = *i;
				active_object->restart();
			}

			brightness_ = 1.f;
		}
	}
	else if ( player_->is_falling_to_dead() )
	{
		brightness_ = math::chase( brightness_, 0.2f, 0.02f );
	}
	else
	{
		brightness_ = math::chase( brightness_, 0.f, 0.02f );
	}

	if ( player_->pop_look_floor_request() )
	{
		camera_->rotate_degree_target().x() = 60.f;
	}

	camera_->position() = player_->position() + vector3( 0.f, player_->get_eye_height(), 0.f ); // + player_->front();
	camera_->update();

	if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
	{
		camera_->set_fov( math::clamp( camera_->fov(), 1.f, 50.f ) );
	}

	// camera_->position() = player_->position() + player_->front() + vector3( 0.f, player_->get_eye_height(), 0.f );


	if ( ! is_cleared_ && player_->is_rocketing() )
	{
		// camera_->set_fov( math::chase( camera_->fov(), 30.f, 0.2f ) );
		camera_->position().x() += common::random( -0.01f, 0.01f );
		camera_->position().y() += common::random( -0.01f, 0.01f );
		camera_->position().z() += common::random( -0.01f, 0.01f );
	}
	else
	{
		// camera_->set_fov( math::chase( camera_->fov(), 60.f, 0.4f ) );
	}

	sound_manager()->set_listener_position( camera_->position() );
	sound_manager()->set_listener_velocity( player_->velocity() );
	sound_manager()->set_listener_orientation( camera_->front(), camera_->up() );
	sound_manager()->commit();
}

/**
 * •`‰æ
 */
bool GamePlayScene::render()
{
	const float object_color_none[] = { 1.f, 1.f, 1.f, 1.f };
	const float add_color_none[] = { 0.f, 0.f, 0.f, 0.f };

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );

	LPDIRECT3DSURFACE9 default_back_buffer_surface = 0;
	LPDIRECT3DSURFACE9 default_depth_surface = 0;
	
	if ( lens_type_ != LENS_TYPE_NORMAL )
	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->GetRenderTarget( 0, & default_back_buffer_surface ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->GetDepthStencilSurface( & default_depth_surface ) );
		
		DIRECT_X_FAIL_CHECK( back_buffer_texture_->GetSurfaceLevel( 0, & back_buffer_surface_ ) );

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderTarget( 0, back_buffer_surface_ ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetDepthStencilSurface( depth_surface_ ) );
	}

	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
	direct_3d()->getEffect()->SetTechnique( technique );

	UINT pass_count = 0;

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", brightness_ ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->Begin( & pass_count, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );
	
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

	D3DXMATRIXA16 world;
	D3DXMATRIXA16 view;
	D3DXMATRIXA16 projection;
	D3DXMATRIXA16 WorldViewProjection;
	D3DXMATRIXA16 transform;

	D3DXMATRIXA16 r;
	D3DXMATRIXA16 s;
	D3DXMATRIXA16 t;

	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xCC, 0xFF ), 1.f, 0 ) );
	
		D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->look_at() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
		D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );
		
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );

		// SkyBox
		if ( sky_box_ )
		{
			D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );
			D3DXMatrixTranslation( & t, camera_->position().x(), camera_->position().y(), camera_->position().z() );

			world = s * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
			sky_box_->render();
		}

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );

		// Ground
		if ( ground_mesh_ )
		{
			const int gx = static_cast< int >( player_->position().x() );
			const int gy = static_cast< int >( player_->position().z() );

			// D3DXMatrixTranslation( & t, static_cast< float >( gx / 10 * 10 ), 0.f, static_cast< float >( gy / 10 * 10 ) );
			D3DXMatrixTranslation( & t, stage_->width() * 0.5f, 0.f, stage_->depth() * 0.5f );
			world = t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
			ground_mesh_->render();
		}
		
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

#if 1
		// GridObject
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
		{
			GridObject* grid_object = *i;

			const float x_length = std::abs( player_->position().x() - static_cast< float >( grid_object->x() ) ) + grid_object->z() * 0.02f;
			const float y_length = std::abs( player_->position().y() - static_cast< float >( grid_object->y() ) );
			const float z_length = std::abs( player_->position().z() - static_cast< float >( grid_object->z() ) ) + grid_object->x() * 0.02f;

			if ( x_length >= grid_object_visible_length_ || z_length >= grid_object_visible_length_ )
			{
				grid_object->set_visible( false );
			}
			else
			{
				grid_object->set_visible( true );
			}

			if ( grid_object->visible_alpha() == 0.f )
			{
				continue;
			}

			const float offset = 0.05f;
			const float flicker = 0.f; // sin( grid_object->x() + grid_object->z() * 0.001f + a ) * 0.5f;

			D3DXMatrixRotationY( & r, math::degree_to_radian( static_cast< float >( grid_object->rotate_degree() ) ) );
			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), static_cast< float >( grid_object->y() + flicker + offset ), static_cast< float >( grid_object->z() ) );

			world = s * r * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );

			if ( x_length < grid_object_lod_0_length_ && z_length < grid_object_lod_0_length_ )
			{
				grid_object->set_lod( 0 );
			}
			else
			{
				grid_object->set_lod( 1 );
			}

			float* object_color = grid_object->color();

			if ( grid_object->has_last_lod() )
			{
				DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

				object_color[ 3 ] = 1.f;
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render_last_lod();

				DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

				object_color[ 3 ] = grid_object->lod_alpha();
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render();
			}
			else
			{
				object_color[ 3 ] = 1.f;
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render();
			}
		}
#endif // 0

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color_none, 4 ) );

		/*
		// Box
		direct_3d()->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

		box_->ready();

		for ( int z = 0; z < stage_->depth(); z++ )
		{
			for ( int x = 0; x < stage_->width(); x++ )
			{
				int y = stage_->cell( x, z ).height();
			
				if ( y > 0 )
				{
					y = 1;

					D3DXMatrixTranslation( & world, x + 0.5f, y - 0.5f, z + 0.5f );

					D3DXMATRIX WorldViewProjection = world * view * projection;
					DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );

					box_->render();
				}
			}
		}
		*/

		// Goal
		D3DXMatrixTranslation( & t, goal_->position().x(), goal_->position().y() + 0.05f, goal_->position().z() );

		world = t;
		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		goal_mesh_->render();

		// ActiveObject ( Shadow )
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

		for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
		{
			ActiveObject* active_object = *i;

			if ( active_object->is_dead() )
			{
				continue;
			}

			render_shadow( active_object, view * projection );
		}

		// Player ( Shadow )
		render_shadow( player_.get(), view * projection );

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

		// Player's Balloon
		if ( player_->get_action_mode() == Player::ACTION_MODE_BALLOON && ( get_current_time() % 20 < 10 || ! player_->is_action_pre_finish() ) )
		{
			vector3 pos = player_->position() + player_->front() * 0.5f - player_->right() * 0.5f;
			pos.y() += 0.5f;

			D3DXMatrixTranslation( & t, pos.x(), pos.y(), pos.z() );

			WorldViewProjection = t * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			balloon_mesh_->render();
		}
		// Player's Rocket
		if ( player_->get_action_mode() == Player::ACTION_MODE_ROCKET && ( get_current_time() % 20 < 10 || ! player_->is_action_pre_finish() ) )
		{
			/*
			vector3 pos = camera_->position() + camera_->front() * 0.2f;
			pos.y() -= 0.2f;

			D3DXMatrixRotationX( & r, math::degree_to_radian( -90.f ) );
			D3DXMatrixRotationY( & s, math::degree_to_radian( camera_->rotate_degree().y() ) ); // bad
			D3DXMatrixTranslation( & t, pos.x(), pos.y(), pos.z() );
			*/

			D3DXMatrixRotationX( & r, math::degree_to_radian( 90.f ) );
			D3DXMatrixRotationZ( & s, math::degree_to_radian( get_current_time() * 0.1f ) );
			D3DXMatrixTranslation( & t, 0.f, -1.f, 0.f );

			WorldViewProjection = r * s * t * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			rocket_mesh_->render();
		}

		// Player
		if ( player_mesh_ )
		{
			vector3 pos = player_->position();
			pos += -camera_->front() * 0.1f;

			D3DXMatrixRotationY( & r, math::degree_to_radian( camera_->rotate_degree().y() ) );
			D3DXMatrixTranslation( & t, pos.x(), pos.y() + 0.05f, pos.z() );

			world = r * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			player_mesh_->render();
		}

		// ActiveObject
		for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
		{
			ActiveObject* active_object = *i;

			if ( active_object->is_dead() )
			{
				continue;
			}

			D3DXMatrixRotationY( & r, math::degree_to_radian( active_object->get_direction_degree() ) );
			D3DXMatrixTranslation( & t, active_object->position().x(), active_object->position().y() + 0.05f, active_object->position().z() );

//			float color[] = { 1.f, 1.f, 0.5f, 0.5f };

			world = r * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
//			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", color, 3 ) );
//			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", color, 4 ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			active_object->get_mesh()->render();
		}

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color_none, 4 ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color_none, 4 ) );

		// Player's Umbrella
		if ( player_->get_action_mode() == Player::ACTION_MODE_UMBRELLA && ( get_current_time() % 20 < 10 || ! player_->is_action_pre_finish() ) )
		{
			vector3 pos = player_->position() + player_->front() * 0.2f - player_->right() * 0.1f;
			pos.y() += 1.f;

			D3DXMatrixRotationY( & r, math::degree_to_radian( camera_->rotate_degree().y() ) );
			D3DXMatrixTranslation( & t, pos.x(), pos.y(), pos.z() );

			WorldViewProjection = r * t * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			umbrella_mesh_->render();
		}

		// cleanup
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );


	// Lens Effect
	if ( lens_type_ != LENS_TYPE_NORMAL )
	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderTarget( 0, default_back_buffer_surface ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetDepthStencilSurface( default_depth_surface ) );
		
		DIRECT_X_RELEASE( default_back_buffer_surface );
		DIRECT_X_RELEASE( default_depth_surface );
		DIRECT_X_RELEASE( back_buffer_surface_ );

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( static_cast< int >( lens_type_ ) ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xCC, 0xCC, 0xFF ), 1.f, 0 ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );

		static float lens_ratio = 0.f;

		{
			if ( player_->is_falling_to_dead() )
			{
				lens_ratio = math::chase( lens_ratio, 1.f, 0.02f );
			}
			else
			{
				lens_ratio = math::chase( lens_ratio, 0.f, 0.02f );
			}
		}
		
		if ( 0 )
		{
			static float lens_ratio_a = 0.f;

			lens_ratio_a += math::degree_to_radian( 1.f );

			lens_ratio = ( std::sin( lens_ratio_a ) + 1.f ) * 0.5f;
		}

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "lens_ratio", lens_ratio ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetTexture( 0, back_buffer_texture_ ) );

		rectangle_->ready();
		rectangle_->render();

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	}

	// aim
	if ( player_->get_selected_item_type() == Player::ITEM_TYPE_ROCKET )
	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 3 ) );

		D3DXMatrixScaling( & s, 5.f, 5.f, 5.f );
		D3DXMatrixTranslation( & t, 0.f, 0.f, 65.f );

		float object_color[] = { 0.f, 0.f, 1.f, 0.5f };

		WorldViewProjection = s * t * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color, 4 ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		aim_mesh_->render();

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", object_color_none, 4 ) );
	}

	// Scope
	if ( player_->get_action_mode() == Player::ACTION_MODE_SCOPE )
	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

		D3DXMatrixScaling( & s, 0.9f / camera_->aspect(), 0.9f, 1.f );
		D3DXMatrixTranslation( & t, 0.f, 0.f, 1.f );

		WorldViewProjection = s * t; // * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
		scope_mesh_->render();

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	// UI
	direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND );

	if ( player_->get_selected_item_type() == Player::ITEM_TYPE_ROCKET )
	{
		for ( int n = 0; n < player_->get_item_count( Player::ITEM_TYPE_ROCKET ); n++ )
		{
			const float offset = n * 20.f;

			win::Rect src_rect = win::Rect::Size( 0, 0, 202, 200 );
			D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

			D3DXMatrixTranslation( & t, get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );
			transform = t;

			direct_3d()->getSprite()->SetTransform( & transform );
			direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
		}

		// aim
		win::Rect src_rect = win::Rect::Size( 256, 0, 76, 80 );
		D3DXVECTOR3 center( 34.f, 38.f, 0.f );

		D3DXMatrixTranslation( & transform, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0x99FFFFFF );
	}
	else if ( player_->get_selected_item_type() == Player::ITEM_TYPE_UMBRELLA )
	{
		for ( int n = 0; n < player_->get_item_count( Player::ITEM_TYPE_UMBRELLA ); n++ )
		{
			const float offset = n * 50.f;

			win::Rect src_rect = win::Rect::Size( 0, 256, 186, 220 );
			D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

			D3DXMatrixTranslation( & t, get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );

			direct_3d()->getSprite()->SetTransform( & t );
			direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
		}
	}
	else if ( player_->get_selected_item_type() == Player::ITEM_TYPE_SCOPE )
	{
		win::Rect src_rect = win::Rect::Size( 0, 512, 192, 192 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & t, get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & t );
		direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	if ( player_->has_medal() )
	{
		win::Rect src_rect = win::Rect::Size( 384, 0, 64, 64 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & t, src_rect.width() * 0.6f, get_height() - src_rect.height() * 0.6f, 0.f );
		direct_3d()->getSprite()->SetTransform( & t );
		direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0x99FFFFFF );
	}

	direct_3d()->getSprite()->End();

	/*
	std::string debug_text = "player : (" + 
			common::serialize( static_cast< int >( player_->position().x() ) ) + "," +
			common::serialize( static_cast< int >( player_->position().y() ) ) + "," +
			common::serialize( static_cast< int >( player_->position().z() ) ) + ")\n" +
			"mouse : ( " +
			common::serialize( input()->get_mouse_x_rate() ) + ", " +
			common::serialize( input()->get_mouse_y_rate() ) + ")\n( " +
			common::serialize( input()->get_mouse_dx() ) + ", " +
			common::serialize( input()->get_mouse_dy() ) + ")";

	debug_text += std::string( "\ncamera : " ) + common::serialize( camera_->rotate_degree_target().y() );

	direct_3d()->getFont()->draw_text( 0, 24, debug_text.c_str(), D3DCOLOR_XRGB( 0, 0, 0 ) );
	*/

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;
}

void GamePlayScene::render_shadow( const ActiveObject* active_object, const D3DXMATRIXA16& after )
{
	D3DXMATRIXA16 r;
	D3DXMATRIXA16 t;
	D3DXMATRIXA16 WorldViewProjection;

	D3DXMatrixRotationY( & r, math::degree_to_radian( active_object->get_direction_degree() ) );

	std::list< float > grid_cell_height_list;

	grid_cell_height_list.push_back( active_object->get_floor_cell_center().height() );
	grid_cell_height_list.push_back( active_object->get_floor_cell_left_front().height() );
	grid_cell_height_list.push_back( active_object->get_floor_cell_right_front().height() );
	grid_cell_height_list.push_back( active_object->get_floor_cell_left_back().height() );
	grid_cell_height_list.push_back( active_object->get_floor_cell_right_back().height() );

	for ( std::list< float >::iterator i = grid_cell_height_list.begin(); i != grid_cell_height_list.end(); ++i )
	{
		if ( *i > active_object->position().y() ) continue;

		D3DXMatrixTranslation( & t, active_object->position().x() , *i + 0.2f, active_object->position().z() );

		WorldViewProjection = r * t * after;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );

		if ( active_object == player_.get() )
		{
			if ( player_->is_if_fall_to_dead( *i ) || player_->is_dead() )
			{
				float add_color[] = { 0.5f, 0.f, 0.f, 1.f };
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color, 4 ) );
			}
			else if ( player_->is_if_fall_to_umbrella_lost( *i ) )
			{
				float add_color[] = { 0.5f, 0.5f, 0.f, 1.f };
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color, 4 ) );
			}
			else
			{
				float add_color[] = { 0.f, 0.f, 0.5f, 1.f };
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color, 4 ) );
			}
		}

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		shadow_mesh_->render();
	}

	if ( active_object == player_.get() )
	{
		float add_color[] = { 0.f, 0.f, 0.f, 1.f };
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "add_color", add_color, 4 ) );
	}
}

} // namespace blue_sky
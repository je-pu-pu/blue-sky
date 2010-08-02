#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include "App.h"

#include "Player.h"
#include "Goal.h"
#include "Enemy.h"
#include "Balloon.h"
#include "Rocket.h"
#include "Camera.h"
#include "Stage.h"

#include "Direct3D9Font.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9SkyBox.h"
#include "Direct3D9Box.h"
#include "Direct3D9Rectangle.h"
#include "Direct3D9.h"
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

float brightness = 1.f;
bool clear_flag = false;

int rocket_count_ = 0;

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, back_buffer_texture_( 0 )
	, back_buffer_surface_( 0 )
	, depth_surface_( 0 )
	, ui_texture_( 0 )
	, grid_object_visible_length_( 500 )
	, grid_object_lod_0_length_( 100 )
	, lens_type_( LENS_TYPE_NORMAL )
{
	/// @todo xxx
	rocket_count_ = 0;

	ambient_color_[ 0 ] = 1.f;
	ambient_color_[ 1 ] = 1.f;
	ambient_color_[ 2 ] = 1.f;
	ambient_color_[ 3 ] = 1.f;

	grid_object_visible_length_ = config()->get( "video.grid-object-visible-length", 500.f );
	grid_object_lod_0_length_ = config()->get( "video.grid-object-lod-0-length", 100.f );

	clear_flag = false;

	// Texture
	ui_texture_ = direct_3d()->getTextureManager()->load( "ui", "media/image/item.png" );

	// Font
	font_ = new Direct3D9Font( direct_3d() );

	// Mesh
	player_mesh_ = new Direct3D9Mesh( direct_3d() );
	player_mesh_->load_x( "media/model/player" );

	enemy_mesh_ = new Direct3D9Mesh( direct_3d() );
	enemy_mesh_->load_x( "media/model/enemy-a" );

	shadow_mesh_ = new Direct3D9Mesh( direct_3d() );
	shadow_mesh_->load_x( "media/model/shadow" );

	ground_mesh_ = new Direct3D9Mesh( direct_3d() );
	ground_mesh_->load_x( "media/model/ground" );

	goal_mesh_ = new Direct3D9Mesh( direct_3d() );
	goal_mesh_->load_x( "media/model/door" );

	balloon_mesh_ = new Direct3D9Mesh( direct_3d() );
	balloon_mesh_->load_x( "media/model/balloon" );

	rocket_mesh_ = new Direct3D9Mesh( direct_3d() );
	rocket_mesh_->load_x( "media/model/rocket" );

	// SkyBox
	// sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box-3", "png" );
	sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box-star-2", "png" );

	// Box
	box_ = new Direct3D9Box( direct_3d(), 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Sound
	{
		sound_manager()->stop_all();
		sound_manager()->unload_all();

		sound_manager()->load_3d_sound( "ok" );
		sound_manager()->load_3d_sound( "vending-machine" );

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
		sound_manager()->load( "rocket-get" );

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
	camera_->set_fov( config()->get( "camera.fov", 60.f ) );

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

	font_.release();

	player_mesh_.release();
	goal_mesh_.release();
	enemy_mesh_.release();
	shadow_mesh_.release();
	ground_mesh_.release();
	
	balloon_mesh_.release();
	rocket_mesh_.release();

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
	active_object_manager()->clear();

	sound_manager()->stop_all();
	sound_manager()->unload_all();
}

void GamePlayScene::generate_random_stage()
{
	grid_object_manager()->clear();

	GridData* building_a_grid = grid_data_manager()->load( "building-a" );
	GridData* building_b_grid = grid_data_manager()->load( "building-b" );
	GridData* building_c_grid = grid_data_manager()->load( "building-c" );
	GridData* house_a_grid = grid_data_manager()->load( "house-a" );
	
	GridData* road_grid = grid_data_manager()->load( "road" );
	GridData* road_curve_grid = grid_data_manager()->load( "road-curve" );

	GridData* tel_box_grid = grid_data_manager()->load( "tel-box" );

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

					if ( tel_box )
					{
						if ( stage_->put( dx, dy, dz, r, tel_box_grid ) )
						{
							grid_object_manager()->add_grid_object( new GridObject( dx, dy, dz, r, tel_box_grid ) );
						}
					}
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
	enemy->set_stage( stage_.get() );
	enemy->set_player( player_.get() );
	enemy->position().set( player_->start_position().x(), 300.f, player_->start_position().z() + common::random( 0, 100 ) );
	enemy->set_direction_degree( 180 );

	active_object_manager()->add_active_object( enemy );

	for ( int n = 0; n < 30; n++ )
	{
		Balloon* balloon = new Balloon();
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

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		std::stringstream ss;
		
		std::string name;
		std::string value;
		
		ss << line;

		ss >> name;

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
				grid_object_manager()->add_grid_object( new GridObject( x, y, z, r, grid_data ) );
			}
		}
		else if ( name == "enemy" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			Enemy* enemy = new Enemy();
			enemy->set_stage( stage_.get() );
			enemy->set_player( player_.get() );
			enemy->start_position().set( x, y, z );
			enemy->set_direction_degree( r );

			active_object_manager()->add_active_object( enemy );
		}
		else if ( name == "balloon" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			ActiveObject* active_object = new Balloon();
			active_object->set_stage( stage_.get() );
			active_object->start_position().set( x, y, z );
			active_object->set_direction_degree( r );
			active_object_manager()->add_active_object( active_object );
		}
		else if ( name == "rocket" )
		{
			float x = 0, y = 0, z = 0, r = 0;
			ss >> x >> y >> z >> r;

			ActiveObject* active_object = new Rocket();
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
	if ( ! player_->is_dead() && ! clear_flag )
	{
		bool step = false;

		if ( input()->press( Input::B ) || input()->press( Input::UP ) )
		{
			player_->step( +1 );
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

		if ( input()->push( Input::A ) )
		{
			if ( rocket_count_ > 0 )
			{
				player_->rocket( camera_->front() );
				rocket_count_--;
			}
			else
			{
				player_->jump();
			}
		}

		if ( input()->push( Input::X ) )
		{
			player_->rocket( camera_->front() );
		}
		if ( input()->push( Input::Y ) )
		{
			player_->start_umbrella_mode();
		}
	}

	if ( ! clear_flag )
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

				rocket_count_++;
			}
			else
			{
				player_->kill();
			}
		}
	}

	if ( ! clear_flag && player_->collision_detection( goal_.get() ) )
	{
		player_->set_gravity( player_->get_gravity() * 0.1f );
		player_->velocity().set( 0.f, 0.5f, 0.f );

		clear_flag = true;

		sound_manager()->stop_all();
		sound_manager()->get_sound( "fin" )->play( false );
	}

	if ( clear_flag )
	{
		vector3 target_position = goal_->position();
		target_position.z() -= 4.f - sound_manager()->get_sound( "fin" )->get_current_position() * 0.5f;

		player_->position() = ( player_->position() * 0.95f + target_position * 0.05f );
		player_->set_direction_degree( 0.f );

		camera_->rotate_degree_target().x() = 0.f;
		camera_->rotate_degree_target().y() = player_->get_direction_degree();
		camera_->rotate_degree_target().z() = 0.f;
		camera_->set_rotate_chase_speed( 0.1f );

		brightness = math::chase( brightness, 1.f, 0.002f );

		if ( sound_manager()->get_sound( "fin" )->get_current_position() >= 6.f && ! sound_manager()->get_sound( "door" )->is_playing() )
		{
			sound_manager()->get_sound( "door" )->play( false );
		}

		if ( sound_manager()->get_sound( "fin" )->get_current_position() >= 9.f )
		{
			save_data()->set( ( StageSelectScene::get_stage_prefix_by_page( save_data()->get( "stage-select.page", 0 ) ) + "." + get_stage_name() ).c_str(), 1 );
			set_next_scene( "stage_outro" );
		}
	}
	else if ( player_->is_dead() )
	{
		camera_->rotate_degree_target().z() = 90.f;
		brightness = math::chase( brightness, -0.4f, 0.01f );

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

			brightness = 1.f;
		}
	}
	else if ( player_->is_falling_to_dead() )
	{
		brightness = math::chase( brightness, 0.2f, 0.02f );
	}
	else
	{
		brightness = math::chase( brightness, 0.f, 0.02f );
	}

	camera_->position() = player_->position() + vector3( 0.f, player_->get_eye_height(), 0.f );
	camera_->update();
	// camera_->position() = player_->position() + player_->front() + vector3( 0.f, player_->get_eye_height(), 0.f );

	if ( ! clear_flag && player_->is_rocketing() )
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

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", brightness ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->Begin( & pass_count, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );
	
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

	D3DXMATRIXA16 world;
	D3DXMATRIXA16 view;
	D3DXMATRIXA16 projection;
	D3DXMATRIXA16 WorldViewProjection;

	static float a = 0.f;
	a += 0.02f;

	{
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xCC, 0xFF ), 1.f, 0 ) );
		
		D3DXMATRIXA16 r;
		D3DXMATRIXA16 s;
		D3DXMATRIXA16 t;
		
		/*
		camera_->set_fov( 60.f );
		vector3 at = camera_->position();
		at.z() += 1.f;
		*/

		D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->look_at() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
		// D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & goal_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
		// D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & at) ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
		D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );
		
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );

		// SkyBox
		if ( sky_box_ )
		// if ( false )
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
//		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );
		
#if 1
		// GridObject
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		float colors[][ 4 ] = {
			{ 1.f, 0.95f, 0.95f, 1.f },
			{ 0.95f, 1.f, 0.95f, 1.f },
			{ 0.95f, 0.95f, 1.f, 1.f },
		};

		int color_index = 0;

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

			if ( grid_object->has_last_lod() )
			{
				DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

				ambient_color_[ 3 ] = 1.f;
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render_last_lod();

				DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

				ambient_color_[ 3 ] = grid_object->lod_alpha();
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render();
			}
			else
			{
				// ambient_color_[ 3 ] = grid_object->visible_alpha();
				// DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_, 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", colors[ color_index ], 4 ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

				grid_object->render();
			}

			color_index = ( color_index + 1 ) % 3;
		}
#endif // 0

		ambient_color_[ 3 ] = 1.f;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "ambient_color", ambient_color_, 4 ) );

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

		// Player
		/*
		vector3 pos = player_->position();
		pos += -camera_->front() * 0.1f;

		D3DXMatrixRotationY( & r, math::degree_to_radian( camera_->rotate_degree().y() ) );
		D3DXMatrixTranslation( & t, pos.x(), pos.y() + 0.05f, pos.z() );

		world = r * t;
		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		player_mesh_->render();
		*/

		// ActiveObject
		for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
		{
			ActiveObject* active_object = *i;

			if ( active_object->is_dead() )
			{
				continue;
			}

			Direct3D9Mesh* mesh = 0;

			if ( dynamic_cast< Enemy* >( active_object ) )
			{
				mesh = enemy_mesh_.get();
			}
			else if ( dynamic_cast< Rocket* >( active_object ) )
			{
				mesh = rocket_mesh_.get();
			}
			else
			{
				mesh = balloon_mesh_.get();
			}

			D3DXMatrixScaling( & s, 1.1f, 1.1f, 1.1f );
			D3DXMatrixRotationY( & r, math::degree_to_radian( active_object->get_direction_degree() ) );
			D3DXMatrixTranslation( & t, active_object->position().x(), active_object->position().y() + 0.05f, active_object->position().z() );

			float white[] = { 0.f, 0.f, 0.f, 1.f };
			float none[] = { 1.f, 1.f, 1.f, 1.f };

			world = s * r * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", white, 4 ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

			mesh->render();

			DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );

			world = r * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloatArray( "object_color", none, 4 ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			mesh->render();
		}

		// ActiveObject ( Shadow )
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

		static float x_ratio_a = 0.f;
		// static float y_ratio_a = 0.f;

		x_ratio_a += math::degree_to_radian( 1.f );
		// y_ratio_a += math::degree_to_radian( 0.9f );

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "ratio", ( std::sin( x_ratio_a ) + 1.f ) * 0.5f ) );
		// DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "y_ratio", ( std::sin( y_ratio_a ) + 1.f ) * 0.5f ) );

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetTexture( 0, back_buffer_texture_ ) );

		rectangle_->ready();
		rectangle_->render();

		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	// UI
	direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND );

	if ( rocket_count_ > 0 )
	{
		D3DXMATRIXA16 t, transform;

		for ( int n = 0; n < rocket_count_; n++ )
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

	/*
	for ( int n = 3; n < 6; n++ )
	{
		const float offset = n * 50.f;

		win::Rect src_rect = win::Rect::Size( 0, 256, 186, 220 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & t, get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );
		transform = t;

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( ui_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}
	*/

	direct_3d()->getSprite()->End();

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

	font_->draw_text( 0, 24, debug_text.c_str(), D3DCOLOR_XRGB( 0, 0, 0 ) );

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
		D3DXMatrixTranslation( & t, active_object->position().x() , *i + 0.1f, active_object->position().z() );

		WorldViewProjection = r * t * after;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		shadow_mesh_->render();
	}
}

} // namespace blue_sky
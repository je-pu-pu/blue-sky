#include "GamePlayScene.h"

#include "App.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"

#include "Direct3D9Font.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9SkyBox.h"
#include "Direct3D9Box.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include "Input.h"
#include "SoundManager.h"
#include "GridDataManager.h"
#include "GridObjectManager.h"
#include "GridObject.h"
#include "GridCell.h"

#include "ActiveObjectManager.h"
#include "Enemy.h"

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

Direct3D9Mesh* landscape_ = 0;
Direct3D9Mesh* enemy_mesh_ = 0;

float brightness = 0.f;

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, player_( 0 )
	, camera_( 0 )
	, stage_( 0 )
	, font_( 0 )
	, player_mesh_( 0 )
	, shadow_mesh_( 0 )
	, ground_mesh_( 0 )
	, sky_box_( 0 )
	, box_( 0 )
	, panorama_y_division_( config()->get( "panorama_y_division", 1 ) )
{
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

	
	landscape_ = new Direct3D9Mesh( direct_3d() );
	landscape_->load_x( "media/model/landscape" );

	// SkyBox
	sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box", "jpg" );

	// Box
	box_ = new Direct3D9Box( direct_3d(), 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Sound
	{
		sound_manager()->stop_all();

//		sound_manager()->load_music( "bgm", "tower" )->play( true );
		sound_manager()->load_music( "rain" )->play( true );
//		sound_manager()->load_music( "izakaya" )->play( false );

		sound_manager()->load( "turn" );
		sound_manager()->load( "clamber" );
		sound_manager()->load( "collision-wall" );
		sound_manager()->load( "jump" );
		sound_manager()->load( "fall", "turn" );
		sound_manager()->load( "super-jump" );
		sound_manager()->load( "land" );
		sound_manager()->load( "short-breath" );
		sound_manager()->load( "dead" );

		sound_manager()->load( "fin" );
	}

	// Player
	player_ = new Player();
	player_->set_input( input() );

	// Camera
	camera_ = new Camera();

	// Stage
	stage_ = new Stage( 1000, 1000 );
	player_->set_stage( stage_.get() );

	// generate_random_stage();

	// load_stage_file( "media/stage/quit" );
	// load_stage_file( "media/stage/stage-1-1" );
	load_stage_file( "media/stage/stage-1-2" );

	player_->position() = player_start_position_;

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF ) );
}

GamePlayScene::~GamePlayScene()
{
	// save_stage_file( "media/stage/quit" );

	font_.release();

	player_mesh_.release();
	shadow_mesh_.release();
	ground_mesh_.release();

	stage_.release();
	camera_.release();
	player_.release();
	
	sky_box_.release();

	box_.release();

	delete landscape_;

	delete enemy_mesh_;

	grid_object_manager()->clear();
	active_object_manager()->clear();
}

void GamePlayScene::generate_random_stage()
{
	grid_object_manager()->clear();

	GridData* building_a_grid = grid_data_manager()->load( "building-a" );
	GridData* building_b_grid = grid_data_manager()->load( "building-b" );
	GridData* house_a_grid = grid_data_manager()->load( "house-a" );
	
	GridData* road_grid = grid_data_manager()->load( "road" );
	GridData* road_curve_grid = grid_data_manager()->load( "road-curve" );

	GridData* tel_box_grid = grid_data_manager()->load( "tel-box" );

	const int x_space = 0;
	const int z_space = 0;

	for ( int d = 0; d < 300; d++ )
	{
		for ( int x = 0; x < 300; x++ )
		{
			GridData* grid_data = 0;
			bool tel_box = false;

			int y = 0;

			int random_value = common::random( 0, 10 );

			if ( random_value < 4 )
			{
				grid_data = building_a_grid;
			}
			else if ( random_value < 5 )
			{
				grid_data = building_b_grid;
			}
			else if ( random_value < 6 )
			{
				grid_data = house_a_grid;
			}
			else
			{
				if ( common::random( 0, 3 ) == 0 )
				{
					grid_data = road_grid;
				}

				if ( common::random( 0, 5 ) == 0 )
				{
					tel_box = true;
				}
			}

			if ( grid_data )
			{
				int r = common::random( 0, 3 ) * 90;
				int dx = x * ( 10 + x_space );
				int dy = y;
				int dz = d * ( 10 + z_space );

				if ( stage_->put( dx, dy, dz, r, grid_data ) )
				{
					grid_object_manager()->add_grid_object( new GridObject( dx, dy, dz, r, grid_data ) );

					if ( tel_box )
					{
						grid_object_manager()->add_grid_object( new GridObject( dx, dy, dz, r, tel_box_grid ) );
					}
				}
			}
		}
	}

	player_start_position_.x() = common::random( 0.f, 100.f );
	player_start_position_.y() = 3.f;
	player_start_position_.z() = common::random( 0.f, 100.f );
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

		if ( name == "player" )
		{
			ss >> player_start_position_.x() >> player_start_position_.y() >> player_start_position_.z();
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
			enemy->position().set( x, y, z );
			enemy->set_direction_degree( r );

			active_object_manager()->add_active_object( enemy );
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
 * メインループ処理
 *
 */
void GamePlayScene::update()
{
	if ( ! player_->is_dead() )
	{
		if ( input()->press( Input::B ) )
		{
			player_->step( +1 );
		}
		if ( input()->push( Input::A ) )
		{
			player_->jump();
		}
	}

	player_->add_direction_degree( input()->get_mouse_dx() * 90.f );

	camera_->rotate_degree_target().y() = player_->get_direction_degree();
	camera_->rotate_degree_target().x() = input()->get_mouse_y() * 90.f;

	player_->update();

	active_object_manager()->update();

	for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
	{
		ActiveObject* active_object = *i;

		if ( active_object->global_aabb().collision_detection( player_->global_aabb() ) )
		{
			// active_object->on_collision( player_ );
			// player_->on_collision( active_object );

			player_->kill();
		}
	}

	camera_->position() = player_->position() + vector3( 0.f, player_->get_eye_height(), 0.f );
	
	if ( player_->is_dead() )
	{
		camera_->rotate_degree_target().z() = 90.f;
		brightness = math::chase( brightness, -0.4f, 0.01f );

		if ( camera_->rotate_degree().z() == camera_->rotate_degree_target().z() && input()->push( Input::A ) )
		{
			camera_->rotate_degree().set( 0.f, 0.f, 0.f );
			camera_->rotate_degree_target().set( 0.f, 0.f, 0.f );

			player_->rebirth();
			player_->position() = player_start_position_;

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

	camera_->update();

	if ( input()->get_mouse_x() <= -1.f ) { input()->set_mouse_x( 0.f ); }
	if ( input()->get_mouse_x() >= +1.f ) { input()->set_mouse_x( 0.f ); }
}

/**
 * 描画
 */
void GamePlayScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );

	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
	direct_3d()->getEffect()->SetTechnique( technique );

	UINT pass_count = 0;

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", brightness ) );


	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->Begin( & pass_count, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );

	D3DXMATRIXA16 world;
	D3DXMATRIXA16 view;
	D3DXMATRIXA16 projection;
	D3DXMATRIXA16 WorldViewProjection;

	camera_->set_panorama_y_division( get_panorama_y_division() );

	static float a = 0.f;
	a += 0.02f;

	for ( int panorama_y = 0; panorama_y < get_panorama_y_division(); panorama_y++ )
	{
		D3DVIEWPORT9 view_port;
		view_port.X = 0;
		view_port.Y = get_height() / get_panorama_y_division() * panorama_y;
		view_port.Width	= get_width();
		view_port.Height = get_height() / get_panorama_y_division();
		view_port.MinZ = 0.f;
		view_port.MaxZ = 1.f;
	
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetViewport( & view_port ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xCC, 0xCC, 0xFF ), 1.f, 0 ) );

		vector3 look_at = camera_->get_look_at_part( panorama_y );
		vector3 up = camera_->get_up_part( panorama_y );
		
		D3DXMATRIXA16 r;
		D3DXMATRIXA16 s;
		D3DXMATRIXA16 t;
		
		D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & look_at ), reinterpret_cast< const D3DXVECTOR3* >( & up ) );
		D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() / get_panorama_y_division() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );
		
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
		const int gx = static_cast< int >( player_->position().x() );
		const int gy = static_cast< int >( player_->position().z() );

		D3DXMatrixTranslation( & t, static_cast< float >( gx / 10 * 10 ), 0.f, static_cast< float >( gy / 10 * 10 ) );
		world = t;

		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
		ground_mesh_->render();

//		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );		
		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );

		// Landscape
		if ( landscape_ )
		{
			D3DXMatrixTranslation( & t, player_->position().x(), 0.f, player_->position().z() );

			for ( int n = 0; n < 10; n++ )
			{
				float scale = 1.4f - ( n * 0.08f );
				D3DXMatrixRotationY( & r, n * 0.1f );
				D3DXMatrixScaling( & s,  scale, scale, scale );

				world = s * r * t;

				WorldViewProjection = world * view * projection;
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
				DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
				landscape_->render();
			}
		}

		DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
		
#if 1
		// GridObject
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
		{
			GridObject* grid_object = *i;

			const int max_length = 500;
			const int lod_length = 100;

			const int x_length = std::abs( static_cast< int >( player_->position().x() ) - grid_object->x() );
			const int y_length = std::abs( static_cast< int >( player_->position().y() ) - grid_object->y() );
			const int z_length = std::abs( static_cast< int >( player_->position().z() ) - grid_object->z() );

			if ( x_length >= max_length ) continue;
			if ( z_length >= max_length ) continue;
			
			const float offset = 0.05f;
			const float flicker = 0.f; // sin( grid_object->x() + grid_object->z() * 0.001f + a ) * 0.5f;

			D3DXMatrixRotationY( & r, math::degree_to_radian( static_cast< float >( grid_object->rotate_degree() ) ) );
			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), static_cast< float >( grid_object->y() + flicker + offset ), static_cast< float >( grid_object->z() ) );

			world = s * r * t;


			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			if ( x_length < lod_length && z_length < lod_length )
			{
				grid_object->mesh( 0 )->render();
			}
			else
			{
				grid_object->mesh( 1 )->render();
			}

			// Shadow
			/*
			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), 0.05f, static_cast< float >( grid_object->z() ) );
			world = s * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			shadow_mesh_->render();

			*/
		}
#endif // 0

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

		// Player ( Shadow )
		render_shadow( player_.get(), view * projection );

		// ActiveObject ( Shadow )
		for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
		{
			render_shadow( *i, view * projection );
		}

		// Player
		vector3 pos = player_->position();
		pos += -camera_->front() * 0.1f;

		D3DXMatrixRotationY( & r, math::degree_to_radian( camera_->rotate_degree().y() ) );
		D3DXMatrixTranslation( & t, pos.x(), pos.y() + 0.05f, pos.z() );

		world = r * t;
		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		player_mesh_->render();

		// ActiveObject
		for ( ActiveObjectManager::ActiveObjectList::const_iterator i = active_object_manager()->active_object_list().begin(); i != active_object_manager()->active_object_list().end(); ++i )
		{
			ActiveObject* active_object = *i;

			D3DXMatrixRotationY( & r, math::degree_to_radian( active_object->get_direction_degree() ) );
			D3DXMatrixTranslation( & t, active_object->position().x(), active_object->position().y() + 0.05f, active_object->position().z() );

			world = r * t;
			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			enemy_mesh_->render();
		}
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	/*
	std::string debug_text = "player : (" + 
		common::serialize( static_cast< int >( player_->position().x() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().y() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().z() ) ) + ")\n" +
		"mouse : ( " +
		common::serialize( input()->get_mouse_x() ) + ", " +
		common::serialize( input()->get_mouse_y() ) + ")\n( " +
		common::serialize( input()->get_mouse_dx() ) + ", " +
		common::serialize( input()->get_mouse_dy() ) + ")";

	font_->draw_text( 0, 24, debug_text.c_str(), D3DCOLOR_XRGB( 0, 0, 0 ) );
	*/

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
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
		D3DXMatrixTranslation( & t, active_object->position().x() , *i + 0.11f, active_object->position().z() );

		WorldViewProjection = r * t * after;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

		shadow_mesh_->render();
	}
}

} // namespace blue_sky
#include "GamePlayScene.h"

#include "App.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"

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

#include "matrix4x4.h"
#include "vector3.h"

#include <game/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

#include <fstream>
#include <sstream>

namespace blue_sky
{

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, player_( 0 )
	, camera_( 0 )
	, stage_( 0 )
	, player_mesh_( 0 )
	, shadow_mesh_( 0 )
	, ground_mesh_( 0 )
	, sky_box_( 0 )
	, box_( 0 )
	, panorama_y_division_( config()->get( "panorama_y_division", 1 ) )
{
	// Mesh
	player_mesh_ = new Direct3D9Mesh( direct_3d() );
	player_mesh_->load_x( "media/model/player.x" );

	shadow_mesh_ = new Direct3D9Mesh( direct_3d() );
	shadow_mesh_->load_x( "media/model/shadow.x" );

	ground_mesh_ = new Direct3D9Mesh( direct_3d() );
	ground_mesh_->load_x( "media/model/ground.x" );

	// SkyBox
	// sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box", "jpg" );
	sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box-star-2", "png" );


	// Box
	box_ = new Direct3D9Box( direct_3d(), 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Sound
	{
		sound_manager()->stop_all();

		sound_manager()->load_music( "bgm", "tower" )->play( true );
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

	// Camera
	camera_ = new Camera();

	// Stage
	stage_ = new Stage( 1000, 1000 );
	player_->set_stage( stage_ );

	// generate_random_stage();

	// load_stage_file( "media/stage/quit" );
	load_stage_file( "media/stage/stage-1-1" );

	player_->position() = player_start_position_;

	for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
	{
		GridObject* grid_object = *i;
		stage_->put( grid_object->x(), grid_object->y(), grid_object->z(), grid_object->grid_data() );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF ) );
}

GamePlayScene::~GamePlayScene()
{
	save_stage_file( "media/stage/quit" );

	delete player_mesh_;
	delete shadow_mesh_;
	delete ground_mesh_;

	delete stage_;
	delete camera_;
	delete player_;
	
	delete sky_box_;

	delete box_;
}

void GamePlayScene::generate_random_stage()
{
	grid_object_manager()->clear();

	GridData* building_a_grid_ = GridData::load_file( "media/object/building-a" );
	GridData* house_a_grid_ = GridData::load_file( "media/object/house-a" );
	
	GridData* road_grid_ = GridData::load_file( "media/object/road" );
	GridData* road_curve_grid_ = GridData::load_file( "media/object/road-curve" );

	const int x_space = 0;
	const int z_space = 0;

	int yy = -15;

	for ( int d = 0; d < 80; d++ )
	{
		for ( int x = 0; x < 20; x++ )
		{
			// grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), road_grid_ ) );
			// continue;

			int y = yy;

			if ( common::random( 0, 2 ) == 0 )
			{
				if ( d > 0 )
				{
					int r = common::random( 0, 3 );

					if ( r == 0 ) y -= 5;
					if ( r >= 2 ) y += 5;
				}

				y = 0;

				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), y, d * ( 10 + z_space ), building_a_grid_ ) );
			}
			else if ( common::random( 0, 2 ) == 0 )
			{
				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_ ) );
			}
			else
			{
				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), road_grid_ ) );
			}
		}

		yy += 5;
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
		else if ( name == "object" )
		{
			std::string grid_data_name;
			int x = 0, y = 0, z = 0;

			ss >> grid_data_name >> x >> y >> z;

			GridData* grid_data = grid_data_manager()->load( grid_data_name.c_str() );
			grid_object_manager()->add_grid_object( new GridObject( x, y, z, grid_data ) );
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
	if ( input()->press( Input::UP    ) ) { player_->step( +1 ); }
	if ( input()->press( Input::DOWN  ) ) { player_->step( -1 ); }
	if ( input()->press( Input::LEFT  ) ) { player_->side_step( -1 ); }
	if ( input()->press( Input::RIGHT ) ) { player_->side_step( +1 ); }

	if ( input()->press( Input::B ) )
	{
		if ( camera_->step_rotate_x_available() )
		{
			if ( input()->press( Input::UP    ) ) { camera_->step_rotate_x( -1 ); sound_manager()->get_sound( "turn" )->play( false ); }
			if ( input()->press( Input::DOWN  ) ) { camera_->step_rotate_x( +1 ); sound_manager()->get_sound( "turn" )->play( false ); }
		}

		if ( player_->is_turn_available() )
		{
			if ( input()->push( Input::LEFT  ) ) { player_->turn( -1 ); }
			if ( input()->push( Input::RIGHT ) ) { player_->turn( +1 ); }
		}

//		if ( input()->push( Input::LEFT  ) ) { camera_->step_rotate_y( -1 ); }
//		if ( input()->push( Input::RIGHT ) ) { camera_->step_rotate_y( +1 ); }
	}

	if ( input()->push( Input::L ) && player_->is_turn_available() ) { player_->turn( -1 ); }
	if ( input()->push( Input::R ) && player_->is_turn_available() ) { player_->turn( +1 ); }
	

	if ( input()->push( Input::A ) ) { player_->is_jumping() ? player_->fall() : player_->jump(); }

	if ( input()->push( Input::X ) )
	{
		App::GetInstance()->set_full_screen( ! App::GetInstance()->is_full_screen() );
		direct_3d()->set_full_screen( App::GetInstance()->is_full_screen() );
	}
	
	if ( input()->push( Input::Y ) )
	{
		sound_manager()->set_enabled( ! sound_manager()->is_enabled() );

		if ( sound_manager()->is_enabled() )
		{
			sound_manager()->stop_all();
			sound_manager()->get_sound( "fin" )->play( false );

			set_next_scene( "title" );
		}
	}

	player_->update();

	camera_->position() = player_->position() + vector3( 0.f, 1.5f, 0.f );
	camera_->rotate_degree_target().y() = player_->direction_degree();
	
	if ( player_->is_dead() )
	{
		// camera_->position() 
		camera_->rotate_degree_target().z() = 120.f;

		if ( camera_->rotate_degree() == camera_->rotate_degree_target() )
		{
			camera_->rotate_degree().set( 0.f, 0.f, 0.f );
			camera_->rotate_degree_target().set( 0.f, 0.f, 0.f );

			player_->rebirth();
			player_->position() = player_start_position_;
		}
	}

	camera_->update();


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

	direct_3d()->getEffect()->Begin( & pass_count, 0 );
	direct_3d()->getEffect()->BeginPass( 0 );

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
		
		// SkyBox
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );
		D3DXMatrixTranslation( & t, camera_->position().x(), camera_->position().y(), camera_->position().z() );

		world = s * t;
		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
		sky_box_->render();

		// Ground
		const int gx = static_cast< int >( player_->position().x() );
		const int gy = static_cast< int >( player_->position().z() );

		D3DXMatrixTranslation( & t, static_cast< float >( gx / 10 * 10 ), 0.f, static_cast< float >( gy / 10 * 10 ) );
		world = t;

		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
		DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );
		// ground_mesh_->render();

		// GridObject
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
		{
			GridObject* grid_object = *i;

			const int max_length = 150;

			if ( std::abs( static_cast< int >( player_->position().x() ) - grid_object->x() ) >= max_length ) continue;
			if ( std::abs( static_cast< int >( player_->position().z() ) - grid_object->z() ) >= max_length ) continue;
			
			const float offset = 0.05f;
			float flicker = 0.f; // sin( grid_object->x() + grid_object->z() * 0.001f + a ) * 0.1f;

			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), static_cast< float >( grid_object->y() + flicker + offset ), static_cast< float >( grid_object->z() ) );
			world = s * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			grid_object->mesh()->render();


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

		// Player ( Shadow )
		std::set< float > grid_cell_height_set;

		grid_cell_height_set.insert( player_->get_floor_cell_center().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_left_front().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_right_front().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_left_back().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_right_back().height() );

		for ( std::set< float >::iterator i = grid_cell_height_set.begin(); i != grid_cell_height_set.end(); ++i )
		{
			D3DXMatrixTranslation( & t, player_->position().x(), *i + 0.11f, player_->position().z() );
			world = r * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
			DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

			shadow_mesh_->render();
		}
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
}

} // namespace blue_sky
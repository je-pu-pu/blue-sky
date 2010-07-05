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


namespace blue_sky
{

/// @todo �܂Ƃ߂�
LPDIRECT3DVERTEXSHADER9 vertex_shader_;
LPDIRECT3DPIXELSHADER9 pixel_shader_;

LPD3DXCONSTANTTABLE vs_constant_table;
LPD3DXCONSTANTTABLE ps_constant_table;

GamePlayScene::GamePlayScene( const GameMain* game_main )
	: Scene( game_main )
	, panorama_y_division_( config()->get( "panorama_y_division", 1 ) )
{
	// Mesh
	shadow_mesh_ = new Direct3D9Mesh( direct_3d() );
	shadow_mesh_->load_x( "media/model/shadow.x" );

	ground_mesh_ = new Direct3D9Mesh( direct_3d() );
	ground_mesh_->load_x( "media/model/ground.x" );

	// SkyBox
	sky_box_ = new Direct3D9SkyBox( direct_3d(), "sky-box", "jpg" );


	// Box
	box_ = new Direct3D9Box( direct_3d(), 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Sound
	{
		sound_manager()->stop_all();

		Sound* bgm = sound_manager()->load( "bgm", "media/music/tower.ogg" );
		bgm->play( true );

		Sound* rain = sound_manager()->load( "rain", "media/music/rain.ogg" );
		rain->play( true );

		Sound* izakaya = sound_manager()->load( "izakaya", "media/music/izakaya.ogg" );
//		izakaya->set_volume( 0.8f );
		izakaya->play( false );

		Sound* turn = sound_manager()->load( "turn", "media/sound/turn.ogg" );

		Sound* collision_wall = sound_manager()->load( "collision_wall", "media/sound/collision_wall.ogg" );
		collision_wall->set_volume( 0.9f );
		collision_wall->set_speed( 0.3f );

		Sound* jump = sound_manager()->load( "jump", "media/sound/jump.ogg" );
		jump->set_speed( 0.5f );

		Sound* land = sound_manager()->load( "land", "media/sound/land.ogg" );
		land->set_speed( 0.5f );

		Sound* fin = sound_manager()->load( "fin", "media/sound/fin.ogg" );
	}

	// Player
	player_ = new Player();
	player_->position().set( 2.f, 300.f, 2.f );

	// Camera
	camera_ = new Camera();

	const char* vs_profile = "vs_2_0"; // D3DXGetVertexShaderProfile( direct_3d()->getDevice() );
	const char* ps_profile = "ps_2_0"; // D3DXGetPixelShaderProfile( direct_3d()->getDevice() );

	LPD3DXBUFFER error_message_buffer;

	// Vertex Shader
	LPD3DXBUFFER vs_buffer;

	if ( FAILED( D3DXCompileShaderFromFile( "test.fx", 0, 0, "vs_main", vs_profile, 0, & vs_buffer, & error_message_buffer, & vs_constant_table ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_message_buffer->GetBufferPointer() ) );
	}

	direct_3d()->getDevice()->CreateVertexShader( static_cast< DWORD* >( vs_buffer->GetBufferPointer() ), & vertex_shader_ );

	// Pixel Shader
	LPD3DXBUFFER ps_buffer;

	if ( FAILED( D3DXCompileShaderFromFile( "test.fx", 0, 0, "ps_main", ps_profile, 0, & ps_buffer, & error_message_buffer, & ps_constant_table ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_message_buffer->GetBufferPointer() ) );
	}
	
	direct_3d()->getDevice()->CreatePixelShader( static_cast< DWORD* >( ps_buffer->GetBufferPointer() ), & pixel_shader_ );

//	app_->setTitle( ( std::string( app_->getTitle() ) + " : " + vs_profile + " : " + ps_profile ).c_str() );

	// Stage
	stage_ = new Stage( 1000, 1000 );
	player_->set_stage( stage_ );

	// Building
	grid_object_manager()->clear();

	GridData* building_a_grid_ = GridData::load_file( "media/object/building-a" );
	GridData* house_a_grid_ = GridData::load_file( "media/object/house-a" );

	const int x_space = 1;
	const int z_space = 1;

	int yy = -15;

	for ( int d = 0; d < 80; d++ )
	{
		for ( int x = 0; x < 20; x++ )
		{
			// grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), building_a_grid_, building_a_mesh_ ) );
			// grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_, house_a_mesh_ ) );
			// continue;

			/*
			if ( common::random( 0, 1 ) == 0 )
			{
				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_, house_a_mesh_ ) );
			}
			else if ( common::random( 0, 1 ) == 0 )
			{
				const int y = d == 0 ? 0 : -15 + common::random( 0, 3 ) * 5;
				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), y, d * ( 10 + z_space ), building_a_grid_, building_a_mesh_ ) );
			}
			continue;
			*/

			int y = yy;

			if ( common::random( 0, 1 ) == 0 )
			{
				if ( d > 0 )
				{
					int r = common::random( 0, 3 );

					if ( r == 0 ) y -= 5;
					if ( r >= 2 ) y += 5;
				}

				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), y, d * ( 10 + z_space ), building_a_grid_ ) );
			}
			else if ( common::random( 0, 1 ) == 0 )
			{
				grid_object_manager()->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_ ) );
			}
		}

		yy += 5;
	}

	for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
	{
		GridObject* grid_object = *i;
		stage_->put( grid_object->x(), grid_object->y(), grid_object->z(), grid_object->grid_data() );
	}
}

GamePlayScene::~GamePlayScene()
{
	delete shadow_mesh_;
	delete ground_mesh_;

	delete stage_;
	delete camera_;
	delete player_;
	
	delete sky_box_;

	delete box_;
}

/**
 * ���C�����[�v����
 *
 */
void GamePlayScene::update()
{
	if ( input()->press( Input::B ) ) { player_->step( +1 ); }

	if ( input()->push( Input::LEFT  ) && player_->is_turn_available() ) { player_->turn( -1 ); }
	if ( input()->push( Input::RIGHT ) && player_->is_turn_available() ) { player_->turn( +1 ); }

	if ( input()->push( Input::A ) ) { player_->is_jumping() ? player_->fall() : player_->jump(); }

	if ( input()->push( Input::X ) )
	{
		App::GetInstance()->set_full_screen( ! App::GetInstance()->is_full_screen() );
		direct_3d()->set_full_screen( App::GetInstance()->is_full_screen() );

		// sound_manager()->set_enabled( ! sound_manager()->is_enabled() );
	}
	
	if ( input()->push( Input::Y ) )
	{
		sound_manager()->stop_all();
		sound_manager()->get_sound( "fin" )->play( false );

		set_next_scene( "title" );
	}

	player_->update();

	Sound* bgm = sound_manager()->get_sound( "bgm" );
	Sound* izakaya = sound_manager()->get_sound( "izakaya" );

	if ( player_->is_jumping() )
	{
//		bgm->set_speed( 1.41421356f );
//		izakaya->set_speed( 1.41421356f );
		 izakaya->set_volume( 0.8f );
	}
	else
	{
		bgm->set_speed( 1.f );
		izakaya->set_speed( 1.f );
		izakaya->set_volume( 1.f );
	}

	camera_->position() = player_->position() + vector3( 0.f, 1.5f, 0.f );
	
	const float under_view_max_speed = 0.1f;
	static float under_view_speed = 0.f;

	if ( player_->is_jumping() || input()->press( Input::B ) )
	{
		under_view_speed += 0.02f;
	}
	else
	{
		under_view_speed -= 0.01f;
	}

	under_view_speed = math::clamp( under_view_speed, -under_view_max_speed, under_view_max_speed );
	
	camera_->set_under_view_rate( camera_->get_under_view_rate() + under_view_speed );
	// camera_->set_under_view_rate( player_->position().y() / 50.f );

	camera_->set_direction_degree_target( player_->direction_degree() );

	camera_->update();
}

/**
 * �`��
 */
void GamePlayScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );

	direct_3d()->getDevice()->SetVertexShader( vertex_shader_ );
	direct_3d()->getDevice()->SetPixelShader( pixel_shader_ );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF ) );


	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGCOLOR, 0xFFEEEEFF ) );
	
	float Start   = 1.f;    // For linear mode
	float End     = 200.f;
	// static float fog_density_a = 0.f;
	// fog_density_a += 0.001f;
	// float fog_density = sin( fog_density_a );
	// End = 2.f + std::abs( sin( fog_density_a ) * 500.f );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGSTART, *(DWORD *)(&Start)) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGEND,   *(DWORD *)(&End)) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR ) );
//	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_RANGEFOGENABLE, TRUE ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ) ); 

	// DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_EXP2 ) ); 
	// DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetRenderState( D3DRS_FOGDENSITY, * ( DWORD* )( & fog_density ) ) );

	direct_3d()->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

	D3DXMATRIXA16 world;
	D3DXMATRIXA16 view;
	D3DXMATRIXA16 projection;
	D3DXMATRIXA16 WorldViewProjection;

	camera_->set_panorama_y_division( get_panorama_y_division() );

	static float fog = 0.f;
	fog += 0.001f;
	vs_constant_table->SetFloat( direct_3d()->getDevice(), "fog", sin( fog ) );

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
		
		D3DXMATRIXA16 s;
		D3DXMATRIXA16 t;
		
		D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & look_at ), reinterpret_cast< const D3DXVECTOR3* >( & up ) );
		D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() / get_panorama_y_division() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );
		
		// SkyBox
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );
		D3DXMatrixTranslation( & t, camera_->position().x(), camera_->position().y(), camera_->position().z() );

		world = s * t;
		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );
		sky_box_->render();

		// Ground
		const int gx = static_cast< int >( player_->position().x() );
		const int gy = static_cast< int >( player_->position().z() );

		D3DXMatrixTranslation( & t, static_cast< float >( gx / 10 * 10 ), 0.f, static_cast< float >( gy / 10 * 10 ) );
		world = t;

		WorldViewProjection = world * view * projection;
		DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );
		ground_mesh_->render();

		// GridObject
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager()->grid_object_list().begin(); i != grid_object_manager()->grid_object_list().end(); ++i )
		{
			GridObject* grid_object = *i;

			const int max_length = 200;

			if ( std::abs( static_cast< int >( player_->position().x() ) - grid_object->x() ) >= max_length ) continue;
			if ( std::abs( static_cast< int >( player_->position().z() ) - grid_object->z() ) >= max_length ) continue;
			
			
			float flicker = sin( grid_object->x() + grid_object->z() * 0.001f + a ) * 0.1f;

			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), static_cast< float >( grid_object->y() + flicker ), static_cast< float >( grid_object->z() ) );
			world = s * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );

			grid_object->mesh()->render();


			// Shadow
			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), 0.05f, static_cast< float >( grid_object->z() ) );
			world = s * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );

			shadow_mesh_->render();
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

		// Player ( Shadow )
		std::set< float > grid_cell_height_set;

		grid_cell_height_set.insert( player_->get_floor_cell_center().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_left_front().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_right_front().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_left_back().height() );
		grid_cell_height_set.insert( player_->get_floor_cell_right_back().height() );

		// D3DXMATRIXA16 r;
		// D3DXMatrixRotationY( & r, math::degree_to_radian( common::random( 0.f, 0.3f ) ) );

		for ( std::set< float >::iterator i = grid_cell_height_set.begin(); i != grid_cell_height_set.end(); ++i )
		{
			D3DXMatrixTranslation( & t, player_->position().x(), *i + 0.11f, player_->position().z() );
			world = t;
			// world = r * t;

			WorldViewProjection = world * view * projection;
			DIRECT_X_FAIL_CHECK( vs_constant_table->SetMatrix( direct_3d()->getDevice(), "WorldViewProjection", & WorldViewProjection ) );
			shadow_mesh_->render();
		}
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
	
	HRESULT hr = direct_3d()->getDevice()->Present( NULL, NULL, NULL, NULL );

	if ( hr == D3DERR_DEVICELOST )
	{
		direct_3d()->reset();
	}
	else
	{
		DIRECT_X_FAIL_CHECK( hr );
	}
}

} // namespace blue_sky
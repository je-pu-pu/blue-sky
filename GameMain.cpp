/**
 *
 *
 */


#include "GameMain.h"

#include "App.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"
#include "Building.h"
#include "House.h"

#include "Direct3D9Mesh.h"
#include "Direct3D9Box.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include "Input.h"
#include "SoundManager.h"
#include "GridObjectManager.h"
#include "GridObject.h"
#include "GridCell.h"

#include "OggVorbisFile.h"

#include "matrix4x4.h"
#include "vector3.h"

#include <game/Sound.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

Direct3D9Mesh* mesh_ = 0;
Direct3D9Mesh* building_a_mesh_ = 0;
Direct3D9Mesh* house_a_mesh_ = 0;
Direct3D9Mesh* shadow_mesh_ = 0;
Direct3D9Mesh* ground_mesh_ = 0;

Direct3D9Box* box_ = 0;

LPD3DXCONSTANTTABLE vs_constant_table;
LPD3DXCONSTANTTABLE ps_constant_table;

namespace blue_sky
{

Player* player_ = 0;
Camera* camera_ = 0;
Stage* stage_ = 0;
Building* building_a_grid_ = 0;
House* house_a_grid_ = 0;

using game::Sound;

//■コンストラクタ
GameMain::GameMain()
	: direct_3d_( 0 )
	, input_( 0 )
	, sound_manager_( 0 )
	, grid_object_manager_( 0 )
	, Width( 0 )
	, Height( 0 )
{
	CApp *app = CApp::GetInstance();
	Width = app->GetWidth();
	Height = app->GetHeight();

	//ランダマイズ
	srand( timeGetTime() );

	// Direct3D 
	direct_3d_ = new Direct3D9( app->GetWindowHandle() );

	// Mesh
	mesh_ = new Direct3D9Mesh( direct_3d_ );
	// mesh_->load_x( "media/model/hoge.x" );
	// mesh_->load_x( "media/model/blue-sky-building-a-field.x" );

	building_a_mesh_ = new Direct3D9Mesh( direct_3d_ );
	building_a_mesh_->load_x( "media/model/building-a.x" );

	house_a_mesh_ = new Direct3D9Mesh( direct_3d_ );
	house_a_mesh_->load_x( "media/model/house-a.x" );

	shadow_mesh_ = new Direct3D9Mesh( direct_3d_ );
	shadow_mesh_->load_x( "media/model/shadow.x" );

	ground_mesh_ = new Direct3D9Mesh( direct_3d_ );
	ground_mesh_->load_x( "media/model/ground.x" );

	// Box
	box_ = new Direct3D9Box( direct_3d_, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	// Input
	input_ = new Input();

	// Sound
	{
		sound_manager_ = new SoundManager( app->GetWindowHandle() );
		Sound* bgm = sound_manager_->load( "bgm", "media/music/tower.ogg" );
		bgm->play( true );

		Sound* rain = sound_manager_->load( "rain", "media/music/rain.ogg" );
		rain->play( true );

		Sound* izakaya = sound_manager_->load( "izakaya", "media/music/izakaya.ogg" );
//		izakaya->set_volume( 0.8f );
		izakaya->play( false );

		Sound* turn = sound_manager_->load( "turn", "media/sound/turn.ogg" );

		Sound* collision_wall = sound_manager_->load( "collision_wall", "media/sound/collision_wall.ogg" );
		collision_wall->set_volume( 0.7f );
		collision_wall->set_speed( 0.3f );

		Sound* jump = sound_manager_->load( "jump", "media/sound/jump.ogg" );
		jump->set_speed( 0.5f );

		Sound* land = sound_manager_->load( "land", "media/sound/land.ogg" );
		land->set_speed( 0.5f );

		Sound* fin = sound_manager_->load( "fin", "media/sound/fin.ogg" );
	}

	// Player
	player_ = new Player();
	player_->position().set( 2.f, 10.f, 2.f );

	// Camera
	camera_ = new Camera();

	const char* vs_profile = D3DXGetVertexShaderProfile( direct_3d_->getDevice() );
	const char* ps_profile = D3DXGetPixelShaderProfile( direct_3d_->getDevice() );

	app->setTitle( ( std::string( app->getTitle() ) + " : " + vs_profile + " : " + ps_profile ).c_str() );

	LPD3DXBUFFER error_message_buffer;

	// Vertex Shader
	LPDIRECT3DVERTEXSHADER9 vertex_shader_;
	LPD3DXBUFFER vs_buffer;

	if ( FAILED( D3DXCompileShaderFromFile( "test.fx", 0, 0, "vs_main", vs_profile, 0, & vs_buffer, & error_message_buffer, & vs_constant_table ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_message_buffer->GetBufferPointer() ) );
	}

	direct_3d_->getDevice()->CreateVertexShader( static_cast< DWORD* >( vs_buffer->GetBufferPointer() ), & vertex_shader_ );
	direct_3d_->getDevice()->SetVertexShader( vertex_shader_ );

	// Pixel Shader
	LPDIRECT3DPIXELSHADER9 pixel_shader_;
	LPD3DXBUFFER ps_buffer;

	if ( FAILED( D3DXCompileShaderFromFile( "test.fx", 0, 0, "ps_main", ps_profile, 0, & ps_buffer, & error_message_buffer, & ps_constant_table ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_message_buffer->GetBufferPointer() ) );
	}
	
	direct_3d_->getDevice()->CreatePixelShader( static_cast< DWORD* >( ps_buffer->GetBufferPointer() ), & pixel_shader_ );
	direct_3d_->getDevice()->SetPixelShader( pixel_shader_ );

	// Stage
	stage_ = new Stage( 1000, 1000 );
	player_->set_stage( stage_ );

	// Building
	building_a_grid_ = new Building( 10, 10 );

	house_a_grid_ = new House( 8, 6 );

	grid_object_manager_ = new GridObjectManager();

	const int x_space = 1;
	const int z_space = 1;

	for ( int d = 0; d < 10; d++ )
	{
		for ( int x = 0; x < 10; x++ )
		{
			// grid_object_manager_->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), building_a_grid_, building_a_mesh_ ) );
			// grid_object_manager_->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_, house_a_mesh_ ) );
			// continue;

			if ( common::random( 0, 1 ) == 0 )
			{
				grid_object_manager_->add_grid_object( new GridObject( x * ( 10 + x_space ), 0, d * ( 10 + z_space ), house_a_grid_, house_a_mesh_ ) );
			}
			else
			{
				const int y = d == 0 ? 0 : -15 + common::random( 0, 3 ) * 5;
				grid_object_manager_->add_grid_object( new GridObject( x * ( 10 + x_space ), y, d * ( 10 + z_space ), building_a_grid_, building_a_mesh_ ) );
			}
			continue;

			if ( common::random( 0, 1 ) == 0 )
			{
				const int y = d == 0 ? 0 : -15 + common::random( 0, 3 ) * 5;
				grid_object_manager_->add_grid_object( new GridObject( x * ( 10 + x_space ), y, d * ( 10 + z_space ), building_a_grid_, building_a_mesh_ ) );
			}
		}
	}

	for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager_->grid_object_list().begin(); i != grid_object_manager_->grid_object_list().end(); ++i )
	{
		GridObject* grid_object = *i;
		stage_->put( grid_object->x(), grid_object->y(), grid_object->z(), grid_object->grid_data() );
	}

	/*
	IGraphBuilder

	CoInitialize( 0 );
	CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, static_cast< void** >( m_cpGraph.ToCreator() ) );
	*/
}

//■デストラクタ
GameMain::~GameMain()
{
	delete building_a_grid_;
	delete building_a_mesh_;

	delete shadow_mesh_;
	delete ground_mesh_;

	delete stage_;
	delete camera_;
	delete player_;

	delete mesh_;
	
	delete box_;

	delete direct_3d_;

	delete input_;

	delete sound_manager_;
}

static int fps = 0, last_fps = 0;

/**
 * メインループ処理
 *
 */
void GameMain::update()
{
	MainLoop.WaitTime = 16;
	MainLoop.WaitTime = 0;

	//秒間50フレームを保持
	if ( ! MainLoop.Loop() )
	{
		return;
	}

	static int sec = 0;
	
	if ( timeGetTime() / 1000 != sec )
	{
		sec = timeGetTime() / 1000;

		last_fps = fps;
		fps = 0;
	}

	fps++;
	
	input_->update();

	if ( input_->press( Input::LEFT  ) ) { player_->side_step( -1 ); }
	if ( input_->press( Input::RIGHT ) ) { player_->side_step( +1 ); }
	if ( input_->press( Input::UP    ) ) { player_->step( +1 ); }
	if ( input_->press( Input::DOWN  ) ) { player_->step( -1 ); }
	
	if ( input_->push( Input::L ) ) { player_->turn( -1 ); }
	if ( input_->push( Input::R ) ) { player_->turn( +1 ); }

	if ( input_->push( Input::A ) ) { player_->is_jumping() ? player_->fall() : player_->jump(); }
//	if ( input_->push( Input::B ) ) { player_->fall(); }

	if ( input_->push( Input::X ) ) { sound_manager_->set_enabled( ! sound_manager_->is_enabled() ); }

	if ( input_->push( Input::Y ) ) { sound_manager_->stop_all(); sound_manager_->get_sound( "fin" )->play( false ); }

	player_->update();

	Sound* bgm = sound_manager_->get_sound( "bgm" );
	Sound* izakaya = sound_manager_->get_sound( "izakaya" );

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

	if ( input_->press( Input::B ) )
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

	sound_manager_->update();

	render();
}

/**
 * 描画
 */
void GameMain::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xEE, 0xEE, 0xFF ), 1.f, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->BeginScene() );

//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF ) );


	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR ) );
	/*
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_XRGB( 255, 255, 255 ) ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ) );
	
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_RANGEFOGENABLE, TRUE ) );

	float Start = 0.1f;
	float End = 0.5f;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGSTART, *(DWORD *)(&Start)) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGEND,   *(DWORD *)(&End)) );

	direct_3d_->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	*/

	const int panorama_y_division = 10;

	camera_->set_panorama_y_division( panorama_y_division );

	for ( int panorama_y = 0; panorama_y < panorama_y_division; panorama_y++ )
	{
		D3DVIEWPORT9 view_port;
		view_port.X = 0;
		view_port.Y = Height / panorama_y_division * panorama_y;
		view_port.Width	= Width;
		view_port.Height = Height / panorama_y_division;
		view_port.MinZ = 0.f;
		view_port.MaxZ = 1.f;
	
		direct_3d_->getDevice()->SetViewport( & view_port );

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xCC, 0xCC, 0xFF ), 1.f, 0 ) );

		D3DXMATRIXA16 world;
		D3DXMATRIXA16 view;
		D3DXMATRIXA16 projection;
		D3DXMATRIXA16 WorldViewProjection;

		vector3 look_at = camera_->get_look_at_part( panorama_y );
		vector3 up = camera_->get_up_part( panorama_y );
		
		D3DXMATRIXA16 s;
		D3DXMatrixScaling( & s, 10.f, 10.f, 10.f );

		D3DXMATRIXA16 t;
		D3DXMatrixTranslation( & t, 50.f, 0.f, 0.f );
		
		world = s * t;
		
		D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & look_at ), reinterpret_cast< const D3DXVECTOR3* >( & up ) );

		D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() / panorama_y_division ), 720.f / ( 480.f / panorama_y_division ), 0.05f, 600.f );

		// Mesh
		/*
		WorldViewProjection = world * view * projection;
		vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

		mesh_->render();
		*/

		// Ground
		const int gx = static_cast< int >( player_->position().x() );
		const int gy = static_cast< int >( player_->position().z() );

		D3DXMatrixTranslation( & t, static_cast< float >( gx / 10 * 10 ), 0.f, static_cast< float >( gy / 10 * 10 ) );
		world = t;

		WorldViewProjection = world * view * projection;
		vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );
		ground_mesh_->render();

		// GridObject
		for ( GridObjectManager::GridObjectList::iterator i = grid_object_manager_->grid_object_list().begin(); i != grid_object_manager_->grid_object_list().end(); ++i )
		{
			GridObject* grid_object = *i;

			const int max_length = 200;

			if ( std::abs( static_cast< int >( player_->position().x() ) - grid_object->x() ) >= max_length ) continue;
			if ( std::abs( static_cast< int >( player_->position().z() ) - grid_object->z() ) >= max_length ) continue;
			
			D3DXMatrixTranslation( & t, static_cast< float >( grid_object->x() ), static_cast< float >( grid_object->y() ), static_cast< float >( grid_object->z() ) );
			world = s * t;

			WorldViewProjection = world * view * projection;
			vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

			grid_object->mesh()->render();
		}

		// Box
		/*
		direct_3d_->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

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
					vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

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
			D3DXMatrixTranslation( & t, player_->position().x(), *i + 0.03f, player_->position().z() );
			world = t;
			// world = r * t;

			WorldViewProjection = world * view * projection;
			vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

			shadow_mesh_->render();
		}
	}

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->EndScene() );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL ) );

	// Debug
	std::string debug_text;
	debug_text = std::string( "blue-sky | FPS : " ) + common::serialize( last_fps ) + ", player : (" + 
		common::serialize( static_cast< int >( player_->position().x() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().y() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().z() ) ) + ")";
	CApp::GetInstance()->setTitle( debug_text.c_str() );
}

} // namespace blue_sky
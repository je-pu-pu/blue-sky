/**
 *
 *
 */


#include "GameMain.h"

#include "App.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"

#include "Direct3D9Mesh.h"
#include "Direct3D9Box.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include "Input.h"
#include "SoundManager.h"

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
Direct3D9Box* box_ = 0;
Direct3D9Box* player_shadow_box_ = 0;

LPD3DXCONSTANTTABLE vs_constant_table;
LPD3DXCONSTANTTABLE ps_constant_table;

namespace blue_sky
{

Player* player_ = 0;
Camera* camera_ = 0;
Stage* stage_ = 0;

using game::Sound;

//■コンストラクタ
GameMain::GameMain()
	: direct_3d_( 0 )
	, input_( 0 )
	, sound_manager_( 0 )
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
	// mesh_->loadX( "media/model/hoge.x" );
	mesh_->loadX( "media/model/blue-sky-building-a-field.x" );

	// Box
	box_ = new Direct3D9Box( direct_3d_, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	player_shadow_box_ = new Direct3D9Box( direct_3d_, 1.f, 0.1f, 1.f, D3DCOLOR_RGBA( 0, 0, 0, 127 ) );

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

		Sound* jump = sound_manager_->load( "jump", "media/sound/jump.ogg" );
		jump->set_speed( 0.5f );

		Sound* land = sound_manager_->load( "land", "media/sound/land.ogg" );
		land->set_speed( 0.5f );
	}

	// Player
	player_ = new Player();
	player_->position().set( 50.f, 10.f, 0.f );

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
	stage_ = new Stage( 100, 100 );
}

//■デストラクタ
GameMain::~GameMain()
{
	delete stage_;
	delete camera_;
	delete player_;

	delete mesh_;
	
	delete box_;
	delete player_shadow_box_;

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

	if ( input_->push( Input::A ) ) { player_->jump(); }

	player_->set_floor_height( 20 ); // stage_->map_chip( static_cast< int >( player_->position().x() ), static_cast< int >( player_->position().z() ) ) ); 
	player_->update();

	Sound* bgm = sound_manager_->get_sound( "bgm" );
	Sound* izakaya = sound_manager_->get_sound( "izakaya" );

	if ( player_->is_jumping() )
	{
		bgm->set_speed( 1.41421356f );
		izakaya->set_speed( 1.41421356f );
		izakaya->set_volume( 0.8f );
	}
	else
	{
		bgm->set_speed( 1.f );
		izakaya->set_speed( 1.f );
		izakaya->set_volume( 1.f );
	}

	camera_->position() = player_->position() + vector3( 0.f, 1.5f, 0.f );
	camera_->set_front_target( player_->front() );
	
	const float under_view_max_speed = 0.1f;
	static float under_view_speed = 0.f;

	if ( player_->is_jumping() || input_->press( Input::B ) )
	{
		under_view_speed += 0.02f;
	}
	else
	{
		under_view_speed -= 0.004f;
	}

	under_view_speed = math::clamp( under_view_speed, -under_view_max_speed, under_view_max_speed );
	
	camera_->set_under_view_rate( camera_->get_under_view_rate() + under_view_speed );
	camera_->update();

	sound_manager_->update();

	render();
}

/**
 * 描画
 */
void GameMain::render()
{
	D3DXMATRIXA16 world, t;
	D3DXMatrixIdentity( & world );
	D3DXMatrixScaling( & world, 10.f, 10.f, 10.f );
	D3DXMatrixTranslation( & t, 50.f, 0.f, 0.f );
	world *= t;
//	D3DXMatrixRotationY( & world, timeGetTime() / 1000.f );
    direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

	D3DXMATRIXA16 view;
	D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->look_at() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
	direct_3d_->getDevice()->SetTransform( D3DTS_VIEW, & view );

	D3DXMATRIXA16 projection;
	D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() ), 720.f / 480.f, 0.01f, 300.f );
	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, & projection );

	D3DXMATRIX WorldViewProjection = world * view * projection;
	vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xEE, 0xEE, 0xFF ), 1.f, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->BeginScene() );

//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF ) );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_XRGB( 255, 255, 255 ) ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR ) );
//	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_RANGEFOGENABLE, TRUE ) );

	float Start = 0.1f;
	float End = 0.5f;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGSTART, *(DWORD *)(&Start)) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_FOGEND,   *(DWORD *)(&End)) );

	direct_3d_->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

	mesh_->render();

	/*
	direct_3d_->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	
	box_->ready();

	for ( int z = 0; z < stage_->depth(); z++ )
	{
		for ( int x = 0; x < stage_->width(); x++ )
		{
			int y = stage_->map_chip( x, z );
			
			if ( y > 0 )
			{
				D3DXMATRIXA16 world;
				D3DXMatrixTranslation( & world, x + 0.5f, y - 0.5f, z + 0.5f );
				direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

				D3DXMATRIX WorldViewProjection = world * view * projection;
				vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

				box_->render();
			}
		}
	}
	*/

	D3DXMatrixTranslation( & world, player_->position().x(), player_->get_floor_height(), player_->position().z() );
	direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

	WorldViewProjection = world * view * projection;
	vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

	player_shadow_box_->ready();
	player_shadow_box_->render();


	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->EndScene() );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL ) );

	// Debug
	std::string debug_text;
	debug_text = std::string( "blue-sky | FPS : " ) + common::serialize( last_fps ) + ", player : " + common::serialize( player_->position().y() );
	CApp::GetInstance()->setTitle( debug_text.c_str() );
}

} // namespace blue_sky
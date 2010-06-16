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

#include "DirectSound.h"
#include "DirectSoundBuffer.h"

#include "matrix4x4.h"
#include "vector3.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

using blue_sky::Player;
using blue_sky::Camera;
using blue_sky::Stage;

Direct3D9Mesh* mesh_ = 0;
Direct3D9Box* box_ = 0;
Direct3D9Box* player_shadow_box_ = 0;

DirectSoundBuffer* direct_sound_buffer_ = 0;

Player* player_ = 0;
Camera* camera_ = 0;
Stage* stage_ = 0;

LPD3DXCONSTANTTABLE vs_constant_table;
LPD3DXCONSTANTTABLE ps_constant_table;

//■コンストラクタ
CGameMain::CGameMain()
	: direct_3d_( 0 )
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
	mesh_->loadX( "hoge.x" );

	// Box
	box_ = new Direct3D9Box( direct_3d_, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x00 ) );

	player_shadow_box_ = new Direct3D9Box( direct_3d_, 1.f, 0.1f, 1.f, D3DCOLOR_RGBA( 0, 0, 0, 127 ) );

	// DirectSound
	direct_sound_ = new DirectSound( app->GetWindowHandle() );
	direct_sound_buffer_ = direct_sound_->load_wave_file( "test.wav" );

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

	/*
	if ( FAILED( direct_3d_->getVertexBuffer()->Lock( 0, 0, reinterpret_cast< void** >( & point_sprite_ ), 0 ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}
	*/
}

//■デストラクタ
CGameMain::~CGameMain()
{
	delete stage_;
	delete camera_;
	delete player_;

	delete mesh_;
	
	delete box_;
	delete player_shadow_box_;

	delete direct_3d_;

	delete direct_sound_buffer_;

	delete direct_sound_;
}

static int fps = 0, last_fps = 0;

/**
 * メインループ処理
 *
 */
void CGameMain::update()
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
	

	// Player
	const float speed = 0.001f;

	if ( GetAsyncKeyState( 'A' ) & 0x8000 ) { player_->velocity().x() -= speed; }
	if ( GetAsyncKeyState( 'D' ) & 0x8000 ) { player_->velocity().x() += speed; }
	if ( GetAsyncKeyState( 'W' ) & 0x8000 ) { player_->velocity().z() += speed; }
	if ( GetAsyncKeyState( 'S' ) & 0x8000 ) { player_->velocity().z() -= speed; }
	if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) { player_->jump(); direct_sound_buffer_->play(); }

	player_->set_floor_height( stage_->map_chip( static_cast< int >( player_->position().x() ), static_cast< int >( player_->position().z() ) ) ); 
	player_->update();

	camera_->position() = player_->position() + vector3( 0.f, 1.5f, 0.f );
	
	const float under_view_max_speed = 0.05f;
	static float under_view_speed = 0.f;

	if ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 )
	{
		under_view_speed += 0.01f;
	}
	else
	{
		under_view_speed -= 0.004f;
	}

	under_view_speed = math::clamp( under_view_speed, -under_view_max_speed, under_view_max_speed );
	
	camera_->set_under_view_rate( camera_->get_under_view_rate() + under_view_speed );

	render();
}

/**
 * 描画
 */
void CGameMain::render()
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
	D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() ), 720.f / 480.f, 0.01f, 400.f );
	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, & projection );

	D3DXMATRIX WorldViewProjection = world * view * projection;
	vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

	FAIL_CHECK( direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xEE, 0xEE, 0xFF ), 1.f, 0 ) );
	FAIL_CHECK( direct_3d_->getDevice()->BeginScene() );

	direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );

	direct_3d_->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	direct_3d_->getDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

//	direct_3d_->getDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	mesh_->render();

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

	D3DXMatrixTranslation( & world, player_->position().x(), player_->get_floor_height(), player_->position().z() );
	direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

	WorldViewProjection = world * view * projection;
	vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

	player_shadow_box_->ready();
	player_shadow_box_->render();


	FAIL_CHECK( direct_3d_->getDevice()->EndScene() );
	FAIL_CHECK( direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL ) );

	// Debug
	std::string debug_text;
	debug_text = std::string( "blue-sky | FPS : " ) + common::serialize( last_fps ) + ", player : " + common::serialize( player_->position().y() );
	CApp::GetInstance()->setTitle( debug_text.c_str() );
}
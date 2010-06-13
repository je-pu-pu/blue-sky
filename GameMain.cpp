/**
 *
 *
 */


#include "GameMain.h"

#include "App.h"

#include "Player.h"
#include "Camera.h"
#include "Stage.h"

#include "Model.h"
#include "Direct3D9Canvas.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9Box.h"

#include "matrix4x4.h"
#include "vector3.h"

#include	"Util.h"
#include	<math.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

using namespace std;

art::Model sample_model;
art::Model field_model;

float g_eye_far_len = 3.f;

const char* model_file_name_list[] =
{
	"./blue-sky-building-13.obj",
	"./blue-sky-building-3.obj",
	"./blue-sky-building-2.obj",
	"./blue-sky-building-4.obj",

	"./blue-sky-box2.obj",
	"./blue-sky-box.obj",

	"./boxes.obj",
	"./blue-sky-box3.obj",
	"./blue-sky-building-3.obj",
	"./blue-sky-building-11.obj",
	"./blue-sky-building-12.obj",

	"./grid-building.obj",

	"./grid.obj",
	"./grid-cube.obj"
};

using blue_sky::Player;
using blue_sky::Camera;
using blue_sky::Stage;

Direct3D9Mesh* mesh_ = 0;
Direct3D9Box* box_ = 0;

Player* player_ = 0;
Camera* camera_ = 0;
Stage* stage_ = 0;

LPD3DXCONSTANTTABLE vs_constant_table;
LPD3DXCONSTANTTABLE ps_constant_table;

//■コンストラクタ
CGameMain::CGameMain()
	: direct_3d_( 0 )
	, canvas_( 0 )
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

	// Canvassssw
	canvas_ = new art::Direct3D9Canvas( direct_3d_ );
	canvas_->createDepthBuffer();

	// Mesh
	mesh_ = new Direct3D9Mesh( direct_3d_ );

	// mesh_->loadX( "blue-sky-building-13.x" );
	// mesh_->loadX( "blue-sky-buildings.x" );
	// mesh_->loadX( "box3.x" );
	// mesh_->loadX( "box2.x" );
	mesh_->loadX( "hoge.x" );

	// Box
	box_ = new Direct3D9Box( direct_3d_, 0.8f, 0.8f, 0.8f );

	// Player
	player_ = new Player();
	player_->position().set( 0.f, 10.f, 0.f );

	// Camera
	camera_ = new Camera();
	camera_->fov() = 90.f;

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

	delete mesh_;
	delete canvas_;
	delete direct_3d_;
}

static art::Vertex eye_pos( 0.f, 2.f, 0.f );

void CGameMain::convert_3d_to_2d( vector3& v )
{
	v *= 0.25f;

	const float cx = static_cast< float >( Width / 2 );
	const float cy = static_cast< float >( Height / 2 );

	const float eye_far_len = g_eye_far_len;

	v = v - eye_pos;

	const float sx = min( max( ( eye_far_len - v.z() ) / eye_far_len, 0.f ), 2.f );
	const float sy = min( max( ( eye_far_len - v.z() ) / eye_far_len, 0.f ), 2.f );

	v.x() *= pow( sx, 4.f );
	v.y() *= pow( sy, 4.f );
	v.z() = ( v.z() + eye_far_len ) * 0.0001f;

	v.x() = v.x() * cy + cx;
	v.y() = v.y() * cy + cy;

	// srand( time( 0 ) );

	// random
	const float r = 10.f;

	// v.x() += ( rand() % RAND_MAX / static_cast< float >( RAND_MAX ) ) * r - ( r / 2.f );
	// v.y() += ( rand() % RAND_MAX / static_cast< float >( RAND_MAX ) ) * r - ( r / 2.f );
}

static int fps = 0, last_fps = 0;
static bool draw_face = true;

/**
 * メインループ処理
 *
 */
void CGameMain::update()
{
	// canvas_->render();
	// return;

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

	// 色
	RGBQUAD white =	{ 255, 255, 255, 20 };
	RGBQUAD red =	{ 0, 0, 255, 127 };
	RGBQUAD blue =	{ 255, 0, 0 };

	RGBQUAD color_building = { 0, 255, 0, 255 };
	RGBQUAD color_building_dark = { 0, 80, 0, 255 };
	RGBQUAD color_building_edge = { 0, 20, 0, 255 };
	RGBQUAD color_river = { 0xFF, 0x60, 0x10, 0x20 };

	const int horizon_y = Height / 2;
	
	canvas_->fillRect( Rect( 0, 0, Width, horizon_y ), RGB( 190, 200, 255 ) );
	canvas_->fillRect( Rect( 0, horizon_y, Width, Height ), RGB( 80, 100, 80 ) );

	static int space;
	static int c;
	static int line_count = 32;
	static int line_len = 200;

	static POINT p[ 3 ];
	static POINT m[ 3 ];

	if(GetAsyncKeyState(VK_SPACE))	space++;
	else							space = 0;

	if ( GetAsyncKeyState( VK_RETURN ) )
	{
		eye_pos = art::Vertex( 0.f, 2.f, 0.f );
	}

	if(space == 1){
		line_count++;
		line_len += 32;
	}

	//キー入力
	if(GetAsyncKeyState(VK_LEFT))	c--;
	if(GetAsyncKeyState(VK_RIGHT))	c++;
	if(GetAsyncKeyState(VK_UP))		line_len++;
	if(GetAsyncKeyState(VK_DOWN))	line_len--;
	if(GetAsyncKeyState('Z'))		line_count++;
	if(GetAsyncKeyState('X'))		line_count--;
	if(line_count < 0)	line_count = 0;
	
	static double sr = 1.f;
	static double sdr = 0.1f;
	static double g = 1.0001f;
	static bool r_random = true;
	static bool p_random = true;
	static int mn = 100;
	
	if(GetAsyncKeyState('A'))		sr -= 0.01f;
	if(GetAsyncKeyState('S'))		sr += 0.01f;
	if(GetAsyncKeyState('D'))		sdr -= 0.01f;
	if(GetAsyncKeyState('F'))		sdr += 0.01f;

	if(GetAsyncKeyState('G'))		g -= 0.01f;
	if(GetAsyncKeyState('H'))		g += 0.01f;
	if(GetAsyncKeyState('R'))		r_random = ! p_random;
	if(GetAsyncKeyState('T'))		p_random = ! p_random;

	if(GetAsyncKeyState('Q'))		mn -= 10;
	if(GetAsyncKeyState('W'))		mn += 10;

	if ( GetAsyncKeyState( '1' ) )	g_solid = ! g_solid;
	if ( GetAsyncKeyState( '2' ) )	g_line = ! g_line;

	if ( GetAsyncKeyState( '3' ) )	g_power -= 0.1f;
	if ( GetAsyncKeyState( '4' ) )	g_power += 0.1f;
	if ( GetAsyncKeyState( '5' ) )	g_power_min -= 0.1f;
	if ( GetAsyncKeyState( '6' ) )	g_power_min += 0.1f;
	if ( GetAsyncKeyState( '7' ) )	g_power_max -= 0.1f;
	if ( GetAsyncKeyState( '8' ) )	g_power_max += 0.1f;
	if ( GetAsyncKeyState( '9' ) )	g_power_plus -= 0.01f;
	if ( GetAsyncKeyState( '0' ) )	g_power_plus += 0.01f;

	if ( GetAsyncKeyState( 'Y' ) )	g_power_rest -= 0.01f;
	if ( GetAsyncKeyState( 'U' ) )	g_power_rest += 0.01f;
	if ( GetAsyncKeyState( 'I' ) )	g_power_plus_reset -= 0.01f;
	if ( GetAsyncKeyState( 'O' ) )	g_power_plus_reset += 0.01f;

	if ( GetAsyncKeyState( 'Z' ) )	g_direction_fix_default -= 0.0001f;
	if ( GetAsyncKeyState( 'X' ) )	g_direction_fix_default += 0.0001f;
	if ( GetAsyncKeyState( 'C' ) )	g_direction_fix_acceleration -= 0.00001f;
	if ( GetAsyncKeyState( 'V' ) )	g_direction_fix_acceleration += 0.00001f;
	if ( GetAsyncKeyState( 'B' ) )	g_direction_random -= 0.001f;
	if ( GetAsyncKeyState( 'N' ) )	g_direction_random += 0.001f;

	if ( GetAsyncKeyState( 'F' ) )	draw_face = ! draw_face;

	if ( GetAsyncKeyState( VK_PRIOR ) ) { g_eye_far_len -= 0.1f; }
	if ( GetAsyncKeyState( VK_NEXT  ) ) { g_eye_far_len += 0.1f; }

	if ( GetAsyncKeyState( VK_TAB ) )
	{
		static int n = 0;

		sample_model.clear();
		sample_model.load_file( model_file_name_list[ n ] );

		n = ( n + 1 ) % ( sizeof( model_file_name_list ) / sizeof( char ) );

		canvas_->clear();

		Sleep( 1000 );
	}

	g_line_count = 0;
	g_circle_count = 0;
	
	// 3D BOX
	matrix4x4 mt;

	if ( GetAsyncKeyState( VK_LEFT  ) ) { eye_pos.x() -= 0.1f; }
	if ( GetAsyncKeyState( VK_RIGHT ) ) { eye_pos.x() += 0.1f; }
	if ( GetAsyncKeyState( VK_UP    ) ) { eye_pos.y() -= 0.1f; }
	if ( GetAsyncKeyState( VK_DOWN  ) ) { eye_pos.y() += 0.1f; }

	if ( GetAsyncKeyState( VK_NUMPAD9 ) ) { eye_pos.z() -= 0.1f; }
	if ( GetAsyncKeyState( VK_NUMPAD3 ) ) { eye_pos.z() += 0.1f; }

	if ( GetAsyncKeyState( VK_NUMPAD4 ) ) { matrix4x4 m; m.rotate_y( +1.f ); mt *= m; }
	if ( GetAsyncKeyState( VK_NUMPAD6 ) ) { matrix4x4 m; m.rotate_y( -1.f ); mt *= m; }
	if ( GetAsyncKeyState( VK_NUMPAD8 ) ) { matrix4x4 m; m.rotate_x( -1.f ); mt *= m; }
	if ( GetAsyncKeyState( VK_NUMPAD2 ) ) { matrix4x4 m; m.rotate_x( +1.f ); mt *= m; }

	const float speed = 0.001f;

	if ( GetAsyncKeyState( 'A' ) & 0x8000 ) { player_->velocity().x() -= speed; }
	if ( GetAsyncKeyState( 'D' ) & 0x8000 ) { player_->velocity().x() += speed; }
	if ( GetAsyncKeyState( 'W' ) & 0x8000 ) { player_->velocity().z() += speed; }
	if ( GetAsyncKeyState( 'S' ) & 0x8000 ) { player_->velocity().z() -= speed; }
	if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8001 ) { player_->jump(); }

	player_->set_floor_height( stage_->map_chip( static_cast< int >( player_->position().x() ), static_cast< int >( player_->position().z() ) ) ); 
	player_->update();

	camera_->position() = player_->position() + vector3( 0.f, 1.5f, 0.f );
	
	if ( player_->jumping() )
	{
		camera_->set_under_view_rate( camera_->get_under_view_rate() + 0.05f );
	}
	else
	{
		camera_->set_under_view_rate( camera_->get_under_view_rate() - 0.05f );
	}

	// rotate
	if ( space > 0 )
	{
		mt.rotate_y( 1.f );
		mt.rotate_z( 0.1f );
	}

	// if ( GetAsyncKeyState( 'L' ) )
	{

	for ( art::Model::VertexList::iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		i->second.vertex() *= mt;
	}

	// 3D 座標を 2D キャンバスターゲット座標にコピー
	for ( art::Model::VertexList::const_iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		canvas_->vertex_list()[ i->first ].target_vertex() = i->second.vertex();
	}

	canvas_->line_list().clear();

	// 3D ラインを 2D キャンバスラインにコピー
	for ( art::Model::LineList::const_iterator i = sample_model.line_list().begin(); i != sample_model.line_list().end(); ++i )
	{
		canvas_->line_list().push_back( art::Canvas::Line( i->start_vertex_id(), i->end_vertex_id(), i->start_color() ) );
	}
	
	// 3D フェイスを 2D キャンバスフェイスにコピー
	canvas_->face_list() = sample_model.face_list();

	// 2D キャンバスターゲット座標を透視投影変換
	for ( art::Canvas::VertexList::iterator i = canvas_->vertex_list().begin(); i != canvas_->vertex_list().end(); ++i )
	{
		convert_3d_to_2d( i->second.target_vertex() );

		i->second.update();
	}

	} // first

	D3DXMATRIXA16 world;
	D3DXMatrixIdentity( & world );
	D3DXMatrixScaling( & world, 10.f, 10.f, 10.f );
//	D3DXMatrixRotationY( & world, timeGetTime() / 100000.f );
    direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

	D3DXMATRIXA16 view;
	D3DXMatrixLookAtLH( & view, reinterpret_cast< D3DXVECTOR3* >( & camera_->position() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->look_at() ), reinterpret_cast< const D3DXVECTOR3* >( & camera_->up() ) );
	direct_3d_->getDevice()->SetTransform( D3DTS_VIEW, & view );

	D3DXMATRIXA16 projection;
	D3DXMatrixPerspectiveFovLH( & projection, math::degree_to_radian( camera_->fov() ), 720.f / 480.f, 1.f, 400.f );
	direct_3d_->getDevice()->SetTransform( D3DTS_PROJECTION, & projection );

	D3DXMATRIX WorldViewProjection = world * view * projection;
	vs_constant_table->SetMatrix( direct_3d_->getDevice(), "WorldViewProjection", & WorldViewProjection );

	std::string debug_text;
	debug_text = std::string( "FPS : " ) + common::serialize( last_fps ) + ", player : " + common::serialize( player_->position().y() );
	CApp::GetInstance()->setTitle( debug_text.c_str() );

	FAIL_CHECK( direct_3d_->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xEE, 0xEE, 0xFF ), 1.f, 0 ) );
	FAIL_CHECK( direct_3d_->getDevice()->BeginScene() );

	direct_3d_->getDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	direct_3d_->getDevice()->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );

	// box_->ready();
	// box_->render();

	mesh_->render();

	FAIL_CHECK( direct_3d_->getDevice()->EndScene() );
	FAIL_CHECK( direct_3d_->getDevice()->Present( NULL, NULL, NULL, NULL ) );

	/*
	for ( int z = 0; z < stage_->depth(); z++ )
	{
		for ( int x = 0; x < stage_->width(); x++ )
		{
			int y = stage_->map_chip( x, z );
			
			if ( y > 0 )
			{
				D3DXMATRIXA16 world;
				D3DXMatrixTranslation( & world, x, y - 1, z );
				direct_3d_->getDevice()->SetTransform( D3DTS_WORLD, & world );

				box_->render();
			}
		}
	}
	*/

	// render();
}

/**
 * 描画
 */
void CGameMain::render()
{
	int n = 0;

	canvas_->clearDepthBuffer();
	canvas_->begin();

	/*
	canvas_->drawCircle( art::Vertex( 110.f, 110.f, 0.1f ), 300.f, art::Color( 255, 0, 0 ), true );
	canvas_->drawCircle( art::Vertex( 120.f, 120.f, 0.5f ), 300.f, art::Color( 0, 255, 0 ), true );
	canvas_->drawCircle( art::Vertex( 130.f, 130.f, 0.9f ), 300.f, art::Color( 0, 0, 255 ), true );
	*/

	if ( draw_face )
	{
		canvas_->sort_face_list_by_z();

		art::Canvas::Brush brush;
		brush.size() = 10.f;
		brush.size_acceleration() = 0.08f;
		canvas_->setBrush( & brush );

		int face_id = 0;

		// 面を描画する
		for ( art::Canvas::FaceList::iterator i = canvas_->face_list().begin(); i != canvas_->face_list().end(); ++i )
		{
			canvas_->setDepthBufferPixelId( face_id );
			canvas_->drawPolygonHumanTouch( *i, i->color() );

			face_id++;
		}
	}

	canvas_->sort_line_list_by_z();

	// 線を描画する
	for ( art::Canvas::LineList::iterator i = canvas_->line_list().begin(); i != canvas_->line_list().end(); ++i )
	{
		art::Canvas::Brush brush;
		brush.size() = 1.f;
		brush.size_acceleration() = 0.05f;

		art::Vertex& from = canvas_->vertex_list()[ i->from() ].vertex();
		art::Vertex& to = canvas_->vertex_list()[ i->to() ].vertex();
		
		canvas_->setBrush( 0 );
		canvas_->setDepthBufferPixelId( i->from() );
		canvas_->drawLineHumanTouch( from, to, i->color() );
	}

	/*
	for ( int n = 0; n < 100; n++ )
	{
		// art::Canvas::Brush brush;
		// brush.size() = 1.f;
		// brush.size_acceleration() = 0.01f;
		// canvas_->setBrush( & brush );
		canvas_->setDepthBufferPixelId( n );

		int x = n / 2 * 30;
		int y = n / 2 * 30;

		if ( n % 2 == 0 )
		{
			canvas_->drawLineHumanTouch( art::Vertex( 10.f, y, 0.f ), art::Vertex( Width - 10.f, y, 0.f ), art::Color( 0, 255, 0 ) );
		}
		else
		{
			// canvas_->drawLineHumanTouch( art::Vertex( x, 10.f, 0.f ), art::Vertex( x, Height - 10.f, 0.f ), art::Color( 0, 0, 255 ) );
		}
	}
	*/

	canvas_->render();

	//デバッグ情報描画
	std::string debug_text;

	debug_text = std::string( "FPS : " ) + common::serialize( last_fps ) + ", circle : " + common::serialize( g_circle_count );
	debug_text += ", line : " + common::serialize( g_line_count ) + " / " + common::serialize( canvas_->line_list().size() );
	debug_text += ", face : " + common::serialize( canvas_->face_list().size() );
	debug_text += ", eye : " + common::serialize( g_eye_far_len );
	debug_text += "\n" + ( boost::format( "power:%.3f,%.3f,%.3f plus:%.3f reset:%.3f plus_reset:%.3f" ) % g_power % g_power_min % g_power_max % g_power_plus % g_power_rest % g_power_plus_reset ).str();
	debug_text += "\n" + ( boost::format( "dir_fix_d:%.5f dir_fix_acc:%.5f dir_rnd:%.4f" ) % g_direction_fix_default % g_direction_fix_acceleration % g_direction_random ).str();

	debug_text += "\n";

	int x = 0;

	for ( Canvas::VertexList::iterator i = canvas_->vertex_list().begin(); i != canvas_->vertex_list().end(); ++i )
	{
		debug_text += ( boost::format( "%-4.3f,%-4.3f,%-4.3f\n" ) % i->second.vertex().x() % i->second.vertex().y() % i->second.vertex().z() ).str();

		if ( x >= 8 )
		{
			debug_text += "...\n";
			break;
		}

		x++;
	}

	debug_text += "\n";

	/*
	for ( int a = 0; a < 100; a++ )
	{
		debug_text += common::serialize( math::clamp( common::random( -4, 4 ), 0, 255 ) ) + ",";
	}
	*/

	canvas_->drawText( art::Vertex( 0.f, 0.f ), debug_text.c_str(), art::Color( 255, 0, 0 ) );

	canvas_->end();

	mesh_->render();
}
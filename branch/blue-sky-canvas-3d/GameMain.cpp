/**
 *
 *
 */


#include	"GameMain.h"
#include	"App.h"

#include	"Model.h"
#include "Direct3D9Canvas.h"
#include "Direct3D9Font.h"

#include	"matrix4x4.h"
#include	"vector3.h"

#include	"Util.h"
#include	<math.h>

#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

using namespace std;

art::Model sample_model;
art::Model field_model;

float g_eye_far_len = 10.f;

const char* model_file_name_list[] =
{
	"./boxes.obj",
	"./blue-sky-box3.obj",
	"./blue-sky-building-3.obj",
	"./blue-sky-building-11.obj",
	"./blue-sky-building-12.obj",

	"./grid-building.obj",

	"./grid.obj",
	"./grid-cube.obj"
};

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
	direct_3d_ = new Direct3D9( app->GetWindowHandle(), Width, Height, true );
	// direct_3d_->getFont()->load( "uzura_font", "media/font/uzura.ttf" );

	// Canvas
	canvas_ = new art::Direct3D9Canvas( direct_3d_ );
	canvas_->createDepthBuffer();
}

//■デストラクタ
CGameMain::~CGameMain()
{
	delete canvas_;
	delete direct_3d_;
}

static art::Vertex eye_pos( 0.f, 0.f, 0.f );

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

//■■■　メインループ　■■■
void CGameMain::Loop()
{
	static int fps = 0, last_fps = 0;
	static int sec = 0;
	
	if ( timeGetTime() / 1000 != sec )
	{
		sec = timeGetTime() / 1000;

		last_fps = fps;
		fps = 0;
	}

	fps++;
	
	// canvas_->render();
	// return;

	MainLoop.WaitTime = 0;

	//秒間50フレームを保持
	// if(! MainLoop.Loop())	return;

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

	static bool draw_face = true;
	
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

		canvas_->vertex_list().clear();

		sample_model.clear();
		sample_model.load_file( model_file_name_list[ n ] );

		n = ( n + 1 ) % ( sizeof( model_file_name_list ) / sizeof( char ) );

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

	canvas_->drawText( art::Vertex( 0.f, 0.f ), debug_text.c_str(), art::Color( 255, 0, 0, 127 ) );

	canvas_->end();
}

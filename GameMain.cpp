/**
 *
 *
 */


#include	"GameMain.h"
#include	"App.h"

#include	"Model.h"
#include	"Direct3D9Canvas.h"

#include	"matrix4x4.h"
#include	"vector3.h"

#include	"Util.h"
#include	<math.h>

using namespace std;

art::Model sample_model;

//■コンストラクタ
CGameMain::CGameMain()
	: canvas_( 0 )
	, Width( 0 )
	, Height( 0 )
{
	CApp *app = CApp::GetInstance();
	Width = app->GetWidth();
	Height = app->GetHeight();

	//ランダマイズ
	srand( timeGetTime() );

	// Direct3D 
	canvas_ = new art::Direct3D9Canvas( app->GetWindowHandle() );

	const float half = 0.5f;

	art::Color color_building( 0, 255, 0, 255 );
	art::Color color_building_dark( 0, 140, 0, 255 );
	art::Color color_building_edge( 0, 80, 0, 255 );

	// index ( id = index + 1 )
	//  
	//   4----5
	//  /|   /|
	// 0----1 |
	// | |  | |
	// | |  | |
	// | 6--|-7
	// |/   |/
	// 2----3

	sample_model.load_file( "./blue-sky-building-2.obj" );
	// sample_model.load_file( "./blue-sky-box.obj" );
}

//■デストラクタ
CGameMain::~CGameMain()
{
	delete canvas_;
}

static float eye_z = 2.f;

void CGameMain::convert_3d_to_2d( vector3& v )
{
	const int cx = Width / 2;
	const int cy = Height / 2;

	const int w = 100;
	
	const float eye_far_len = 10.f;

	v.x() *= ( ( eye_far_len - ( eye_z - v.z() ) ) / eye_far_len );
	v.y() *= ( ( eye_far_len - ( eye_z - v.z() ) ) / eye_far_len );

	v.x() = v.x() * w + cx;
	v.y() = v.y() * w + cy;

	// random
	const int r = 1;

	v.x() += rand() % r - r / 2;
	v.y() += rand() % r - r / 2;
}

//■■■　メインループ　■■■
void CGameMain::Loop()
{
	static bool g_solid = false;
	static bool g_line = false;

	static double g_power = 1.0;
	static double g_power_min = 1.0;
	static double g_power_max = 5.0;
	static double g_power_plus = 0.1;
	static double g_power_rest = 0.8;
	static double g_power_plus_reset = -1.0;

	static double g_direction_fix_default = 0.0001;
	static double g_direction_fix_acceleration = 0.01;
	static double g_direction_random = 0.01;

	static int g_circle_count = 0;

	MainLoop.WaitTime = 0;

	//秒間50フレームを保持
	if(! MainLoop.Loop())	return;

	// 色
	RGBQUAD white =	{ 255, 255, 255, 20 };
	RGBQUAD red =	{ 0, 0, 255, 127 };
	RGBQUAD blue =	{ 255, 0, 0 };

	RGBQUAD color_building = { 0, 255, 0, 255 };
	RGBQUAD color_building_dark = { 0, 80, 0, 255 };
	RGBQUAD color_building_edge = { 0, 20, 0, 255 };
	RGBQUAD color_river = { 0xFF, 0x60, 0x10, 0x20 };

	const int horizon_y = Height / 2;
	
	canvas_->FillRect( Rect( 0, 0, Width, horizon_y ), RGB( 190, 200, 255 ) );
	canvas_->FillRect( Rect( 0, horizon_y, Width, Height ), RGB( 80, 100, 80 ) );

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
		eye_z = 2.f;
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

	if ( GetAsyncKeyState( '3' ) )	g_power -= 0.1;
	if ( GetAsyncKeyState( '4' ) )	g_power += 0.1;
	if ( GetAsyncKeyState( '5' ) )	g_power_min -= 0.1;
	if ( GetAsyncKeyState( '6' ) )	g_power_min += 0.1;
	if ( GetAsyncKeyState( '7' ) )	g_power_max -= 0.1;
	if ( GetAsyncKeyState( '8' ) )	g_power_max += 0.1;
	if ( GetAsyncKeyState( '9' ) )	g_power_plus -= 0.01;
	if ( GetAsyncKeyState( '0' ) )	g_power_plus += 0.01;

	if ( GetAsyncKeyState( 'Y' ) )	g_power_rest -= 0.01;
	if ( GetAsyncKeyState( 'U' ) )	g_power_rest += 0.01;
	if ( GetAsyncKeyState( 'I' ) )	g_power_plus_reset -= 0.01;
	if ( GetAsyncKeyState( 'O' ) )	g_power_plus_reset += 0.01;

	if ( GetAsyncKeyState( 'Z' ) )	g_direction_fix_default -= 0.0001;
	if ( GetAsyncKeyState( 'X' ) )	g_direction_fix_default += 0.0001;
	if ( GetAsyncKeyState( 'C' ) )	g_direction_fix_acceleration -= 0.001;
	if ( GetAsyncKeyState( 'V' ) )	g_direction_fix_acceleration += 0.001;
	if ( GetAsyncKeyState( 'B' ) )	g_direction_random -= 0.001;
	if ( GetAsyncKeyState( 'N' ) )	g_direction_random += 0.001;

	g_circle_count = 0;
	
	// 3D BOX
	matrix4x4 mt;

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

	for ( art::Model::VertexList::iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		i->second.vertex() *= mt;
	}

	canvas_->line_list().clear();

	// 3D 座標を 2D キャンバスターゲット座標にコピー
	for ( art::Model::VertexList::const_iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		canvas_->vertex_list()[ i->first ].target_vertex() = i->second.vertex();
	}

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

	eye_z -= 0.001f;

	int n = 0;

	// 面を描画する
	for ( art::Canvas::FaceList::iterator i = canvas_->face_list().begin(); i != canvas_->face_list().end(); ++i )
	{
		srand( n + getMainLoop().GetNowTime() / 12000 );
		
		POINT p[] = {
			{ canvas_->vertex_list()[ i->index_list()[ 0 ] ].vertex().x(), canvas_->vertex_list()[ i->index_list()[ 0 ] ].vertex().y() },
			{ canvas_->vertex_list()[ i->index_list()[ 1 ] ].vertex().x(), canvas_->vertex_list()[ i->index_list()[ 1 ] ].vertex().y() },
			{ canvas_->vertex_list()[ i->index_list()[ 2 ] ].vertex().x(), canvas_->vertex_list()[ i->index_list()[ 2 ] ].vertex().y() },
			{ canvas_->vertex_list()[ i->index_list()[ 3 ] ].vertex().x(), canvas_->vertex_list()[ i->index_list()[ 3 ] ].vertex().y() },
		};
		
		canvas_->DrawPolygonHumanTouch( p, i->color() );
	}

	// 線を描画する
	for ( art::Canvas::LineList::iterator i = canvas_->line_list().begin(); i != canvas_->line_list().end(); ++i )
	{
		n++;
		
		// srand( n + getMainLoop().GetNowTime() / 1000 );

		art::Vertex& from = canvas_->vertex_list()[ i->from() ].vertex();
		art::Vertex& to = canvas_->vertex_list()[ i->to() ].vertex();
		
		canvas_->DrawLineHumanTouch( from.x(), from.y(), to.x(), to.y(), RGBQUAD( i->color() ) );
	}

	/*
	for ( art::Canvas::LineList::iterator i = canvas_->line_list().begin(); i != canvas_->line_list().end(); ++i )
	{
		n++;
		
		srand( n + getMainLoop().GetNowTime() / 100000 );

		art::Vertex& from = canvas_->vertex_list()[ i->from() ].target_vertex();
		art::Vertex& to = canvas_->vertex_list()[ i->to() ].target_vertex();

		lpDst->DrawLineHumanTouch( from.x(), from.y(), to.x(), to.y(), art::Color( 0, 0, 0, 100 ) );
	}
	*/

	
	canvas_->render();

	if ( true )
	{
		sprintf(DebugStr2, "dir_fix_d:%.5f dir_fix_acc:%.5f dir_rnd:%.4f", g_direction_fix_default, g_direction_fix_acceleration, g_direction_random );
	}

	//デバッグ情報描画
	char fps[255];
	wsprintf(fps, "FPS : %d, circle : %d", MainLoop.GetFPS(), g_circle_count );
}
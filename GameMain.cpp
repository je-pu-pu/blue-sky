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

#include <common/serialize.h>
#include <boost/format.hpp>

using namespace std;

art::Model sample_model;

//���R���X�g���N�^
CGameMain::CGameMain()
	: canvas_( 0 )
	, Width( 0 )
	, Height( 0 )
{
	CApp *app = CApp::GetInstance();
	Width = app->GetWidth();
	Height = app->GetHeight();

	//�����_�}�C�Y
	srand( timeGetTime() );

	// Direct3D 
	canvas_ = new art::Direct3D9Canvas( app->GetWindowHandle() );

	// sample_model.load_file( "./blue-sky-building-3.obj" );
	// sample_model.load_file( "./blue-sky-building-2.obj" );
	sample_model.load_file( "./blue-sky-building-4.obj" );
	// sample_model.load_file( "./blue-sky-box.obj" );
	// sample_model.load_file( "./blue-sky-box2.obj" );
	// sample_model.load_file( "./blue-sky-box3.obj" );

	// sample_model.load_file( "./grid-building.obj" );

	// sample_model.load_file( "./grid.obj" );
	// sample_model.load_file( "./grid-cube.obj" );
}

//���f�X�g���N�^
CGameMain::~CGameMain()
{
	delete canvas_;
}

static art::Vertex eye_pos( 0.f, 2.f, 0.f );

void CGameMain::convert_3d_to_2d( vector3& v )
{
	v *= 0.25f;

	const float cx = static_cast< float >( Width / 2 );
	const float cy = static_cast< float >( Height / 2 );

	const float eye_far_len = 50.f;

	v = v - eye_pos;

	const float sx = min( max( ( eye_far_len - v.z() ) / eye_far_len, 0.f ), 2.f );
	const float sy = min( max( ( eye_far_len - v.z() ) / eye_far_len, 0.f ), 2.f );

	v.x() *= pow( sx, 5.f );
	v.y() *= pow( sy, 5.f );
	v.z() = -v.z();

	v.x() = v.x() * cy + cx;
	v.y() = v.y() * cy + cy;

	// srand( time( 0 ) );

	// random
	const float r = 10.f;

	// v.x() += ( rand() % RAND_MAX / static_cast< float >( RAND_MAX ) ) * r - ( r / 2.f );
	// v.y() += ( rand() % RAND_MAX / static_cast< float >( RAND_MAX ) ) * r - ( r / 2.f );
}

//�������@���C�����[�v�@������
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

	//�b��50�t���[����ێ�
	// if(! MainLoop.Loop())	return;

	// �F
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

	//�L�[����
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
	if ( GetAsyncKeyState( 'C' ) )	g_direction_fix_acceleration -= 0.001f;
	if ( GetAsyncKeyState( 'V' ) )	g_direction_fix_acceleration += 0.001f;
	if ( GetAsyncKeyState( 'B' ) )	g_direction_random -= 0.001f;
	if ( GetAsyncKeyState( 'N' ) )	g_direction_random += 0.001f;

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

	if ( GetAsyncKeyState( 'L' ) )
	{

	for ( art::Model::VertexList::iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		i->second.vertex() *= mt;
	}

	// 3D ���W�� 2D �L�����o�X�^�[�Q�b�g���W�ɃR�s�[
	for ( art::Model::VertexList::const_iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		canvas_->vertex_list()[ i->first ].target_vertex() = i->second.vertex();
	}

	canvas_->line_list().clear();

	// 3D ���C���� 2D �L�����o�X���C���ɃR�s�[
	for ( art::Model::LineList::const_iterator i = sample_model.line_list().begin(); i != sample_model.line_list().end(); ++i )
	{
		canvas_->line_list().push_back( art::Canvas::Line( i->start_vertex_id(), i->end_vertex_id(), i->start_color() ) );
	}
	
	// 3D �t�F�C�X�� 2D �L�����o�X�t�F�C�X�ɃR�s�[
	canvas_->face_list() = sample_model.face_list();

	// 2D �L�����o�X�^�[�Q�b�g���W�𓧎����e�ϊ�
	for ( art::Canvas::VertexList::iterator i = canvas_->vertex_list().begin(); i != canvas_->vertex_list().end(); ++i )
	{
		convert_3d_to_2d( i->second.target_vertex() );

		i->second.update();
	}

	} // first

	int n = 0;

	canvas_->begin();

	// �ʂ�`�悷��
	for ( art::Canvas::FaceList::iterator i = canvas_->face_list().begin(); i != canvas_->face_list().end(); ++i )
	{
		canvas_->drawPolygonHumanTouch( *i, i->color() );
	}

	art::Direct3D9Canvas::BeginLine();

	// ����`�悷��
	for ( art::Canvas::LineList::iterator i = canvas_->line_list().begin(); i != canvas_->line_list().end(); ++i )
	{
		art::Canvas::Brush brush;
		brush.size() = 1.f;
		brush.size_acceleration() = 0.05f;

		art::Vertex& from = canvas_->vertex_list()[ i->from() ].vertex();
		art::Vertex& to = canvas_->vertex_list()[ i->to() ].vertex();
		
		canvas_->setBrush( & brush );
		canvas_->drawLineHumanTouch( from, to, i->color() );
	}

	if ( ! g_solid )
	{
		draw_2d_buildings();
	}

	art::Direct3D9Canvas::EndLine();
	

	//�f�o�b�O���`��
	std::string debug_text;

	debug_text = std::string( "FPS : " ) + common::serialize( last_fps ) + ", circle : " + common::serialize( g_circle_count ) + ", line : " + common::serialize( g_line_count ) + " / " + common::serialize( canvas_->line_list().size() );
	debug_text += "\n" + ( boost::format( "power:%.3f,%.3f,%.3f plus:%.3f reset:%.3f plus_reset:%.3f" ) % g_power % g_power_min % g_power_max % g_power_plus % g_power_rest % g_power_plus_reset ).str();
	debug_text += "\n" + ( boost::format( "dir_fix_d:%.5f dir_fix_acc:%.5f dir_rnd:%.4f" ) % g_direction_fix_default % g_direction_fix_acceleration % g_direction_random ).str();

	canvas_->drawText( art::Vertex( 0.f, 0.f ), debug_text.c_str(), art::Color( 255, 0, 0 ) );

	canvas_->end();
	
}

void CGameMain::draw_2d_buildings() const
{
	// �}�E�X
	POINT mp;
	GetCursorPos( & mp );
	ScreenToClient( CApp::GetInstance()->GetWindowHandle(), & mp );

	const int line_count = 32;
	const int line_len = 200;
	
	const int horizon_y = Height / 2;
	
	canvas_->fillRect( Rect( 0, 0, Width, horizon_y ), RGB( 190, 200, 255 ) );
	canvas_->fillRect( Rect( 0, horizon_y, Width, Height ), RGB( 80, 100, 80 ) );

	static float sr = 1.f;
	static float sdr = 0.1f;
	static float g = 1.0001f;
	static bool r_random = true;
	static bool p_random = true;
	static int mn = 100;

	// �F
	const art::Color white( 255, 255, 255, 20 );
	const art::Color red( 0, 0, 255, 127 );
	const art::Color blue( 255, 0, 0 );

	const art::Color color_building( 0, 255, 0, 255 );
	const art::Color color_building_dark( 0, 80, 0, 255 );
	const art::Color color_building_edge( 0, 20, 0, 255 );
	const art::Color color_river( 0xFF, 0x60, 0x10, 0x20 );

	static float cc = 0.f;

	cc += 0.01f;

	float dx = cos( cc );
	float dy = sin( cc );

	// ��
	art::Vertex v( Width / 2.f, Height / 2.f );

	float r = sr;
	float dr = sdr;

	for ( int n = 0; n < mn; n++ )
	{
		v.x() += dx * r * 0.1f;
		v.y() += dy * r * 0.1f;

		r += dr;
		dr *= g;

		if ( r_random )
		{
			r += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
		}

		if ( p_random )
		{
			v.x() += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
			v.y() += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
		}

		if ( r > 20.f * ( n / 100.f ) ) r = 20.f * ( n / 100.f );
		if ( r < 0.1f ) r = 0.1f;

		canvas_->drawCircle( v, r, color_river, false );
	}

	// �C
	for ( int n = 0; n < 10; n++ )
	{
		art::Vertex v1( 0, horizon_y + n );
		art::Vertex v2( Width, horizon_y + n * 4 );

		canvas_->drawLineHumanTouch( v1, v2, color_river );
	}

	// house
	for ( int y = 0; y < 4; y++ )
	{
		for ( int x = 0; x < 6; x++ )
		{
			draw_house( art::Vertex( mp.x + x * 40 + ( x * y * 12 ), mp.y + y * 30 + x * 2 ) );
		}
	}

	// �r��
	int building_left = 20;
	int building_top = Height / 2 + 80;
	int building_bottom = Height;

	for ( int n = 0; n < 100; n++ )
	{
		if ( n < 60 )
		{
			canvas_->drawLineHumanTouch( building_left + n * 2, building_top, building_left + n * 2, Height, color_building );
		}
		else
		{
			canvas_->drawLineHumanTouch( building_left + n * 2, building_top - ( n - 60 ), building_left + n * 2, building_bottom, color_building_dark );
		}
	}

	for ( int n = 0; n < 60; n++ )
	{
		canvas_->drawLineHumanTouch( building_left + n * 2, building_top, building_left + n * 2 + 80, building_top - 40, color_building );
	}

	canvas_->drawLineHumanTouch( building_left, building_top, building_left, building_bottom, color_building_edge );
	canvas_->drawLineHumanTouch( building_left + 120, building_top, building_left + 120, building_bottom, color_building_edge );
	canvas_->drawLineHumanTouch( building_left, building_top, building_left + 120, building_top, color_building_edge );

	// ���C���`��
	for ( int i = 0; i < line_count; i++ )
	{
		art::Vertex v1( Width / 2.f, Height / 2.f );
		art::Vertex v2( v1.x() + cos( cc ) * line_len, v1.y() + sin( cc ) * line_len );
		
		canvas_->drawLineHumanTouch( v1, v2, white );

		cc += static_cast< float >( M_PI * 2.f ) / line_count;
	}
}

void CGameMain::draw_house( const art::Vertex& mp ) const
{
	// �w�i�`��
	RGBQUAD orange = { 0x11, 0xAA, 0xFF, 0xFF };
	RGBQUAD dark_orange = { 0x00, 0x66, 0x99, 0xFF };
	RGBQUAD dark_ground = { 50, 80, 50, 0xFF };

	// �e
	for ( int n = 0; n < 10; n++ )
	{
		canvas_->drawLineHumanTouch( mp.x(), mp.y() + 10 + n * 1, mp.x() + 40 + n * 2, mp.y() + 10 + n * 1, dark_ground );
	}

	// ����
	for ( int n = 0; n < 20; n++ )
	{
		if ( n < 15 )
		{
			canvas_->drawLineHumanTouch( mp.x() + n * 1, mp.y(), mp.x() + n * 1, mp.y() + 20, orange );
		}
		else
		{
			canvas_->drawLineHumanTouch( mp.x() + n * 1, mp.y(), mp.x() + n * 1, mp.y() + 20, dark_orange );
		}
	}
}
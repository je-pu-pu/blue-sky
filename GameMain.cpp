//��������������������������������������������������������������������������������
//GameMain.cpp
//�ŏI�X�V��	2001/11/23
//��������������������������������������������������������������������������������

//�A���`�G�C���A�X�����C������������

#include	"GameMain.h"
#include	"App.h"

#include	"Model.h"
#include	"Canvas.h"

#include	"matrix4x4.h"
#include	"vector3.h"

#include	"DirectDraw.h"
#include	"DirectDrawSurface.h"
#include	"Util.h"
#include	<math.h>

using namespace std;

art::Model sample_model;
art::Canvas canvas;

//���R���X�g���N�^
CGameMain::CGameMain()
{
	MaxAngle = 1024;

	lpDirectDraw = NULL;
	lpPrimary = NULL;
	lpBack = NULL;
	lpSrc = NULL;
	lpDst = NULL;
	
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

	/*
	sample_model.vertex_list().push_back( art::LiveVertex( -half, +half, +half ) ); // 0
	sample_model.vertex_list().push_back( art::LiveVertex( +half, +half, +half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( -half, -half, +half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( +half, -half, +half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( -half, +half, -half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( +half, +half, -half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( -half, -half, -half ) );
	sample_model.vertex_list().push_back( art::LiveVertex( +half, -half, -half ) ); // 7
	
	sample_model.line_list().push_back( art::Line( 0, 1, color_building ) );
	sample_model.line_list().push_back( art::Line( 1, 3, color_building ) );
	sample_model.line_list().push_back( art::Line( 3, 2, color_building ) );
	sample_model.line_list().push_back( art::Line( 2, 0, color_building ) );

	sample_model.line_list().push_back( art::Line( 4, 5, color_building_dark ) );
	sample_model.line_list().push_back( art::Line( 5, 7, color_building_dark ) );
	sample_model.line_list().push_back( art::Line( 7, 6, color_building_dark ) );
	sample_model.line_list().push_back( art::Line( 6, 4, color_building_dark ) );

	sample_model.line_list().push_back( art::Line( 0, 4, color_building_edge ) );
	sample_model.line_list().push_back( art::Line( 1, 5, color_building_edge ) );
	sample_model.line_list().push_back( art::Line( 2, 6, color_building_edge ) );
	sample_model.line_list().push_back( art::Line( 3, 7, color_building_edge ) );

	// temp
	for ( art::Model::LineList::iterator i = sample_model.line_list().begin(); i != sample_model.line_list().end(); ++i )
	{
		i->start_vertex_id()++;
		i->end_vertex_id()++;
	}

	art::Face f;
	f.index_list().push_back( 1 );
	f.index_list().push_back( 2 );
	f.index_list().push_back( 4 );
	f.index_list().push_back( 3 );
	f.color() = art::Color( 255, 127, 127, 255 );

	sample_model.face_list().push_back( f );
	
	f.index_list().clear();
	*/
}

//���f�X�g���N�^
CGameMain::~CGameMain()
{
	UnInit();
}

//��������
bool CGameMain::Init()
{
	CApp *app = CApp::GetInstance();
	hWnd = app->GetWindowHandle();
	Width = app->GetWidth();
	Height = app->GetHeight();

	//�����_�}�C�Y
	srand(timeGetTime());
	//DirectDraw������
	lpDirectDraw = new CDirectDraw[1];
	if(! lpDirectDraw->Init(hWnd, Width, Height, 32)){
		return false;
	}
	lpPrimary = lpDirectDraw->GetPrimary();
	lpBack = lpDirectDraw->GetBackBuffer();

	lpPrimary->Clear(0);
	lpBack->Clear(0);

	//Surface������
	lpDst = new CDirectDrawSurface[1];
	if(! lpDst->CreateOnSystemMemory(lpDirectDraw, Width, Height)){
		return false;
	}
	//�T�C���e�[�u���쐬
	CreateSinTable();

	return true;
}

//���T�C�e�[�u���쐬
void CGameMain::CreateSinTable()
{
	if(SinTable == NULL){
		SinTable = new double[MaxAngle];
		for(int i = 0; i < MaxAngle; i++){
			SinTable[i] = sin(i * 3.1415926563 / (MaxAngle / 2));
		}
	}
}

//���I������
void CGameMain::UnInit()
{
	DebugPrint("CGameMain::UnInit()\n");

	//DirectDraw�I������
	Free(lpSrc);
	Free(lpDst);
	if(lpDirectDraw != NULL){
		int i = lpDirectDraw->UnInit();
		wsprintf(DebugStr, "�J���R�� : %d\n", i);
		DebugPrint();
	}
	Free(lpDirectDraw);
	Free(SinTable);
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

//�������@���C�����[�v�@������
void CGameMain::Loop()
{
	MainLoop.WaitTime = 0;

	//�b��50�t���[����ێ�
	if(! MainLoop.Loop())	return;

	// �F
	RGBQUAD white =	{ 255, 255, 255, 20 };
	RGBQUAD red =	{ 0, 0, 255, 127 };
	RGBQUAD blue =	{ 255, 0, 0 };

	RGBQUAD color_building = { 0, 255, 0, 255 };
	RGBQUAD color_building_dark = { 0, 80, 0, 255 };
	RGBQUAD color_building_edge = { 0, 20, 0, 255 };
	RGBQUAD color_river = { 0xFF, 0x60, 0x10, 0x20 };

	lpDirectDraw->LostCheck();
	lpDst->LostCheck();

	//�N���A
	lpDst->Clear( 0 );

	const int horizon_y = Height / 2;

	lpDst->FillRect( Rect( 0, 0, Width, horizon_y ), RGB( 190, 200, 255 ) );
	lpDst->FillRect( Rect( 0, horizon_y, Width, Height ), RGB( 80, 100, 80 ) );

	//���b�N
	lpDst->Lock();

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

	POINT mp;
	GetCursorPos( & mp );

	double x = Width / 2;
	double y = Height / 2;

	int cc = ( c + MaxAngle ) % MaxAngle;
	double dx = Cos( cc );
	double dy = Sin( cc );

	// ��
	double r = sr;
	double dr = sdr;

	/*
	for ( int n = 0; n < mn; n++ )
	{
		x += dx * r * 0.1f;
		y += dy * r * 0.1f;

		r += dr;
		dr *= g;

		if ( r_random )
		{
			r += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
		}

		if ( p_random )
		{
			x += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
			y += ( ( rand() % 100 / 100.f ) - 0.5f ) * 1.f;
		}

		if ( r > 20.f * ( n / 100.f ) ) r = 20.f * ( n / 100.f );
		if ( r < 0.1f ) r = 0.1f;

		lpDst->DrawCircle( x, y, r, color_river, false );
	}

	// �C
	for ( int n = 0; n < 10; n++ )
	{
		lpDst->DrawLineHumanTouch( 0, horizon_y + n, Width, horizon_y + n * 4, color_river );
	}

	// house
	for ( int y = 0; y < 4; y++ )
	{
		for ( int x = 0; x < 6; x++ )
		{
			POINT p = mp;

			p.x += x * 40 + ( x * y * 12 );
			p.y += y * 30 + x * 2;

			draw_house( p );
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
			lpDst->DrawLineHumanTouch( building_left + n * 2, building_top, building_left + n * 2, Height, color_building );
		}
		else
		{
			lpDst->DrawLineHumanTouch( building_left + n * 2, building_top - ( n - 60 ), building_left + n * 2, building_bottom, color_building_dark );
		}
	}

	for ( int n = 0; n < 60; n++ )
	{
		lpDst->DrawLineHumanTouch( building_left + n * 2, building_top, building_left + n * 2 + 80, building_top - 40, color_building );
	}

	lpDst->DrawLineHumanTouch( building_left, building_top, building_left, building_bottom, color_building_edge );
	lpDst->DrawLineHumanTouch( building_left + 120, building_top, building_left + 120, building_bottom, color_building_edge );
	lpDst->DrawLineHumanTouch( building_left, building_top, building_left + 120, building_top, color_building_edge );

	// ���C���`��
	for(int i = 0; i < line_count; i++){
		double x1 = Width / 2;
		double y1 = Height / 2;
		double x2 = Cos(cc) * line_len + x1;
		double y2 = Sin(cc) * line_len + y1;
		
		lpDst->DrawLineHumanTouch(x1, y1, x2, y2, white);
		cc += MaxAngle / line_count;
	}
	
	*/

	// �~�`��
	if ( GetAsyncKeyState( 'L' ) )
	{
	lpDst->DrawRing(mp.x, mp.y, line_len / 2, line_len / 4, red, true);

	for ( double x = 10.0; x < Width; x *= 1.1 )
	{
		lpDst->DrawCircle( int( x ), Height - 10, 10, red, true );
	}
	for ( double x = 10.0; x < Width; x *= 1.1 )
	{
		lpDst->DrawCircle( x, Height - 30.0, 10.0, red, true );
	}

	for ( int n = 0; n < 3; n++ )
	{
		p[ n ].x += m[ n ].x;
		p[ n ].y += m[ n ].y;

		if ( p[ n ].x < 0 )
		{
			p[ n ].x = 0;
			m[ n ].x = -m[ n ].x;
		}
		if ( p[ n ].y < 0 )
		{
			p[ n ].y = 0;
			m[ n ].y = -m[ n ].y;
		}
		if ( p[ n ].x >= Width )
		{
			p[ n ].x = Width - 1;
			m[ n ].x = -m[ n ].x;
		}
		if ( p[ n ].y >= Height )
		{
			p[ n ].y = Height - 1;
			m[ n ].y = -m[ n ].y;
		}
	}

	static bool first = true;

	if ( first || space == 1 )
	{
		for ( int n = 0; n < 3; n++ )
		{
			p[ n ].x = rand() % Width;
			p[ n ].y = rand() % Height;
			m[ n ].x = rand() % 40 - 20;
			m[ n ].y = rand() % 40 - 20;
		}

		first = false;
	}
	
	// �O�p
	RGBQUAD c2 = { 0, 200, 0, 255 };

	lpDst->DrawPolygonHumanTouch( p, c2 );

	for ( int n = 0; n < 3; n++ )
	{
		double x1 = p[ n ].x;
		double y1 = p[ n ].y;
		double x2 = p[ ( n + 1 ) % 3 ].x;
		double y2 = p[ ( n + 1 ) % 3 ].y;
		
		RGBQUAD cc = { 0, 255, 0, 80 };

		lpDst->DrawLineHumanTouch( x1, y1, x2, y2, cc );
	}
	
	}
	
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

	canvas.line_list().clear();

	// 3D ���W�� 2D �L�����o�X�^�[�Q�b�g���W�ɃR�s�[
	for ( art::Model::VertexList::const_iterator i = sample_model.vertex_list().begin(); i != sample_model.vertex_list().end(); ++i )
	{
		canvas.vertex_list()[ i->first ].target_vertex() = i->second.vertex();
	}

	// 3D ���C���� 2D �L�����o�X���C���ɃR�s�[
	for ( art::Model::LineList::const_iterator i = sample_model.line_list().begin(); i != sample_model.line_list().end(); ++i )
	{
		canvas.line_list().push_back( art::Canvas::Line( i->start_vertex_id(), i->end_vertex_id(), i->start_color() ) );
	}
	
	// 3D �t�F�C�X�� 2D �L�����o�X�t�F�C�X�ɃR�s�[
	canvas.face_list() = sample_model.face_list();

	// 2D �L�����o�X�^�[�Q�b�g���W�𓧎����e�ϊ�
	for ( art::Canvas::VertexList::iterator i = canvas.vertex_list().begin(); i != canvas.vertex_list().end(); ++i )
	{
		convert_3d_to_2d( i->second.target_vertex() );

		i->second.update();
	}

	eye_z -= 0.001f;

	int n = 0;

	// �ʂ�`�悷��
	for ( art::Canvas::FaceList::iterator i = canvas.face_list().begin(); i != canvas.face_list().end(); ++i )
	{
		srand( n + getMainLoop().GetNowTime() / 12000 );
		
		POINT p[] = {
			{ canvas.vertex_list()[ i->index_list()[ 0 ] ].vertex().x(), canvas.vertex_list()[ i->index_list()[ 0 ] ].vertex().y() },
			{ canvas.vertex_list()[ i->index_list()[ 1 ] ].vertex().x(), canvas.vertex_list()[ i->index_list()[ 1 ] ].vertex().y() },
			{ canvas.vertex_list()[ i->index_list()[ 2 ] ].vertex().x(), canvas.vertex_list()[ i->index_list()[ 2 ] ].vertex().y() },
			{ canvas.vertex_list()[ i->index_list()[ 3 ] ].vertex().x(), canvas.vertex_list()[ i->index_list()[ 3 ] ].vertex().y() },
		};

		lpDst->DrawSquareHumanTouch( p, i->color() );
	}

	// ����`�悷��
	for ( art::Canvas::LineList::iterator i = canvas.line_list().begin(); i != canvas.line_list().end(); ++i )
	{
		n++;
		
		// srand( n + getMainLoop().GetNowTime() / 1000 );

		art::Vertex& from = canvas.vertex_list()[ i->from() ].vertex();
		art::Vertex& to = canvas.vertex_list()[ i->to() ].vertex();

		lpDst->DrawLineHumanTouch( from.x(), from.y(), to.x(), to.y(), RGBQUAD( i->color() ) );
	}

	/*
	for ( art::Canvas::LineList::iterator i = canvas.line_list().begin(); i != canvas.line_list().end(); ++i )
	{
		n++;
		
		srand( n + getMainLoop().GetNowTime() / 100000 );

		art::Vertex& from = canvas.vertex_list()[ i->from() ].target_vertex();
		art::Vertex& to = canvas.vertex_list()[ i->to() ].target_vertex();

		lpDst->DrawLineHumanTouch( from.x(), from.y(), to.x(), to.y(), art::Color( 0, 0, 0, 100 ) );
	}
	*/

	// lpDst->Draw

	// �ڂ���
	/*
	std::vector<POINT> pv( 4 );
	// pv[ 0 ].x =  0; pv[ 0 ].y = -1;
	pv[ 1 ].x = -1; pv[ 1 ].y =  0;
	pv[ 2 ].x =  1; pv[ 2 ].y =  0;
	pv[ 0 ].x = -2; pv[ 0 ].y =  0;
	pv[ 3 ].x =  2; pv[ 3 ].y =  0;
	// pv[ 3 ].x =  0; pv[ 3 ].y =  1;

	lpDst->Blur( Rect( 0, 0, Width / 2, Height / 2 ), pv );
	*/

	//�A�����b�N
	lpDst->UnLock();
	lpBack->Blt(0, 0, Width, Height, lpDst, 0, 0, false);

	// lpBack->Blt(Rect(0, 0, 200, 200), lpBack, Rect(200, 200, 250, 250), false);

	if ( true )
	{
		/*
		g_power -= 0.1;
	if ( GetAsyncKeyState( '4' ) )	g_power += 0.1;
	if ( GetAsyncKeyState( '5' ) )	g_power_min -= 0.01;
	if ( GetAsyncKeyState( '6' ) )	g_power_min += 0.01;
	if ( GetAsyncKeyState( '7' ) )	g_power_max -= 0.01;
	if ( GetAsyncKeyState( '8' ) )	g_power_max += 0.01;
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
	*/

		sprintf(DebugStr, "power:%.3f,%.3f,%.3f plus:%.3f reset:%.3f plus_reset:%.3f", g_power, g_power_min, g_power_max, g_power_plus, g_power_rest, g_power_plus_reset );
		sprintf(DebugStr2, "dir_fix_d:%.5f dir_fix_acc:%.5f dir_rnd:%.4f", g_direction_fix_default, g_direction_fix_acceleration, g_direction_random );
	}

	//�f�o�b�O���`��
	char fps[255];
	wsprintf(fps, "FPS : %d, circle : %d", MainLoop.GetFPS(), g_circle_count );
	lpBack->TextOut(0, 0, fps);
	lpBack->TextOut(0, 20, DebugStr);
	lpBack->TextOut(0, 40, DebugStr2);
	lpBack->TextOut(0, 60, DebugStr3);
	//�t���b�v
	lpDirectDraw->Flip();
}

//���I������
void CGameMain::Quit(string str)
{
	//�E�B���h�E�I�����b�Z�[�W
	PostMessage(hWnd, WM_CLOSE, 0, 0);
	MessageBox(hWnd, str.c_str(), "Quit", MB_OK);
}

//���T�C���E�R�T�C��
double CGameMain::Sin(int i)
{
	i += MaxAngle;
	return SinTable[i % MaxAngle];
}
double CGameMain::Cos(int i)
{
	i += MaxAngle;
	return SinTable[(i + MaxAngle / 4) % MaxAngle];
}

void CGameMain::OnKeyDonw(UINT key)
{
}

void CGameMain::OnKeyPress(char key)
{
}

void CGameMain::draw_house( const POINT& mp )
{
	// �w�i�`��
	RGBQUAD orange = { 0x11, 0xAA, 0xFF, 0xFF };
	RGBQUAD dark_orange = { 0x00, 0x66, 0x99, 0xFF };
	RGBQUAD dark_ground = { 50, 80, 50, 0xFF };

	// �e
	for ( int n = 0; n < 10; n++ )
	{
		lpDst->DrawLineHumanTouch( mp.x, mp.y + 10 + n * 1, mp.x + 40 + n * 2, mp.y + 10 + n * 1, dark_ground );
	}

	// ����
	for ( int n = 0; n < 20; n++ )
	{
		if ( n < 15 )
		{
			lpDst->DrawLineHumanTouch( mp.x + n * 1, mp.y, mp.x + n * 1, mp.y + 20, orange );
		}
		else
		{
			lpDst->DrawLineHumanTouch( mp.x + n * 1, mp.y, mp.x + n * 1, mp.y + 20, dark_orange );
		}
	}
}
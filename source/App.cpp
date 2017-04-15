#include "App.h"
#include "GameMain.h"
#include "resource.h"

#include <win/Tablet.h>
#include <win/Rect.h>
#include <win/Point.h>

#include <winnls32.h>

#include <boost/filesystem.hpp>
#include <iostream>

#include "memory.h"

#define ENABLE_DEBUG_CONSOLE

//���R���X�g���N�^
App::App()
	: hWnd( 0 )
	, hInst( 0 )
	, hMutex( 0 )
	, class_name_( "blue-sky" )
	, title_( "blue-sky" )
	, style_( get_window_style() )
	, width_( DEFAULT_WIDTH )
	, height_( DEFAULT_HEIGHT )
	, is_active_( false )
	, is_full_screen_( false )
	, is_mouse_in_window_( false )
	, is_clip_cursor_enabled_( false )
	, config_( new Config() )
{
	/*
	TCHAR exe_path[ MAX_PATH ];
	GetModuleFileName( 0, exe_path, MAX_PATH );

	boost::filesystem::path dir_path = exe_path;
	boost::filesystem::current_path( dir_path.parent_path() );
	*/

	setlocale( LC_CTYPE, "" );

#ifdef ENABLE_DEBUG_CONSOLE
	AllocConsole();
	freopen( "CONOUT$", "w", stdout );
#endif
}

//���f�X�g���N�^
App::~App()
{
	ReleaseMutex( hMutex );			// �~���[�e�b�N�X�J��
	WINNLSEnableIME( hWnd, TRUE );	// IME�\��

	clip_cursor( false );

#ifdef ENABLE_DEBUG_CONSOLE
	FreeConsole();
#endif
}

//��������
bool App::Init(HINSTANCE hi, int nCmdShow)
{
	//�C���X�^���X�n���h�����R�s�[
	hInst = hi;
	//�Q�d�N���h�~
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, title_.c_str() );
	if(hMutex != NULL){
		CloseHandle(hMutex);
		return false;
	}
	hMutex = CreateMutex(NULL, FALSE, title_.c_str());

	//WNDCLASS�\����
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,				//�X�^�C��
		WinProc,								//�v���V�[�W���֐�
		NULL,									//�g���p�p�����[�^
		NULL,									//�g���p�p�����[�^
		hInst,									//�C���X�^���X�n���h��
		LoadIcon( hi, MAKEINTRESOURCE( IDI_ICON1 ) ),			//�A�C�R��
		LoadCursor( NULL, IDC_ARROW ),							//�}�E�X�J�[�\��
		(HBRUSH)GetStockObject(BLACK_BRUSH),	//�w�i�F
		NULL,									//���j���[
		class_name_.c_str(),						//�N���X��
	};
	//�E�B���h�E�N���X�̓o�^
	if(! RegisterClass(&wc))	return false;
	
	// �E�B���h�E�T�C�Y�̎擾
	RECT rc = { 0, 0, width_, height_ };
	AdjustWindowRect( & rc, style_, FALSE );
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	//�E�C���h�E�쐬
	hWnd = CreateWindowEx(
		0 /* WS_EX_TOPMOST */,		//��O�ɕ\��
		class_name_.c_str(),	//�N���X��
		title_.c_str(),		//�^�C�g��
		style_,				//�X�^�C��
		CW_USEDEFAULT,		//�\�����W
		CW_USEDEFAULT,		//
		w,					//�T�C�Y
		h,					//
		NULL,				//�e�E�B���h�E�̃n���h��
		NULL,				//���j���[�̃n���h��
		hInst,				//�C���X�^���X�n���h��
		NULL				//�E�B���h�E�쐬�f�[�^ �A�h���X
	);

	if ( ! hWnd )
	{
		return false;
	}
	
	// �ݒ��ǂݍ���
	config_->load_file( "blue-sky.config" );

	set_size( get_config()->get( "video.width", DEFAULT_WIDTH ), get_config()->get( "video.height", DEFAULT_HEIGHT ) );
	set_full_screen( get_config()->get( "video.full_screen", 0 ) != 0 );

	// �Q�[��������������
	blue_sky::GameMain::get_instance()->setup_scene();
	game_ = blue_sky::GameMain::get_instance();

	ShowWindow( hWnd, nCmdShow );		//�\��
	UpdateWindow( hWnd );				//�`��

	// WINNLSEnableIME(hWnd, FALSE);	//IME��\��

	return true;
}

//�����b�Z�[�W���[�v
int App::MessageLoop()
{
	MSG msg;

	while ( true )
	{
		if ( PeekMessage( & msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage( & msg );
			DispatchMessage( & msg );
		}
		else
		{
			if ( ! game_->update() )
			{
				Sleep( 0 );
			}
		}
	}

	return msg.wParam;
}

//���E�B���h�E�v���V�[�W��
LRESULT CALLBACK App::WinProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch ( msg )
	{
	case WM_CREATE:
	{
		SetCursor( 0 );
		break;
	}
	case WM_SIZE:
	{
		// int w = ( lp >>  0 ) & 0xFFFF;
		// int h = ( lp >> 16 ) & 0xFFFF;

		on_resize( hwnd );
		break;
	}
	case WM_KEYDOWN:
	{
		if ( App::GetInstance()->game_ )
		{
			App::GetInstance()->game_->on_function_key_down( wp - VK_F1 + 1 );
		}

		break;
	}
	case WM_MOUSEMOVE:
	{
		if ( ! App::GetInstance()->is_mouse_in_window_ )
		{
			App::GetInstance()->is_mouse_in_window_ = true;

			ShowCursor( FALSE );

			TRACKMOUSEEVENT track_mouse_event = { sizeof( TRACKMOUSEEVENT ), TME_LEAVE, hwnd };
			TrackMouseEvent( & track_mouse_event );
		}
		break;
	}
	case WM_MOUSELEAVE:
	{
		App::GetInstance()->is_mouse_in_window_ = false;

		ShowCursor( TRUE );

		break;
	}
	case WM_MOUSEWHEEL:
	{
		if ( App::GetInstance()->game_ )
		{
			App::GetInstance()->game_->on_mouse_wheel( GET_WHEEL_DELTA_WPARAM( wp ) );
		}

		break;
	}
	case WM_ACTIVATEAPP:
	{
		App::GetInstance()->set_active( LOWORD( wp ) != 0 );
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage( 0 );
		break;
	}
	case WM_ACTIVATE:
	{
		win::Tablet::get_instance( hwnd )->on_activate( wp, lp );
		break;
	}
	case WT_PACKET:
	{
		win::Tablet::get_instance( hwnd )->on_packet( wp, lp );
		break;
	}
	default:
		return DefWindowProc( hwnd, msg, wp, lp );
	}

	return	0;
}

void App::set_active( bool active )
{
	is_active_ = active;

	if ( active )
	{
		App::GetInstance()->clip_cursor( is_clip_cursor_enabled_ );
	}
}

void App::on_resize( HWND /* hwnd */ )
{
	if ( ! App::GetInstance()->hWnd )
	{
		return;
	}

	// App::GetInstance()->clip_cursor( App::GetInstance()->is_clip_cursor_enabled_ );
	
	if ( App::GetInstance()->game_ )
	{
		App::GetInstance()->game_->on_resize();
	}
}

void App::clip_cursor( bool clip )
{
	is_clip_cursor_enabled_ = clip;

	if ( is_clip_cursor_enabled_ )
	{
		RECT rect;
		GetClientRect( hWnd, & rect );

		POINT point = { rect.left, rect.top };
		ClientToScreen( hWnd, & point );
	
		rect.left = point.x;
		rect.top = point.y;
		rect.right += point.x;
		rect.bottom += point.y;

		ClipCursor( & rect );
	}
	else
	{
		ClipCursor( 0 );
	}
}

void App::set_size( int w, int h )
{
	width_ = w;
	height_ = h;

	int sw = GetSystemMetrics( SM_CXSCREEN );
	int sh = GetSystemMetrics( SM_CYSCREEN );

	
	RECT rect = { 0, 0, width_, height_ };
	AdjustWindowRect( & rect, style_, false );

	int ww = rect.right - rect.left;
	int wh = rect.bottom - rect.top;

	SetWindowPos( hWnd, HWND_NOTOPMOST, ( sw - ww ) / 2, ( sh - wh ) / 2, ww, wh, SWP_SHOWWINDOW );

	// PostMessage( hWnd, WM_EXITSIZEMOVE, 0, 0 );
}

/**
 * �^�C�g�����擾����
 *
 * @return �^�C�g��
 */
const App::char_t* App::get_title()
{
	if ( hWnd )
	{
		title_.resize( GetWindowTextLength( hWnd ) + 1 );
		GetWindowText( hWnd, & title_[ 0 ], title_.size() );
	}

	return title_.c_str();
}

/**
 * �^�C�g����ݒ肷��
 *
 * @param t �^�C�g��
 */
void App::set_title( const char_t* t )
{
	title_ = t;

	if ( hWnd )
	{
		SetWindowText( hWnd, title_.c_str() );
	}
}

void App::set_full_screen( bool full_screen )
{
	/*
	if ( full_screen )
	{
		if ( ! is_full_screen_ )
		{
			GetWindowRect( hWnd, & last_window_rect_ );
			ShowCursor( FALSE );
		}

		SetWindowLong( hWnd, GWL_STYLE, get_window_style_full_scrren() );
	}
	else
	{
		SetWindowLong( hWnd, GWL_STYLE, get_window_style() );

		if ( is_full_screen_ )
		{
			SetWindowPos(  hWnd, HWND_NOTOPMOST, last_window_rect_.left, last_window_rect_.top, last_window_rect_.right - last_window_rect_.left, last_window_rect_.bottom - last_window_rect_.top, SWP_SHOWWINDOW );
			ShowCursor( TRUE );
		}
	}
	*/

	is_full_screen_ = full_screen;
}

LONG App::get_window_style() const
{
	return WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
}

LONG App::get_window_style_full_scrren() const
{
	return WS_POPUP;
}

void App::close()
{
	PostMessage( hWnd, WM_CLOSE, 0, 0 );
}
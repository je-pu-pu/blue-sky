#include "App.h"
#include "GameMain.h"

#include <winnls32.h>

#include <iostream>

//���R���X�g���N�^
App::App()
	: hWnd( 0 )
	, hInst( 0 )
	, hMutex( 0 )
	, width_( DEFAULT_WIDTH )
	, height_( DEFAULT_HEIGHT )
	, is_full_screen_( false )
{
	ClassName = "blue-sky";
	WinTitle = "blue-sky";
	WinStyle = get_window_style();
}

//���f�X�g���N�^
App::~App()
{
	ReleaseMutex(hMutex);			//�~���[�e�b�N�X�J��
	WINNLSEnableIME(hWnd, TRUE);	//IME�\��
	ShowCursor( TRUE );				//�J�[�\���\��
}

//��������
bool App::Init(HINSTANCE hi, int nCmdShow)
{
	//�C���X�^���X�n���h�����R�s�[
	hInst = hi;
	//�Q�d�N���h�~
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, WinTitle.c_str());
	if(hMutex != NULL){
		CloseHandle(hMutex);
		return false;
	}
	hMutex = CreateMutex(NULL, FALSE, WinTitle.c_str());
	//WNDCLASS�\����
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,				//�X�^�C��
		WinProc,								//�v���V�[�W���֐�
		NULL,									//�g���p�p�����[�^
		NULL,									//�g���p�p�����[�^
		hInst,									//�C���X�^���X�n���h��
		LoadIcon(NULL, IDI_WINLOGO),			//�A�C�R��
		LoadCursor(NULL, IDC_ARROW),			//�}�E�X�J�[�\��
		(HBRUSH)GetStockObject(BLACK_BRUSH),	//�w�i�F
		NULL,									//���j���[
		ClassName.c_str(),						//�N���X��
	};
	//�E�B���h�E�N���X�̓o�^
	if(! RegisterClass(&wc))	return false;
	
	// �E�B���h�E�T�C�Y�̎擾
	RECT rc = { 0, 0, width_, height_ };
	AdjustWindowRect( & rc, WinStyle, FALSE );
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	//�E�C���h�E�쐬
	hWnd = CreateWindowEx(
		0 /* WS_EX_TOPMOST */,		//��O�ɕ\��
		ClassName.c_str(),	//�N���X��
		WinTitle.c_str(),	//�^�C�g��
		WinStyle,			//�X�^�C��
		CW_USEDEFAULT ,		//�\�����W
		CW_USEDEFAULT ,		//
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
	
	// �Q�[��������������
	blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

	ShowWindow( hWnd, nCmdShow );		//�\��
	UpdateWindow( hWnd );				//�`��

	// WINNLSEnableIME(hWnd, FALSE);	//IME��\��
	
	// ShowCursor( FALSE );			// �J�[�\����\��

	return true;
}

//�����b�Z�[�W���[�v
int App::MessageLoop()
{
	MSG msg;

	blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

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
			game->update();
		}
	}

	return msg.wParam;
}

//���E�B���h�E�v���V�[�W��
LRESULT CALLBACK App::WinProc( HWND hw, UINT msg, WPARAM wp, LPARAM lp )
{
	switch ( msg )
	{
	case	WM_CREATE:		break;
	case	WM_KEYDOWN:
		if ( wp == VK_ESCAPE )
		{
			PostMessage( hw, WM_CLOSE, 0, 0 );
		}
		break;
	case	WM_CHAR:		break;
	case	WM_ACTIVATEAPP:	break;
	case	WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc(hw, msg, wp, lp);
	}

	return	0;
}

void App::set_size( int w, int h )
{
	width_ = w;
	height_ = h;

	RECT rect;

	GetWindowRect( hWnd, & rect );
	SetWindowPos( hWnd, HWND_NOTOPMOST, rect.left, rect.top, width_, height_, SWP_SHOWWINDOW );
}

/**
 * �^�C�g�����擾����
 *
 * @return �^�C�g��
 */
const char* App::getTitle()
{
	title_.resize( GetWindowTextLength( hWnd ) + 1 );
	GetWindowText( hWnd, & title_[ 0 ], title_.size() );

	return title_.c_str();
}

/**
 * �^�C�g����ݒ肷��
 *
 * @param t �^�C�g��
 */
void App::setTitle( const char* t )
{
	SetWindowText( hWnd, t );
}

void App::set_full_screen( bool full_screen )
{
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

	is_full_screen_ = full_screen;
}

LONG App::get_window_style() const
{
	return WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	// return WS_OVERLAPPEDWINDOW;
}

LONG App::get_window_style_full_scrren() const
{
	return WS_POPUP;
}
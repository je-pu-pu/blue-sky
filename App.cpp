#include "App.h"
#include "GameMain.h"

#include <winnls32.h>

#include <iostream>

//���R���X�g���N�^
CApp::CApp()
	: hWnd( 0 )
	, hInst( 0 )
	, hMutex( 0 )
	, Width( 720 )
	, Height( 480 )
{
	ClassName = "blue-sky";
	WinTitle = "blue-sky";
	// WinStyle = WS_POPUP;
	// WinStyle = WS_CAPTION;
	
	WinStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	// WinStyle = WS_POPUP;
}

//���f�X�g���N�^
CApp::~CApp()
{
	ReleaseMutex(hMutex);			//�~���[�e�b�N�X�J��
	WINNLSEnableIME(hWnd, TRUE);	//IME�\��
	ShowCursor(TRUE);				//�J�[�\���\��
}

//��������
bool CApp::Init(HINSTANCE hi, int nCmdShow)
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
	//�E�B���h�E�T�C�Y�̎擾
	RECT rc = {0, 0, Width, Height};
	AdjustWindowRect(&rc, WinStyle, FALSE);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	//�E�C���h�E�쐬
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST,		//��O�ɕ\��
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
int CApp::MessageLoop()
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
LRESULT CALLBACK CApp::WinProc( HWND hw, UINT msg, WPARAM wp, LPARAM lp )
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

/**
 * �^�C�g�����擾����
 *
 * @return �^�C�g��
 */
const char* CApp::getTitle()
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
void CApp::setTitle( const char* t )
{
	SetWindowText( hWnd, t );
}

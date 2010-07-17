#include "App.h"
#include "GameMain.h"

#include <winnls32.h>

#include <iostream>

//□コンストラクタ
App::App()
	: hWnd( 0 )
	, hInst( 0 )
	, hMutex( 0 )
	, width_( DEFAULT_WIDTH )
	, height_( DEFAULT_HEIGHT )
	, is_full_screen_( false )
	, is_clip_cursor_enabled_( false )
{
	ClassName = "blue-sky";
	WinTitle = "blue-sky";
	WinStyle = get_window_style();
}

//□デストラクタ
App::~App()
{
	ReleaseMutex(hMutex);			//ミューテックス開放
	WINNLSEnableIME(hWnd, TRUE);	//IME表示

	ClipCursor( 0 );
	ShowCursor( TRUE );				//カーソル表示
}

//■初期化
bool App::Init(HINSTANCE hi, int nCmdShow)
{
	//インスタンスハンドルをコピー
	hInst = hi;
	//２重起動防止
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, WinTitle.c_str());
	if(hMutex != NULL){
		CloseHandle(hMutex);
		return false;
	}
	hMutex = CreateMutex(NULL, FALSE, WinTitle.c_str());
	//WNDCLASS構造体
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,				//スタイル
		WinProc,								//プロシージャ関数
		NULL,									//拡張用パラメータ
		NULL,									//拡張用パラメータ
		hInst,									//インスタンスハンドル
		LoadIcon(NULL, IDI_WINLOGO),			//アイコン
		LoadCursor(NULL, IDC_ARROW),			//マウスカーソル
		(HBRUSH)GetStockObject(BLACK_BRUSH),	//背景色
		NULL,									//メニュー
		ClassName.c_str(),						//クラス名
	};
	//ウィンドウクラスの登録
	if(! RegisterClass(&wc))	return false;
	
	// ウィンドウサイズの取得
	RECT rc = { 0, 0, width_, height_ };
	AdjustWindowRect( & rc, WinStyle, FALSE );
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	//ウインドウ作成
	hWnd = CreateWindowEx(
		0 /* WS_EX_TOPMOST */,		//手前に表示
		ClassName.c_str(),	//クラス名
		WinTitle.c_str(),	//タイトル
		WinStyle,			//スタイル
		CW_USEDEFAULT,		//表示座標
		CW_USEDEFAULT,		//
		w,					//サイズ
		h,					//
		NULL,				//親ウィンドウのハンドル
		NULL,				//メニューのハンドル
		hInst,				//インスタンスハンドル
		NULL				//ウィンドウ作成データ アドレス
	);

	if ( ! hWnd )
	{
		return false;
	}
	
	// ゲームを初期化する
	blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

	ShowWindow( hWnd, nCmdShow );		//表示
	UpdateWindow( hWnd );				//描画

	// WINNLSEnableIME(hWnd, FALSE);	//IME非表示

	ShowCursor( FALSE );			// カーソル非表示

	return true;
}

//□メッセージループ
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
			if ( ! game->update() )
			{
				Sleep( 1 );
			}
		}
	}

	return msg.wParam;
}

//□ウィンドウプロシージャ
LRESULT CALLBACK App::WinProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch ( msg )
	{
	case WM_CREATE: break;
	case WM_EXITSIZEMOVE: on_resize( hwnd ); break;
	case WM_KEYDOWN:
	{
		blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

		if ( wp == VK_ESCAPE ) game->on_reset_key_down();
		if ( wp >= VK_F1 && wp <= VK_F24 ) game->on_function_key_down( wp - VK_F1 + 1 );
		break;
	}
	case WM_MOUSEWHEEL:
	{
		blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

		game->on_mouse_wheel( GET_WHEEL_DELTA_WPARAM( wp ) );
		break;
	}
	case WM_CHAR:		break;
	case WM_ACTIVATEAPP:
		if ( LOWORD( wp ) )
		{
			App::GetInstance()->clip_cursor( App::GetInstance()->is_clip_cursor_enabled_ );
		}
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hwnd, msg, wp, lp );
	}

	return	0;
}

void App::on_resize( HWND hwnd )
{
	App::GetInstance()->clip_cursor( App::GetInstance()->is_clip_cursor_enabled_ );
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
	AdjustWindowRect( & rect, WinStyle, false );

	int ww = rect.right - rect.left;
	int wh = rect.bottom - rect.top;

	SetWindowPos( hWnd, HWND_NOTOPMOST, ( sw - ww ) / 2, ( sh - wh ) / 2, ww, wh, SWP_SHOWWINDOW );

	PostMessage( hWnd, WM_EXITSIZEMOVE, 0, 0 );
}

/**
 * タイトルを取得する
 *
 * @return タイトル
 */
const char* App::getTitle()
{
	title_.resize( GetWindowTextLength( hWnd ) + 1 );
	GetWindowText( hWnd, & title_[ 0 ], title_.size() );

	return title_.c_str();
}

/**
 * タイトルを設定する
 *
 * @param t タイトル
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

void App::close()
{
	PostMessage( hWnd, WM_CLOSE, 0, 0 );
}
#include "App.h"
#include "GameMain.h"
#include "GameSettings.h"
#include "resource.h"

#include <win/Tablet.h>
#include <win/Rect.h>
#include <win/Point.h>
#include <win/LastError.h>

#include <common/string.h>

#include <winnls32.h>

#include <boost/filesystem.hpp>
#include <iostream>

// デバッグ用のコンソール（std::cout の出力先）。リリースビルドでは開かない
#ifdef _DEBUG
#define ENABLE_DEBUG_CONSOLE
#endif

//□コンストラクタ
App::App()
	: hInst( nullptr )
	, hWnd( nullptr )
	, hMutex( nullptr )
	, class_name_( "blue-sky" )
	, title_( "blue-sky" )
	, style_( get_window_style() )
	, width_( DEFAULT_WIDTH )
	, height_( DEFAULT_HEIGHT )
	, is_active_( false )
	, is_full_screen_( false )
	, last_window_rect_{ 0, 0, 0, 0 }
	, is_mouse_in_window_( false )
	, is_clip_cursor_enabled_( false )
	, config_( new Config() )
	, user_config_( new Config() )
{
	/*
	TCHAR exe_path[ MAX_PATH ];
	GetModuleFileName( 0, exe_path, MAX_PATH );

	boost::filesystem::path dir_path = exe_path;
	boost::filesystem::current_path( dir_path.parent_path() );
	*/

	setlocale( LC_CTYPE, "" );

#ifdef ENABLE_DEBUG_CONSOLE
	// コンソールを開けなくてもゲームは続ける（出力が見えないだけ）
	if ( AllocConsole() )
	{
		FILE* out = nullptr;
		freopen_s( & out, "CONOUT$", "w", stdout );
	}
#endif
}

//□デストラクタ
App::~App()
{
	ReleaseMutex( hMutex );			// ミューテックス開放
	WINNLSEnableIME( hWnd, TRUE );	// IME表示

	clip_cursor( false );

#ifdef ENABLE_DEBUG_CONSOLE
	FreeConsole();
#endif
}

//■初期化
bool App::Init(HINSTANCE hi, int nCmdShow)
{
	// 設定を読み込む : 開発用・エンジンの設定と、ユーザーの設定（無ければ前者から引き継ぐ）
	config_->load_file( "blue-sky.config" );
	blue_sky::game_settings::load_user_settings_file( * user_config_, * config_ );

	// 表示設定は、ゲーム中のストアと同じ登録（既定値・範囲）で読む
	// （以前は App とストアがばらばらに読み、既定値が 800x600 と 1920x1080 の 2 つあり、範囲の検査も違った）
	bool initial_full_screen = false;
	blue_sky::game_settings::read_initial_display( * user_config_, width_, height_, initial_full_screen );

	//インスタンスハンドルをコピー
	hInst = hi;
	//２重起動防止
	hMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, title_.c_str() );
	
	if ( hMutex )
	{
		CloseHandle(hMutex);
		return false;
	}

	hMutex = CreateMutex( nullptr, FALSE, title_.c_str());

	//WNDCLASS構造体
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,				//スタイル
		WinProc,								//プロシージャ関数
		0,										//拡張用パラメータ
		0,										//拡張用パラメータ
		hInst,									//インスタンスハンドル
		LoadIcon( hi, MAKEINTRESOURCE( IDI_ICON1 ) ),			//アイコン
		LoadCursor( nullptr, IDC_ARROW ),		//マウスカーソル
		(HBRUSH)GetStockObject(BLACK_BRUSH),	//背景色
		nullptr,								//メニュー
		class_name_.c_str(),					//クラス名
	};
	//ウィンドウクラスの登録
	if(! RegisterClass(&wc))	return false;

	// ウィンドウサイズの取得
	RECT rc = { 0, 0, width_, height_ };
	AdjustWindowRect( & rc, style_, FALSE );
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = ( GetSystemMetrics( SM_CXSCREEN ) - w ) / 2;
	int y = ( GetSystemMetrics( SM_CYSCREEN ) - h ) / 2;

	//ウインドウ作成
	hWnd = CreateWindowEx(
		0 /* WS_EX_TOPMOST */,		//手前に表示
		class_name_.c_str(),	//クラス名
		title_.c_str(),		//タイトル
		style_,				//スタイル
		x,					//表示座標
		y,					//
		w,					//サイズ
		h,					//
		nullptr,			//親ウィンドウのハンドル
		nullptr,			//メニューのハンドル
		hInst,				//インスタンスハンドル
		nullptr				//ウィンドウ作成データ アドレス
	);

	if ( ! hWnd )
	{
		return false;
	}
	
	set_full_screen( initial_full_screen );

	// ゲームを初期化する
	game_ = blue_sky::GameMain::get_instance();
	blue_sky::GameMain::get_instance()->setup_scene();
	
	ShowWindow( hWnd, nCmdShow );		//表示
	UpdateWindow( hWnd );				//描画

	// WINNLSEnableIME(hWnd, FALSE);	//IME非表示

	return true;
}

//□メッセージループ
int App::MessageLoop()
{
	MSG msg;

	while ( true )
	{
		if ( PeekMessage( & msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage( & msg );
			DispatchMessage( & msg );

			// ウィンドウプロシージャの中で起きた例外は、ここで投げ直す（WinMain が受け止める）
			if ( pending_exception_ )
			{
				std::exception_ptr e = pending_exception_;
				pending_exception_ = nullptr;
				std::rethrow_exception( e );
			}
		}
		else
		{
			if ( ! game_->update() )
			{
				// 次のフレームまで眠る（メッセージが来たらすぐ起きる）
				// 以前は Sleep( 0 ) で回り続け、CPU のコアを 1 つ使い切っていた
				const DWORD wait = game_->get_idle_wait_msec();

				if ( wait > 0 )
				{
					MsgWaitForMultipleObjects( 0, nullptr, FALSE, wait, QS_ALLINPUT );
				}
			}
		}
	}

	// 正常に終わったときだけ、設定などを保存して後片付けする（例外で抜けたときは、壊れた状態を保存しない）
	if ( game_ )
	{
		game_->shutdown();
	}

	return static_cast< int >( msg.wParam );
}

/**
 * ウィンドウプロシージャ
 *
 * 例外を user32 の外へ出さない（x64 ではコールバックをまたぐ例外が握りつぶされることがある）。
 * 中で起きた例外は預けておき、メッセージループに戻ってから投げ直す。
 */
LRESULT CALLBACK App::WinProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	try
	{
		return WinProcImpl( hwnd, msg, wp, lp );
	}
	catch ( ... )
	{
		App::get_instance()->pending_exception_ = std::current_exception();
		return 0;
	}
}

LRESULT App::WinProcImpl( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	if ( ImGui_ImplWin32_WndProcHandler( hwnd, msg, wp, lp ) )
	{
		return true;
	}

	switch ( msg )
	{
	case WM_CREATE:
	{
		SetCursor( nullptr );
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
		// 押し続けによるキーリピートは、ファンクションキー（ミュート・フルスクリーンなどの切り替え）には渡さない
		const bool is_repeat = ( lp & ( 1 << 30 ) ) != 0;
		const bool is_function_key = wp >= VK_F1 && wp <= VK_F24;

		if ( App::get_instance()->game_ && ! ( is_repeat && is_function_key ) )
		{
			App::get_instance()->game_->on_special_key_down( static_cast< int >( wp ) );
		}

		break;
	}
	case WM_SYSKEYDOWN:
	{
		// Alt+Enter : フルスクリーンの切り替え（DXGI の自動処理は止めてある）
		if ( wp == VK_RETURN && ( lp & ( 1 << 29 ) ) && ! ( lp & ( 1 << 30 ) ) )
		{
			if ( App::get_instance()->game_ )
			{
				App::get_instance()->game_->on_toggle_full_screen();
			}

			return 0;
		}

		return DefWindowProc( hwnd, msg, wp, lp );
	}
	case WM_SYSCOMMAND:
	{
		// Alt や F10 を単独で押したときに、ウィンドウメニューのモードに入ってゲームの入力が奪われないようにする
		if ( ( wp & 0xFFF0 ) == SC_KEYMENU )
		{
			return 0;
		}

		return DefWindowProc( hwnd, msg, wp, lp );
	}
	case WM_CHAR:
	{
		if ( App::get_instance()->game_ )
		{
			App::get_instance()->game_->on_key_down( static_cast< char_t >( wp ) );
		}

		break;
	}
	case WM_MOUSEMOVE:
	{
		if ( ! App::get_instance()->is_mouse_in_window_ )
		{
			App::get_instance()->is_mouse_in_window_ = true;
			App::get_instance()->show_cursor( App::get_instance()->game_->is_show_cursor() );

			TRACKMOUSEEVENT track_mouse_event = { sizeof( TRACKMOUSEEVENT ), TME_LEAVE, hwnd };
			TrackMouseEvent( & track_mouse_event );
		}

		break;
	}
	case WM_MOUSELEAVE:
	{
		App::get_instance()->is_mouse_in_window_ = false;
		App::get_instance()->show_cursor( App::get_instance()->game_->is_show_cursor() );

		break;
	}
	case WM_MOUSEWHEEL:
	{
		if ( App::get_instance()->game_ )
		{
			App::get_instance()->game_->on_mouse_wheel( GET_WHEEL_DELTA_WPARAM( wp ) );
		}

		break;
	}
	case WM_ACTIVATEAPP:
	{
		App::get_instance()->set_active( LOWORD( wp ) != 0 );

		if ( wp )
		{
			// フォーカスを失って解除された排他フルスクリーンを、設定に合わせて戻す
			if ( App::get_instance()->game_ )
			{
				App::get_instance()->game_->on_activated();
			}
		}

		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage( 0 );
		break;
	}
	case WM_ACTIVATE:
	{
		if ( win::Tablet::is_enabled() )
		{
			win::Tablet::get_instance( hwnd )->on_activate( wp, lp );
		}
		break;
	}
	case WT_PACKET:
	{
		if ( win::Tablet::is_enabled() )
		{
			win::Tablet::get_instance( hwnd )->on_packet( wp, lp );
		}
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
		App::get_instance()->clip_cursor( is_clip_cursor_enabled_ );
	}
}

void App::on_resize( HWND /* hwnd */ )
{
	if ( ! App::get_instance()->hWnd )
	{
		return;
	}

	// App::GetInstance()->clip_cursor( App::GetInstance()->is_clip_cursor_enabled_ );
	
	if ( App::get_instance()->game_ )
	{
		App::get_instance()->game_->on_resize();
	}
}

/**
 * カーソルの表示を設定する
 *
 * @param show カーソルを表示する場合は true を、表示しない場合は false を指定する 
 */
void App::show_cursor( bool show )
{
	if ( show != is_show_cursor_enabled_ )
	{
		is_show_cursor_enabled_ = show;

		ShowCursor( show );
		// std::cout << "ShowCursor : " << ShowCursor( show ) << std::endl;
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
		ClipCursor( nullptr );
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
 * タイトルを取得する
 *
 * @return タイトル
 */
const char_t* App::get_title()
{
	if ( hWnd )
	{
		title_.resize( static_cast< string_t::size_type >( GetWindowTextLength( hWnd ) + 1 ) );
		GetWindowText( hWnd, & title_[ 0 ], static_cast< int >( title_.size() ) );
	}

	return title_.c_str();
}

/**
 * タイトルを設定する
 *
 * @param t タイトル
 */
void App::set_title( const char_t* t )
{
	title_ = t;

	if ( hWnd )
	{
		SetWindowText( hWnd, title_.c_str() );
	}
}

/**
 * エラーメッセージを表示する
 *
 * @param message エラーメッセージ
 */
void App::show_error_message( const char_t* message ) const
{
	// メッセージは UTF-8（ソースは /utf-8）。ANSI 版の MessageBox では日本語が化けるので UTF-16 にして出す
	MessageBoxW( hWnd, common::convert_to_wstring( message ).c_str(), L"エラー", MB_OK | MB_ICONERROR );
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

DWORD App::get_window_style() const
{
	return WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
}

DWORD App::get_window_style_full_scrren() const
{
	return WS_POPUP;
}

/**
 * ディレクトリが変更された際に実行するハンドラを登録する
 * 
 * @param
 */
void App::watch_directory_change( const char* dir_path, std::function< void () > f )
{
	HANDLE h = FindFirstChangeNotification( dir_path, true, FILE_NOTIFY_CHANGE_LAST_WRITE );

	if ( h == INVALID_HANDLE_VALUE )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "App::watch_directory_change() " ) + dir_path + " failed." )
	}

	watch_directory_change_handler_list_[ h ] = f;
}

/**
 * ディレクトリの変更をチェックする
 * 
 */
void App::check_directory_change() const
{
	for ( const auto& i: watch_directory_change_handler_list_ )
	{
		const auto result = WaitForSingleObject( i.first, 0 );

		if ( result == WAIT_FAILED )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( win::LastError().get_error_message() );
		}

		if ( result == WAIT_OBJECT_0 )
		{
			i.second();

			// 監視を継続する
			if ( ! FindNextChangeNotification( i.first ) )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( win::LastError().get_error_message() );
			}
		}
	}
}

void App::close()
{
	PostMessage( hWnd, WM_CLOSE, 0, 0 );
}
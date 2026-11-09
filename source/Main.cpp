#include <blue_sky/App.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/CrashReport.h>

#include <core/graphics/DeviceLostException.h>

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>
#include <common/string.h>

#include <exception>
#include <string>

#include <windows.h>
#include <objbase.h>

namespace
{

/// デバイスロストで起動し直した回数を子プロセスへ渡す環境変数（起動し直しが続くときに止めるため）
constexpr const wchar_t* DEVICE_LOST_RESTART_COUNT_ENV = L"BLUE_SKY_DEVICE_LOST_RESTARTS";

/// 起動し直したときの親プロセスの ID（二重起動の防止に掛からないよう、親が終わるまで待つ）
constexpr const wchar_t* RESTART_PARENT_PID_ENV = L"BLUE_SKY_RESTART_PARENT_PID";

/// デバイスロストで続けて起動し直す回数の上限（GPU が壊れているなど、何度起動しても失われる場合に止める）
constexpr int MAX_DEVICE_LOST_RESTARTS = 2;

int get_device_lost_restart_count()
{
	wchar_t buffer[ 16 ] = { };

	if ( GetEnvironmentVariableW( DEVICE_LOST_RESTART_COUNT_ENV, buffer, 16 ) == 0 )
	{
		return 0;
	}

	return _wtoi( buffer );
}

/**
 * 同じ引数・同じ作業ディレクトリでゲームを起動し直す
 *
 * @return 起動できたら true
 */
bool restart_self( int restart_count )
{
	wchar_t exe_path[ MAX_PATH ] = { };

	if ( GetModuleFileNameW( nullptr, exe_path, MAX_PATH ) == 0 )
	{
		return false;
	}

	// 子プロセスは環境変数を受け継ぐ
	SetEnvironmentVariableW( DEVICE_LOST_RESTART_COUNT_ENV, std::to_wstring( restart_count ).c_str() );
	SetEnvironmentVariableW( RESTART_PARENT_PID_ENV, std::to_wstring( GetCurrentProcessId() ).c_str() );

	std::wstring command_line = GetCommandLineW();		// CreateProcessW は書き換え可能なバッファを要求する

	STARTUPINFOW startup_info = { };
	startup_info.cb = sizeof( startup_info );
	PROCESS_INFORMATION process_info = { };

	if ( ! CreateProcessW( exe_path, command_line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, & startup_info, & process_info ) )
	{
		return false;
	}

	CloseHandle( process_info.hThread );
	CloseHandle( process_info.hProcess );

	return true;
}

/**
 * 起動し直されたプロセスなら、親プロセスが終わるまで待つ
 *
 * 二重起動の防止（App::Init のミューテックス）は親が終わるまで解けないので、待たないと黙って終了してしまう。
 */
void wait_for_restart_parent()
{
	wchar_t buffer[ 16 ] = { };

	if ( GetEnvironmentVariableW( RESTART_PARENT_PID_ENV, buffer, 16 ) == 0 )
	{
		return;
	}

	SetEnvironmentVariableW( RESTART_PARENT_PID_ENV, nullptr );		// 自分がさらに起動し直すときに残らないように消す

	if ( HANDLE parent = OpenProcess( SYNCHRONIZE, FALSE, static_cast< DWORD >( _wtoi( buffer ) ) ) )
	{
		WaitForSingleObject( parent, 10 * 1000 );
		CloseHandle( parent );
	}
}

/**
 * デバイスロストから復旧する（設計上の問題 A9）
 *
 * GPU のリソースは各所が持っていて、その場で全部は作り直せないので、確認してからゲームを起動し直す。
 * 設定は変更のたびに保存しているので失われない。起動し直しが続くときは、終了の案内だけにする。
 */
void recover_from_device_lost( App* app, const std::string& message )
{
	const int count = get_device_lost_restart_count();

	if ( count < MAX_DEVICE_LOST_RESTARTS )
	{
		const std::wstring text = common::convert_to_wstring( message + "\n\nゲームを起動し直しますか？" );

		if ( MessageBoxW( nullptr, text.c_str(), L"blue-sky", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON1 ) == IDYES )
		{
			if ( restart_self( count + 1 ) )
			{
				common::log( "log/error.log", "デバイスロストのため起動し直しました" );
				return;
			}

			common::log( "log/error.log", "デバイスロストの後、起動し直せませんでした" );
		}

		return;
	}

	if ( app )
	{
		app->show_error_message( ( message + "\n\n起動し直しても GPU のデバイスが失われるため、終了します。" ).c_str() );
	}
}

} // namespace

/**
 * @brief メイン
 * @param hInst 
 * @param  
 * @param  
 * @param nCmdShow 
 * @return 
 */
int WINAPI WinMain( _In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nCmdShow )
{
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// 画面の拡大率（DPI）をモニターごとに自分で扱うと宣言する。
	// 宣言しないと、拡大率 150% などの環境でウィンドウを OS がビットマップで拡大し、画像も文字もぼける
	SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	// COM を明示的に初期化する（WIC の画像読み込みなどが使う）。
	// 以前はエンジンが初期化しておらず、どこか（音声の初期化など）が暗黙に初期化していたので動いていた
	const HRESULT com_result = CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );

	struct ComScope
	{
		bool initialized;
		~ComScope() { if ( initialized ) { CoUninitialize(); } }
	} com_scope { SUCCEEDED( com_result ) };

	// 落ちたときに原因（例外の種類と呼び出し履歴）を log/crash.log に残す
	blue_sky::install_crash_report();

	wait_for_restart_parent();

	App* app = 0;
	
	try
	{
		// アプリケーションを初期化する
		app = App::get_instance();
		app->set_title( "blue-sky 2 ( update 1.0.4 )" );
		app->set_class_name( "blue-sky-2" );
		app->set_size( 800, 600 );

		if ( ! app->Init( hInst, nCmdShow ) )
		{
			return 0;
		}

		return app->MessageLoop();
	}
	catch ( const core::graphics::DeviceLostException& e )
	{
		// common::exception< std::string > より先に受け取る（派生クラスなので）
		const std::string message = e.data();

		common::log( "log/error.log", std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n" + message );
		recover_from_device_lost( app, message );

		return -1;
	}
	catch ( const common::exception< HRESULT >& e )
	{
		std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n";

		LPVOID output = 0;
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, static_cast< DWORD >( e.data() ), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )( & output ), 0, 0 );
		message += static_cast< LPCTSTR >( output );

		LocalFree( output );

		common::log( "log/error.log", message );

		if ( app )
		{
			app->show_error_message( message.c_str() );
		}
	}
	catch ( const common::exception< std::string >& e )
	{
		std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n" + e.data();
		
		common::log( "log/error.log", message );

		if ( app )
		{
			app->show_error_message( message.c_str() );
		}

		return -1;
	}
	catch ( const std::exception& e )
	{
		// 標準ライブラリの例外（std::out_of_range など）も、内容を残して表示する
		const std::string message = std::string( "exception : " ) + e.what();

		common::log( "log/error.log", message );

		if ( app )
		{
			app->show_error_message( message.c_str() );
		}
	}
	catch ( ... )
	{
		common::log( "log/error.log", "unknown exception" );

		if ( app )
		{
			app->show_error_message( "Unknown Error" );
		}
	}

	return -1;
}
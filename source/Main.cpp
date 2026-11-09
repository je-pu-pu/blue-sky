#include <blue_sky/App.h>
#include <blue_sky/GameMain.h>

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>

#include <exception>
#include <string>

#include <windows.h>
#include <objbase.h>

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
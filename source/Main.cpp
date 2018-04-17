#include "App.h"
#include "GameMain.h"

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>

#include <windows.h>

//■■■　メイン　■■■
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE /* hPrevInst */, LPSTR /* lpszCmdLine */, int nCmdShow )
{
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	App* app = 0;
	
	try
	{
		// アプリケーションを初期化する
		app = App::GetInstance();
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
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, e.data(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )( & output ), 0, 0 );
		message += static_cast< LPCTSTR >( output );

		LocalFree( output );

		common::log( "log/error.log", message );

		if ( app )
		{
			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}
	}
	catch ( const common::exception< std::string >& e )
	{
		std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n" + e.data();
		
		common::log( "log/error.log", message );

		if ( app )
		{
			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}
	catch ( ... )
	{
		if ( app )
		{
			MessageBox( app->GetWindowHandle(), "Unknown Error", "ERROR", MB_OK );
		}
	}

	return -1;
}
#include "App.h"
#include "GameMain.h"

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>

#include <windows.h>

#include <dxerr.h>

#include "memory.h"

//■■■　メイン　■■■
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow )
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

		// ゲームを初期化する
		Game* game = Game::get_instance();

		// メッセージループ
		return app->MessageLoop();
	}
	catch ( const common::exception< HRESULT >& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n";
			message += std::string( DXGetErrorString( e.data() ) ) + " : " + DXGetErrorDescription( e.data() );

			common::log( "log/error.log", message );

			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}
	}
	catch ( const common::exception< std::string >& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n" + e.data();

			common::log( "log/error.log", message );

			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
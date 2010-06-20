#include "App.h"
#include "GameMain.h"

#include <common/exception.h>
#include <common/serialize.h>

#include <windows.h>

//■■■　メイン　■■■
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow )
{
	CApp* app = 0;
	
	try
	{
		// アプリケーションを初期化する
		app = CApp::GetInstance();
		if( ! app->Init( hInst, nCmdShow) )	return 0;
		
		// ゲームを初期化する
		blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

		// メッセージループ
		return app->MessageLoop();
	}
	catch ( const common::exception& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() );
			
			if ( strlen( e.message() ) )
			{
				message += std::string( "\n\n" ) + e.message();
			}

			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
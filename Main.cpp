#include "App.h"
#include "GameMain.h"

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>

#include <windows.h>

#include <iomanip>
#include <sstream>


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

			time_t t;
			tm* t_st;

			time( & t );
			t_st = localtime( & t );

			std::stringstream ss;

			ss << "error_";
			ss << std::setfill( '0' );
			ss << ( 1900 + t_st->tm_year ) << std::setw( 2 ) << ( 1 + t_st->tm_mon ) << std::setw( 2 ) << t_st->tm_mday;
			ss << "_" << std::setw( 2 ) << t_st->tm_hour << std::setw( 2 ) << t_st->tm_min << std::setw( 2 ) << t_st->tm_sec << ".log";
			common::log( ss.str().c_str(), message );

			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
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
	App* app = 0;
	
	try
	{
		// アプリケーションを初期化する
		app = App::GetInstance();
		if( ! app->Init( hInst, nCmdShow) )	return 0;
		
		// ゲームを初期化する
		blue_sky::GameMain* game = blue_sky::GameMain::getInstance();

		// メッセージループ
		return app->MessageLoop();
	}
	catch ( const common::exception& e )
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
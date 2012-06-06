#include "App.h"
#include "GameMain.h"

#include "DirectX.h"

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
	
	std::string error_file;
	int error_line;
	std::string error_message;

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
	catch ( const common::exception< HRESULT >& e )
	{
		error_file = e.file();
		error_line = e.line();
		error_message = std::string( DXGetErrorString9( e.data() ) ) + " : " + DXGetErrorDescription9( e.data() );
	}
	catch ( const common::exception< std::string >& e )
	{
		error_file = e.file();
		error_line = e.line();
		error_message= e.data();
	}
	catch ( ... )
	{
		if ( app )
		{
			MessageBox( app->GetWindowHandle(), "Unknown Error", "ERROR", MB_OK );
		}
	}

	if ( ! error_file.empty() )
	{
		std::string message = std::string( "exception on " ) + error_file + ":" + common::serialize( error_line );
			
		if ( error_message.length() )
		{
			message += std::string( "\n\n" ) + error_message;
		}

		time_t t;
		tm* t_st;

		time( & t );
		t_st = localtime( & t );

		std::stringstream ss;

		ss << "log/error_";
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

	return -1;
}
#include "App.h"
#include "GameMain.h"

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
		CGameMain* game = CGameMain::GetInstange();

		// メッセージループ
		return app->MessageLoop();
	}
	catch ( std::string message )
	{
		if ( app )
		{
			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
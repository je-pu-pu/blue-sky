
#include	<Windows.h>
#include	"App.h"
#include	"GameMain.h"

//■■■　メイン　■■■
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	//初期化
	CApp *app = CApp::GetInstance();
	if( ! app->Init(hInst, nCmdShow) )	return 0;
	
	//メッセージループ
	int i = app->MessageLoop();
	
	//終了処理
	CGameMain *game = CGameMain::GetInstange();
	game->UnInit();

	return i;
}
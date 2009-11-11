
#include	<Windows.h>
#include	"App.h"
#include	"GameMain.h"

//■■■　メイン　■■■
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	CApp *app = CApp::GetInstance();
	CGameMain *game = CGameMain::GetInstange();
	//初期化
	if( ! app->Init(hInst, nCmdShow) )	return 0;
	//メッセージループ
	int i = app->MessageLoop();
	//終了処理
	game->UnInit();
	return i;
}

#include	<Windows.h>
#include	"App.h"
#include	"GameMain.h"

//�������@���C���@������
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	//������
	CApp *app = CApp::GetInstance();
	if( ! app->Init(hInst, nCmdShow) )	return 0;
	
	//���b�Z�[�W���[�v
	int i = app->MessageLoop();
	
	//�I������
	CGameMain *game = CGameMain::GetInstange();
	game->UnInit();

	return i;
}
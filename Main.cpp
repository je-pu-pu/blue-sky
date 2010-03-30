#include "App.h"
#include "GameMain.h"

#include <windows.h>

//�������@���C���@������
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow )
{
	CApp* app = 0;
	
	try
	{
		// �A�v���P�[�V����������������
		app = CApp::GetInstance();
		if( ! app->Init( hInst, nCmdShow) )	return 0;
		
		// �Q�[��������������
		CGameMain* game = CGameMain::GetInstange();

		// ���b�Z�[�W���[�v
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
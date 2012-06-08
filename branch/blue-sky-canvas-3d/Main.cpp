#include "App.h"
#include "GameMain.h"

#include <common/exception.h>
#include <common/serialize.h>

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
	catch ( const common::exception< std::string >& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() );
			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
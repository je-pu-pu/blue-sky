#include "App.h"
#include "GameMain.h"

#include <common/exception.h>
#include <common/serialize.h>

#include <windows.h>

#include <dxerr.h>

//�������@���C���@������
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow )
{
	App* app = 0;
	
	try
	{
		// �A�v���P�[�V����������������
		app = App::GetInstance();
		app->set_title( "blue-sky 2 preview" );
		app->set_class_name( "blue-sky-2" );
		app->set_size( 800, 600 );

		if ( ! app->Init( hInst, nCmdShow ) )
		{
			return 0;
		}

		// �Q�[��������������
		Game* game = Game::get_instance();

		// ���b�Z�[�W���[�v
		return app->MessageLoop();
	}
	catch ( const common::exception< HRESULT >& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n";
			message += std::string( DXGetErrorString( e.data() ) ) + " : " + DXGetErrorDescription( e.data() );

			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}
	}
	catch ( const common::exception< std::string >& e )
	{
		if ( app )
		{
			std::string message = std::string( "exception on " ) + e.file() + ":" + common::serialize( e.line() ) + "\n" + e.data();
			MessageBox( app->GetWindowHandle(), message.c_str(), "ERROR", MB_OK );
		}

		return -1;
	}

	return -1;
}
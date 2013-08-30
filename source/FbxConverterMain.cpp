#include "App.h"

#include "FbxFileLoader.h"
#include "DirectX.h"
#include "type.h"

#include <common/log.h>
#include <common/exception.h>
#include <common/serialize.h>

#include <boost/filesystem/convenience.hpp>

#include <windows.h>

#include <dxerr.h>

#include "memory.h"



void convert_fbx()
{
	WIN32_FIND_DATA find_data;

	HANDLE find_handle = FindFirstFile( "*.fbx", & find_data );

	if ( find_handle  != INVALID_HANDLE_VALUE )
	{
		FbxFileLoader fbx_file_loader;

		while ( true )
		{
			const string_t dir_path = "./";
			const string_t file_path = dir_path + find_data.cFileName;

			if (  file_path.find( ".bin.fbx", 0 ) == string_t::npos )
			{
				const string_t bin_file_path = dir_path + boost::filesystem::basename( file_path ) + ".bin.fbx";

				fbx_file_loader.convert_to_binaly( file_path.c_str(), bin_file_path.c_str() );
			}

			if ( ! FindNextFile( find_handle, & find_data ) )
			{
				break;
			}
		}

		FindClose( find_handle );
	}
}

void delete_bin_fbx_all()
{
	WIN32_FIND_DATA find_data;

	HANDLE find_handle = FindFirstFile( "media/model/*.bin.fbx", & find_data );

	if ( find_handle  != INVALID_HANDLE_VALUE )
	{
		CHAR exe_file_path[ MAX_PATH ];
			
		if ( GetModuleFileName( NULL, exe_file_path, MAX_PATH ) == 0 )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "GetModuleFileName failed." );
		}

		while ( true )
		{
			const string_t file_path = string_t( "media/model/" ) + find_data.cFileName;

			if ( boost::filesystem::last_write_time( file_path ) < boost::filesystem::last_write_time( exe_file_path ) )
			{
				if ( ! boost::filesystem::remove( file_path ) )
				{
					COMMON_THROW_EXCEPTION_MESSAGE( string_t( "file remove failed : " ) + file_path );
				}

				common::log( "log/debug.log", string_t( "removed .bin.fbx : " + file_path ) );
			}
			else
			{
				common::log( "log/debug.log", string_t( "NOT removed .bin.fbx : " + file_path ) );
			}

			if ( ! FindNextFile( find_handle, & find_data ) )
			{
				break;
			}
		}

		FindClose( find_handle );
	}
}

//¡¡¡@ƒƒCƒ“@¡¡¡
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow )
{
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	convert_fbx();

	return -1;
}
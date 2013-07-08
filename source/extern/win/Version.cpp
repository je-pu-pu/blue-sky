//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Version.cpp
//! \date		2012/07/15
//! \author		JE
//! \brief		Version の実装
//**********************************************************

#include "Version.h"

#include <common/log.h>
#include <common/serialize.h>

#include <windows.h>

namespace win
{

//! バージョン情報をログに記録する
bool Version::log( const char* file_path ) const
{
	OSVERSIONINFO version_info = { sizeof( OSVERSIONINFO ) };

	if ( ! GetVersionEx( & version_info ) )
	{
		return false;
	}

	std::string output = "windows version:\n";

	output += COMMON_FIELD_LOG( version_info, dwMajorVersion );
	output += COMMON_FIELD_LOG( version_info, dwMinorVersion );
	output += COMMON_FIELD_LOG( version_info, dwBuildNumber );
	output += COMMON_FIELD_LOG( version_info, dwPlatformId );
	output += COMMON_FIELD_LOG( version_info, szCSDVersion );

	common::log( file_path, output );
	
	return true;
}

} // namespace win



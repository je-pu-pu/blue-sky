#pragma once

#include <windows.h>
#include <string>

namespace win
{

/**
 * @brief GetLastError() をラップし、エラーメッセージを取得できるようにしたクラス
 * 
 */
class LastError
{
private:
	DWORD last_error_;
	std::string error_message_;

public:
	LastError()
		: last_error_( GetLastError() )
	{
		LPVOID output = 0;
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, last_error_, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )( & output ), 0, 0 );
		error_message_ = static_cast< LPCTSTR >( output );

		LocalFree( output );
	}

	const std::string& get_error_message()
	{
		return error_message_;
	}
};

}
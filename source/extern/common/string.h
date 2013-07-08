// *********************************************************
// string.h
// copyright (c) JE all rights reserved
// 2012/08/03
// *********************************************************

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <string>

namespace common
{

std::wstring convert_to_wstring( const std::string s )
{
	wchar_t* ws = new wchar_t[ s.length() + 1 ];

	mbstowcs( ws, s.c_str(), s.length() + 1 );

	std::wstring result = ws;

	delete [] ws;
	
	return result;
}

std::string convert_to_string( const std::wstring ws )
{
	char* s = new char[ ws.length() * MB_CUR_MAX + 1 ];

	wcstombs( s, ws.c_str(), ws.length() * MB_CUR_MAX + 1 );

	std::string result = s;

	delete [] s;
	
	return result;
}

} // namespace math

#endif // COMMON_MATH_H

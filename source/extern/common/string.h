// *********************************************************
// string.h
// copyright (c) JE all rights reserved
// 2012/08/03
// *********************************************************

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <string>
#include <windows.h>

namespace common
{

inline std::wstring convert_to_wstring( const std::string& s )
{
	if ( s.empty() ) return {};
	int len = MultiByteToWideChar( CP_UTF8, 0, s.c_str(), -1, nullptr, 0 );
	if ( len <= 0 ) return {};
	std::wstring ws( len - 1, L'\0' );
	MultiByteToWideChar( CP_UTF8, 0, s.c_str(), -1, &ws[0], len );
	return ws;
}

inline std::string convert_to_string( const std::wstring& ws )
{
	if ( ws.empty() ) return {};
	int len = WideCharToMultiByte( CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr );
	if ( len <= 0 ) return {};
	std::string s( len - 1, '\0' );
	WideCharToMultiByte( CP_UTF8, 0, ws.c_str(), -1, &s[0], len, nullptr, nullptr );
	return s;
}

} // namespace common

#endif // COMMON_STRING_H

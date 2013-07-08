#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include "time_stamp.h"

#include <string>
#include <fstream>

namespace common
{

inline void log( const char* file_name, const std::string& text, bool append = true )
{
	std::ofstream out( file_name, append ? std::ios::out | std::ios::app : std::ios::out );

	if ( ! out.is_open() )
	{
		return;
	}

	out << "[" << time_stamp::str() << "] ";
	out << text;

	if ( ! text.empty() && text[ text.size() - 1 ] != '\n' )
	{
		out << '\n';
	}
}

inline void log( const char* file_name, const std::wstring& text, bool append = true )
{
	std::wofstream out( file_name, append ? std::ios::out | std::ios::app : std::ios::out );

	if ( ! out.is_open() )
	{
		return;
	}

	out << L"[" << time_stamp::w_str() << L"] ";
	out << text;
}

} // namespace common

#define COMMON_FIELD_LOG( prefix, name ) std::string( "\t" ) + # name + " : " + common::serialize( prefix.name ) + "\n"
#define COMMON_FIELD_LOG_W( prefix, name ) std::wstring( L"\t" ) + L# name + L" : " + common::w_serialize( prefix.name ) + L"\n"

#endif // COMMON_LOG_H

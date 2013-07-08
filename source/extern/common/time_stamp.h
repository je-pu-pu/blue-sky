#ifndef COMMON_TIME_STAMP_H
#define COMMON_TIME_STAMP_H

#include <ctime>
#include <sstream>
#include <iomanip>

namespace common
{

class time_stamp
{
public:
	static std::string str()
	{
		time_t t;
		tm* t_st;

		time( & t );
		t_st = localtime( & t );

		std::stringstream ss;

		ss << std::setfill( '0' );
		ss << ( 1900 + t_st->tm_year ) << "-" << std::setw( 2 ) << ( 1 + t_st->tm_mon ) << "-" << std::setw( 2 ) << t_st->tm_mday;
		ss << " " << std::setw( 2 ) << t_st->tm_hour << ":" << std::setw( 2 ) << t_st->tm_min << ":" << std::setw( 2 ) << t_st->tm_sec;

		return ss.str();
	}

	static std::wstring w_str()
	{
		time_t t;
		tm* t_st;

		time( & t );
		t_st = localtime( & t );

		std::wstringstream ss;

		ss << std::setfill( L'0' );
		ss << ( 1900 + t_st->tm_year ) << L"-" << std::setw( 2 ) << ( 1 + t_st->tm_mon ) << L"-" << std::setw( 2 ) << t_st->tm_mday;
		ss << L" " << std::setw( 2 ) << t_st->tm_hour << L":" <<  std::setw( 2 ) << t_st->tm_min << L":" << std::setw( 2 ) << t_st->tm_sec;

		return ss.str();
	}

}; // class time_stamp

} // namespace common

#endif // COMMON_TIME_STAMP_H

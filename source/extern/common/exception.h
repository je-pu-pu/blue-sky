// *********************************************************
// exception.h
// copyright (c) JE all rights reserved
// 2010/05/31
// *********************************************************

#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <string>

namespace common
{

template< typename data_type >
class exception
{
private:
	const char* file_;
	unsigned int line_;
	data_type data_;

public:
	exception( const char* file, unsigned int line, const data_type& data ) : file_( file ), line_( line ), data_( data ) { }

	const char* file() const { return file_; }
	unsigned int line() const { return line_; }
	const data_type& data() const { return data_; }
};

} // namespace common

#define COMMON_THROW_EXCEPTION throw common::exception< std::string >( __FILE__, __LINE__, "" );
#define COMMON_THROW_EXCEPTION_MESSAGE( m ) throw common::exception< std::string >( __FILE__, __LINE__, ( m ) );
#define COMMON_THROW_EXCEPTION_SERIALIZE( x ) throw common::exception< std::string >( __FILE__, __LINE__, ( common::serialize( x ) ) );

#endif // COMMON_EXCEPTION_H

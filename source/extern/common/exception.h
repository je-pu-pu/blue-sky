// *********************************************************
// exception.h
// copyright (c) JE all rights reserved
// 2010/05/31
// *********************************************************

#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <exception>
#include <string>
#include <type_traits>

namespace common
{

/**
 * エンジンの例外の共通の基底
 *
 * 以前は例外の型に共通の基底が無く（common::exception< std::string >・common::exception< HRESULT >・std::exception など）、
 * 受け取る側が型ごとに catch を並べる必要があった。これを受け取れば、エンジンの例外はすべて受け取れる。
 * what() は「ファイル:行 内容」の文字列（UTF-8）。
 */
class exception_base : public std::exception
{
private:
	std::string message_;

protected:
	explicit exception_base( std::string message ) : message_( std::move( message ) ) { }

public:
	const char* what() const noexcept override { return message_.c_str(); }
};

template< typename data_type >
class exception : public exception_base
{
private:
	const char* file_;
	int line_;
	data_type data_;

	static std::string describe( const char* file, int line, const data_type& data )
	{
		std::string text = std::string( file ? file : "" ) + ":" + std::to_string( line ) + " ";

		if constexpr ( std::is_same_v< data_type, std::string > )
		{
			text += data;
		}
		else if constexpr ( std::is_arithmetic_v< data_type > )
		{
			text += std::to_string( data );
		}

		return text;
	}

public:
	exception( const char* file, int line, const data_type& data )
		: exception_base( describe( file, line, data ) ), file_( file ), line_( line ), data_( data ) { }

	const char* file() const { return file_; }
	int line() const { return line_; }
	const data_type& data() const { return data_; }
};

} // namespace common

#define COMMON_THROW_EXCEPTION throw common::exception< std::string >( __FILE__, __LINE__, "" );
#define COMMON_THROW_EXCEPTION_MESSAGE( m ) throw common::exception< std::string >( __FILE__, __LINE__, ( m ) );
#define COMMON_THROW_EXCEPTION_SERIALIZE( x ) throw common::exception< std::string >( __FILE__, __LINE__, ( common::serialize( x ) ) );

#endif // COMMON_EXCEPTION_H

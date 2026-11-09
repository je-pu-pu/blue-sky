#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include <windows.h>

namespace blue_sky
{

/// ログファイルの場所（作業ディレクトリからの相対）
inline constexpr const char* DEBUG_LOG_PATH = "log/blue-sky.log";

/**
 * ログを 1 行出す（UTF-8）
 *
 * - デバッガの出力ウィンドウ：ソースは /utf-8 でコンパイルしているので日本語のリテラルは UTF-8 になる。
 *   OutputDebugStringA は ANSI（CP932）として解釈して化けるので、UTF-16 に変換して OutputDebugStringW で出す。
 * - ファイル（log/blue-sky.log）：デバッガを付けていないとき（配布版・自動テスト）にも原因を追えるように、時刻付きで追記する。
 */
inline void debug_log( const std::string& utf8 )
{
	if ( utf8.empty() )
	{
		return;
	}

	const int n = MultiByteToWideChar( CP_UTF8, 0, utf8.data(), static_cast< int >( utf8.size() ), nullptr, 0 );

	if ( n > 0 )
	{
		std::wstring w( static_cast< size_t >( n ), L'\0' );
		MultiByteToWideChar( CP_UTF8, 0, utf8.data(), static_cast< int >( utf8.size() ), w.data(), n );
		w += L'\n';

		OutputDebugStringW( w.c_str() );
	}

	std::error_code ec;
	std::filesystem::create_directories( std::filesystem::path( DEBUG_LOG_PATH ).parent_path(), ec );

	std::ofstream out( DEBUG_LOG_PATH, std::ios::binary | std::ios::app );

	if ( out )
	{
		const std::time_t now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
		std::tm local {};
		localtime_s( & local, & now );

		char stamp[ 32 ];
		std::strftime( stamp, sizeof( stamp ), "%Y-%m-%d %H:%M:%S", & local );

		out << stamp << ' ' << utf8 << '\n';
	}
}

/**
 * std::cout に書いたものを、1 行ずつ debug_log にも流す（元の出力先＝開発用のコンソールにもそのまま出す）
 *
 * 以前はログの出口が 4 系統あり（std::cout・debug_log・common::log・OutputDebugString）、
 * std::cout（モデルの読み込みなど 100 か所以上）はコンソールを開いていないと何も残らなかった。
 * 出口を debug_log（log/blue-sky.log とデバッガ）にそろえる。common::log はエラーなど特定のファイルに残す用途だけに使う。
 */
class DebugLogStreamBuf : public std::streambuf
{
private:
	std::streambuf* original_;
	std::string line_;

protected:
	int overflow( int c ) override
	{
		if ( c == traits_type::eof() )
		{
			return traits_type::not_eof( c );
		}

		if ( original_ )
		{
			original_->sputc( static_cast< char >( c ) );
		}

		if ( c == '\n' )
		{
			debug_log( line_ );
			line_.clear();
		}
		else if ( c != '\r' )
		{
			line_ += static_cast< char >( c );
		}

		return c;
	}

	int sync() override
	{
		return original_ ? original_->pubsync() : 0;
	}

public:
	explicit DebugLogStreamBuf( std::streambuf* original ) : original_( original ) { }

	/// 改行で終わっていない残りを出す
	void flush_line()
	{
		if ( ! line_.empty() )
		{
			debug_log( line_ );
			line_.clear();
		}
	}
};

/// 生きている間、std::cout を debug_log にも流す
class ScopedCoutToLog
{
private:
	std::streambuf* original_;
	DebugLogStreamBuf buffer_;

public:
	ScopedCoutToLog() : original_( std::cout.rdbuf() ), buffer_( original_ ) { std::cout.rdbuf( & buffer_ ); }
	~ScopedCoutToLog() { buffer_.flush_line(); std::cout.rdbuf( original_ ); }

	ScopedCoutToLog( const ScopedCoutToLog& ) = delete;
	ScopedCoutToLog& operator = ( const ScopedCoutToLog& ) = delete;
};

} // namespace blue_sky

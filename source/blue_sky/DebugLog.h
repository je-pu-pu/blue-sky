#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
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

} // namespace blue_sky

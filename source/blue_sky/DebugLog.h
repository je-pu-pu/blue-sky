#pragma once

#include <string>

#include <windows.h>

namespace blue_sky
{

/**
 * デバッガの出力ウィンドウに UTF-8 の文字列を出す
 *
 * ソースは /utf-8 でコンパイルしているので、日本語のリテラルは UTF-8 になる。
 * OutputDebugStringA は ANSI（CP932）として解釈するため日本語が化ける。UTF-16 に変換して OutputDebugStringW で出す。
 */
inline void debug_log( const std::string& utf8 )
{
	if ( utf8.empty() )
	{
		return;
	}

	const int n = MultiByteToWideChar( CP_UTF8, 0, utf8.data(), static_cast< int >( utf8.size() ), nullptr, 0 );

	if ( n <= 0 )
	{
		return;
	}

	std::wstring w( static_cast< size_t >( n ), L'\0' );
	MultiByteToWideChar( CP_UTF8, 0, utf8.data(), static_cast< int >( utf8.size() ), w.data(), n );
	w += L'\n';

	OutputDebugStringW( w.c_str() );
}

} // namespace blue_sky

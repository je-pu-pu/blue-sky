#include "Config.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace game
{

Config::Config()
{

}

Config::~Config()
{

}

bool Config::load_file( const char* file_name )
{
	std::ifstream in( file_name );

	if ( ! in )
	{
		return false;
	}

	std::string line;

	while ( std::getline( in, line ) )
	{
		read_line( line );
	}

	return true;
}

/**
 * 1 行を読む（"名前 値..."）
 *
 * - 値は空白で区切られた語を 1 つの空白でつないだもの。値が無い行は空文字列として読む
 *   （以前は空の値で例外になり、行末の空白をエディタが消しただけで起動できなかった）
 * - 空行と、# で始まる行は読み飛ばす
 */
void Config::read_line( const std::string& line )
{
	std::istringstream ss( line );

	std::string name;

	if ( ! ( ss >> name ) || name[ 0 ] == '#' )
	{
		return;
	}

	std::string value;
	std::string word;

	while ( ss >> word )
	{
		if ( ! value.empty() )
		{
			value += ' ';
		}

		value += word;
	}

	value_[ name ] = value;
}

/**
 * ファイルに書く
 *
 * 一時ファイルに書いてから置き換えるので、書き込みの途中で落ちても元のファイルは壊れない。
 *
 * @return 書き込みと置き換えに成功したら true
 */
bool Config::save_file( const char* file_name )
{
	const std::filesystem::path path( file_name );
	std::filesystem::path temp = path;
	temp += ".tmp";

	{
		std::ofstream out( temp, std::ios::binary | std::ios::trunc );

		if ( ! out )
		{
			return false;
		}

		for ( const auto& v : value_ )
		{
			out << v.first << " " << v.second << '\n';
		}

		out.flush();

		if ( ! out )
		{
			return false;
		}
	}

	std::error_code ec;
	std::filesystem::rename( temp, path, ec );		// 既存のファイルは置き換えられる

	if ( ec )
	{
		std::filesystem::remove( temp, ec );
		return false;
	}

	return true;
}

} // namespace game
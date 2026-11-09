#include "StringTable.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace core::ui
{

namespace
{

/// 先頭・末尾の空白（スペース・タブ・CR）を取り除く
string_t trim( const string_t& s )
{
	const char_t* ws = " \t\r";
	const auto begin = s.find_first_not_of( ws );

	if ( begin == string_t::npos )
	{
		return string_t();
	}

	const auto end = s.find_last_not_of( ws );
	return s.substr( begin, end - begin + 1 );
}

/// 値の中のエスケープ `\n` を改行に変換する
string_t unescape( const string_t& s )
{
	string_t result;
	result.reserve( s.size() );

	for ( size_t i = 0; i < s.size(); i++ )
	{
		if ( s[ i ] == '\\' && i + 1 < s.size() && s[ i + 1 ] == 'n' )
		{
			result.push_back( '\n' );
			i++;
		}
		else
		{
			result.push_back( s[ i ] );
		}
	}

	return result;
}

} // namespace

int StringTable::load_from_string( const string_t& text )
{
	string_t body = text;

	// UTF-8 BOM を読み飛ばす
	if ( body.size() >= 3 &&
		static_cast< unsigned char >( body[ 0 ] ) == 0xEF &&
		static_cast< unsigned char >( body[ 1 ] ) == 0xBB &&
		static_cast< unsigned char >( body[ 2 ] ) == 0xBF )
	{
		body.erase( 0, 3 );
	}

	std::istringstream ss( body );
	string_t line;
	int count = 0;

	while ( std::getline( ss, line ) )
	{
		const string_t trimmed = trim( line );

		if ( trimmed.empty() || trimmed[ 0 ] == '#' )
		{
			continue;
		}

		const auto eq = trimmed.find( '=' );

		if ( eq == string_t::npos )
		{
			continue;
		}

		const string_t key = trim( trimmed.substr( 0, eq ) );

		if ( key.empty() )
		{
			continue;
		}

		table_[ key ] = unescape( trim( trimmed.substr( eq + 1 ) ) );
		count++;
	}

	return count;
}

bool StringTable::load_file( const char_t* path )
{
	std::ifstream in( path, std::ios::binary );

	if ( ! in )
	{
		return false;
	}

	std::stringstream ss;
	ss << in.rdbuf();

	load_from_string( ss.str() );

	return true;
}

string_t StringTable::get( const string_t& key ) const
{
	auto i = table_.find( key );

	if ( i == table_.end() )
	{
		if ( ! key.empty() )
		{
			missing_.insert( key );
		}

		return key;
	}

	return i->second;
}

string_t StringTable::format( const string_t& key, std::initializer_list< string_t > args ) const
{
	return format_text( get( key ), args );
}

string_t StringTable::format_text( const string_t& text, std::initializer_list< string_t > args )
{
	const std::vector< string_t > list( args );
	string_t result;
	result.reserve( text.size() );

	for ( size_t i = 0; i < text.size(); i++ )
	{
		if ( text[ i ] == '{' )
		{
			const auto close = text.find( '}', i + 1 );

			if ( close != string_t::npos && close > i + 1 )
			{
				const string_t index_text = text.substr( i + 1, close - i - 1 );
				bool is_number = true;

				for ( char_t c : index_text )
				{
					if ( c < '0' || c > '9' )
					{
						is_number = false;
						break;
					}
				}

				if ( is_number )
				{
					const size_t index = static_cast< size_t >( std::stoul( index_text ) );

					if ( index < list.size() )
					{
						result += list[ index ];
						i = close;
						continue;
					}
				}
			}
		}

		result.push_back( text[ i ] );
	}

	return result;
}

} // namespace core::ui

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <common/serialize.h>
#include <map>

namespace game
{

class Config
{
public:
	typedef std::map< std::string, std::string > ValueMap;

private:
	ValueMap value_;

public:
	Config();
	virtual ~Config();

	template< typename T > T get( const char* name, T default ) const
	{
		ValueMap::const_iterator i = value_.find( name );

		if ( i == value_.end() )
		{
			return default;
		}

		try
		{
			return common::deserialize< T >( i->second );
		}
		catch ( boost::bad_lexical_cast )
		{
			return default;
		}
	}

	template< typename T > T get( const char* name, T default )
	{
		ValueMap::const_iterator i = value_.find( name );

		if ( i == value_.end() )
		{
			set( name, default );
			return default;
		}

		try
		{
			return common::deserialize< T >( i->second );
		}
		catch ( boost::bad_lexical_cast )
		{
			set( name, default );
			return default;
		}
	}

	template< typename T > void set( const char* name, T value ) { value_[ name ] = common::serialize( value ); }

	bool save_file( const char* );
	bool load_file( const char* );

}; // class Config

} // game

#endif // GAME_CONFIG_H
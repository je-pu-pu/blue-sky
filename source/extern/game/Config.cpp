#include "Config.h"
#include <common/exception.h>
#include <fstream>

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
	
	while ( in.good() )
	{		
		std::string line;		
		std::getline( in, line );

		read_line( line );
	}

	return true;
}

void Config::read_line( const string_t& line )
{
	std::stringstream ss;
		
	std::string name;
	std::string value;
		
	ss << line;
	ss >> name;

	if ( name == "" )
	{
		return;
	}

	while ( ss.good() )
	{
		std::string v;

		ss >> v;

		value += v;
		value += " ";
	}

	value.erase( value.size() - 1 );

	value_[ name ] = value;

	return;
}

bool Config::save_file( const char* file_name )
{
	std::ofstream out( file_name );
	
	for ( ValueMap::const_iterator i = value_.begin(); i != value_.end(); i++ )
	{
		out <<  i->first << " " << i->second << std::endl;
	}

	return true;
}

} // namespace game
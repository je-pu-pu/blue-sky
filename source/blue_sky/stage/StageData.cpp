#include "StageData.h"

#include <cstdio>

namespace blue_sky::stage
{

string_t SourceLocation::to_string() const
{
	string_t s = file;

	if ( line > 0 )
	{
		s += ":" + std::to_string( line );

		if ( column > 0 )
		{
			s += ":" + std::to_string( column );
		}
	}

	return s;
}

namespace
{

/// 数を、読み直すと同じ値になる桁で書く
string_t number( double v )
{
	char buffer[ 64 ];
	std::snprintf( buffer, sizeof( buffer ), "%.17g", v );
	return buffer;
}

string_t numbers( const Vector3d& v )
{
	return "[" + number( v[ 0 ] ) + ", " + number( v[ 1 ] ) + ", " + number( v[ 2 ] ) + "]";
}

string_t describe_action( const Action& a )
{
	return a.name + " (" + a.params + ")";
}

} // namespace

string_t describe( const StageData& data )
{
	string_t s;

	s += "collision = " + data.collision + "\n";
	s += "ground = " + data.ground + "\n";
	s += "bgm = " + data.bgm + "\n";
	s += "bgm_bpm = " + ( data.bgm_bpm ? number( * data.bgm_bpm ) : string_t( "-" ) ) + "\n";
	s += "bgm_loop = " + ( data.bgm_loop ? string_t( * data.bgm_loop ? "true" : "false" ) : string_t( "-" ) ) + "\n";

	if ( data.player )
	{
		s += "player = " + numbers( data.player->position ) + " camera_direction = " + ( data.player->camera_direction ? number( * data.player->camera_direction ) : string_t( "-" ) ) + "\n";
	}
	else
	{
		s += "player = -\n";
	}

	s += "goal = " + ( data.goal ? numbers( * data.goal ) : string_t( "-" ) ) + "\n";

	for ( size_t i = 0; i < data.objects.size(); ++i )
	{
		const ObjectDesc& o = data.objects[ i ];

		s += "object " + std::to_string( i ) + " : " + o.type;

		if ( ! o.model.empty() )
		{
			s += " model = " + o.model;
		}

		s += " position = " + numbers( o.position );
		s += " rotation = " + numbers( o.rotation );
		s += " direction = " + number( o.direction );
		s += " size = " + numbers( o.size );
		s += " speed = " + number( o.speed );

		for ( const auto& name : o.names )
		{
			s += " name = " + name;
		}

		s += "\n";

		for ( const auto& e : o.events )
		{
			s += "  on " + e.event + " : " + describe_action( e.action ) + "\n";
		}
	}

	for ( const auto& a : data.start_actions )
	{
		s += "start : " + describe_action( a ) + "\n";
	}

	return s;
}

} // namespace blue_sky::stage

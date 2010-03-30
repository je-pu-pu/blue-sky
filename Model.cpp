#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

namespace art
{

/**
 * ƒtƒ@ƒCƒ‹‚ğ“Ç‚İ‚Ş
 * 
 */
void Model::load_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	std::map<Index,ID> id_map;
	int vertex_index = 1;

	while ( in.good() )
	{		
		std::string line;		
		std::getline( in, line );

		std::stringstream ss;
		std::string command;
		
		ss << line;
		ss >> command;
		
		if ( command == "v" )
		{
			// vertex
			float x = 0.f;
			float y = 0.f;
			float z = 0.f;
			
			ss >> x >> y >> z;
			
			std::swap( y, z );
			y = -y;
			
			LiveVertex v( x, y, z );

			v.vertex() += Vertex( 0.f, 5.f, 5.f );
			v.vertex() *= 0.4f;

			vertex_list()[ v.id() ] = v;
			id_map[ vertex_index ] = v.id();

			vertex_index++;
		}
		else if ( command == "f" )
		{
			int start_vertex_index, end_vertex_index;
			unsigned int start_color, end_color;

			ss >> start_vertex_index >> end_vertex_index >> start_color >> end_color;

			int start_vertex_id = id_map[ start_vertex_index ];
			int end_vertex_id = id_map[ end_vertex_index ];

			line_list().push_back( Line( start_vertex_id, end_vertex_id, start_color, end_color ) );
		}
	}
}

}; // namespace art
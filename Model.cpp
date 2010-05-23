#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

namespace art
{

/**
 * ÉtÉ@ÉCÉãÇì«Ç›çûÇﬁ
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

			vertex_list()[ v.id() ] = v;
			id_map[ vertex_index ] = v.id();

			vertex_index++;
		}
		else if ( command == "f" )
		{
			Face face;

			while ( ss.good() )
			{
				int index;
				ss >> index;

				face.id_list().push_back( id_map[ index ] );
			}

			if ( face.id_list().size() == 2 )
			{
				COLORREF start_color = RGB( 255, 255, 255 ), end_color = RGB( 255, 255, 255 );
				line_list().push_back( Line( face.id_list()[ 0 ], face.id_list()[ 1 ], start_color, end_color ) );
			}
			else
			{
				static int n = 0;
				Color colors[] = { Color( 164, 188, 151 ), Color( 50, 70, 40 ), Color( 0xCC, 0xFF, 0xCC ) };

				face.color() = colors[ n ];
				face_list().push_back( face );

				n = ( n + 1 ) % 3;
			}
		}
	}
}

}; // namespace art
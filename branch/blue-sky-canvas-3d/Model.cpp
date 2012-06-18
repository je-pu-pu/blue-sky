#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

namespace art
{

void Model::clear()
{
	vertex_list().clear();
	line_list().clear();
	face_list().clear();
}

/**
 * ファイルを読み込む
 * 
 */
void Model::load_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	std::map<Index,ID> id_map;
	int vertex_index = 1;

	// Color color( 164, 188, 151 ); // Color( 50, 70, 40 ), Color( 0xCC, 0xFF, 0xCC ) };
	
	// 面の色
	Color color( 0x66, 0xCC, 0xFF );

	// 線の色
	const Color line_start_color( 255, 255, 255 );
	const Color line_end_color( 255, 255, 255 );

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
				line_list().push_back( Line( face.id_list()[ 0 ], face.id_list()[ 1 ], line_start_color, line_end_color ) );
			}
			else
			{
				face.color() = color;
				face_list().push_back( face );
			}
		}
		else if ( command == "c" )
		{
			if ( ss.good() )
			{
				int r, g, b;
				ss >> r >> g >> b;

				color = Color( r, g, b );
			}
		}
	}
}

}; // namespace art
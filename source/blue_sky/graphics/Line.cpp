#include "Line.h"

#include <fstream>
#include <sstream>

namespace blue_sky::graphics
{

Line::Line()
	: color_( 0.f, 0.f, 0.f, 0.f )
	, is_cast_shadow_( false )
{

}

Line::~Line()
{
	
}

bool Line::load_obj( const char* file_name )
{
	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	PositionList position_list;
	ColorList color_list;

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
			Vertex v;

			v.Color = Color( 0.f, 0.f, 0.f, 0.f );

			ss >> v.Position.x() >> v.Position.y() >> v.Position.z();
			ss >> v.Color.r() >> v.Color.g() >> v.Color.b() >> v.Color.a();

			v.Position.z() = -v.Position.z();
			
			vertex_list_.push_back( v );
		}
		else if ( command == "f" )
		{
			while ( ss.good() )
			{
				IndexList::value_type index;

				ss >> index;

				// 1 origin -> 0 origin
				index--;

				index_list_.push_back( index );
			}
		}
	}

	create_vertex_buffer();
	create_index_buffer();

	create_texture();

	index_size_ = index_list_.size();

	vertex_list_.clear();
	index_list_.clear();

	return true;
}

} // namespace blue_sky::graphics

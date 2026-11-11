#include "Line.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace blue_sky::graphics
{

Line::Line()
	: color_( 0.f, 0.f, 0.f, 0.f )
	, is_cast_shadow_( false )
	, index_size_( 0 )
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

	// 頂点を囲む箱と、いちばん長い線分（2 つずつ組の線分。番号が範囲外の組は数えない）を覚えてから頂点を消す（カリングに使う）
	bounds_ = LocalBounds();
	max_segment_length_ = 0.f;

	for ( const auto& v : vertex_list_ )
	{
		bounds_.add( v.Position );
	}

	for ( size_t n = 0; n + 1 < index_list_.size(); n += 2 )
	{
		if ( index_list_[ n ] < vertex_list_.size() && index_list_[ n + 1 ] < vertex_list_.size() )
		{
			max_segment_length_ = std::max( max_segment_length_, ( vertex_list_[ index_list_[ n + 1 ] ].Position - vertex_list_[ index_list_[ n ] ].Position ).length() );
		}
	}

	vertex_list_.clear();
	index_list_.clear();

	return true;
}

} // namespace blue_sky::graphics

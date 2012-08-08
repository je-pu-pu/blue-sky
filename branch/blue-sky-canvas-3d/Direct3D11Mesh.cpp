#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string_regex.hpp>

#include <map>

#include <fstream>
#include <sstream>

Direct3D11Mesh::Direct3D11Mesh( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
{

}

Direct3D11Mesh::~Direct3D11Mesh()
{
	for ( MaterialList::iterator i = material_list_.begin(); i != material_list_.end(); ++i )
	{
		delete *i;
	}

	DIRECT_X_RELEASE( vertex_buffer_ );

}

bool Direct3D11Mesh::load_obj( const char* file_name )
{
	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	typedef std::map< Vertex, Material::Index > VertexIndexMap;
	VertexIndexMap vertex_index_map;

	PositionList position_list;
	TexCoordList tex_coord_list;

	material_list_.push_back( new Material( direct_3d_ ) );
	Material* material = material_list_[ material_list_.size() - 1 ];

	std::string texture_file_name;

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
			Position v;

			ss >> v.x >> v.y >> v.z;

			v.z = -v.z;

			position_list.push_back( v );
		}
		else if ( command == "vt" )
		{
			TexCoord uv;

			ss >> uv.x >> uv.y;

			tex_coord_list.push_back( uv );
		}
		else if ( command == "f" )
		{
			std::list< std::string > f_list;

			while ( ss.good() )
			{
				std::string f;

				ss >> f;

				f_list.push_back( f );
			}

			if ( f_list.size() >= 3 )
			{
				f_list.reverse();
			}
			

			for ( auto fi = f_list.begin(); fi != f_list.end(); ++fi )
			{
				std::string& f = *fi;

				std::replace( f.begin(), f.end(), '/', ' ' );
				
				std::stringstream fss;
				fss << f;

				PositionList::size_type position_index;
				TexCoordList::size_type tex_coord_index;

				fss >> position_index >> tex_coord_index;

				// 1 origin -> 0 origin
				position_index--;
				tex_coord_index--;

				Vertex v;
				
				v.Position = position_list[ position_index ];
				v.TexCoord = tex_coord_list[ tex_coord_index ];
				v.TexCoord.y = 1.f - v.TexCoord.y;

				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					material->get_index_list().push_back( i->second );
				}
				else
				{
					Material::Index vertex_index = vertex_list_.size();

					vertex_list_.push_back( v );
					material->get_index_list().push_back( vertex_index );

					vertex_index_map[ v ] = vertex_index;
				}
			}
		}
		else if ( command == "texture" )
		{
			ss >> texture_file_name;

			material->load_texture( get_texture_file_name_by_texture_name( texture_file_name.c_str() ).c_str() );
		}
		else if ( command == "usemtl" )
		{
			if ( ! material->get_index_list().empty() )
			{
				material->create_index_buffer();

				material_list_.push_back( new Material( direct_3d_ ) );
				material = material_list_[ material_list_.size() - 1 ];

				vertex_index_map.clear();
			}

			
			ss >> texture_file_name;

			boost::algorithm::replace_all_regex( texture_file_name, boost::regex( "(.*)_(.*)" ), std::string( "$2" ) );

			if ( ! texture_file_name.empty() )
			{
				material->load_texture( get_texture_file_name_by_texture_name( texture_file_name.c_str() ).c_str() );
			}
		}
	}

	create_vertex_buffer();
	create_index_buffer( material );

	// !!!
	if ( texture_file_name.empty() )
	{
		boost::filesystem::path path( file_name );
		texture_file_name = std::string( "media/model/" ) + path.stem().string() + ".png";

		try
		{
			material->load_texture( get_texture_file_name_by_texture_name( texture_file_name.c_str() ).c_str() );
		}
		catch ( ... )
		{

		}
	}

	return true;
}

void Direct3D11Mesh::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & vertex_list_[ 0 ];
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void Direct3D11Mesh::create_index_buffer( Material* material )
{
	if ( ! material->get_index_list().empty() )
	{
		material->create_index_buffer();
	}
}

string_t Direct3D11Mesh::get_texture_file_name_by_texture_name( const char* texture_name ) const
{
	/// !!!
	if ( ! boost::filesystem::path( texture_name ).has_parent_path() )
	{
		return string_t( "media/model/" ) + texture_name;
	}

	return texture_name;
}

void Direct3D11Mesh::render() const
{
	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for ( MaterialList::const_iterator i = material_list_.begin(); i != material_list_.end(); ++i )
	{
		( *i )->render();
	}
}
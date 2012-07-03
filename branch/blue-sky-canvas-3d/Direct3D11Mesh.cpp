#include "Direct3D11Mesh.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

// #include <boost/filesystem.hpp>

#include <map>

#include <fstream>
#include <sstream>

Direct3D11Mesh::Direct3D11Mesh( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
	, index_buffer_( 0 )

	, texture_resource_view_( 0 )
{

}

Direct3D11Mesh::~Direct3D11Mesh()
{
	DIRECT_X_RELEASE( texture_resource_view_ );

	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

bool Direct3D11Mesh::load_obj( const char* file_name )
{
	std::ifstream in( file_name );

	typedef std::map< Vertex, Index > VertexIndexMap;
	VertexIndexMap vertex_index_map;

	PositionList position_list;
	TexCoordList tex_coord_list;

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

			position_list.push_back( v );
		}
		else if ( command == "uv" )
		{
			TexCoord uv;

			ss >> uv.x >> uv.y;

			tex_coord_list.push_back( uv );
		}
		else if ( command == "f" )
		{
			while ( ss.good() )
			{
				std::string f;

				ss >> f;

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

				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					index_list_.push_back( i->second );
				}
				else
				{
					Index vertex_index = vertex_index_map.size();

					vertex_list_.push_back( v );
					index_list_.push_back( vertex_index );


					vertex_index_map[ v ] = vertex_index;
				}
			}
		}
	}

	create_vertex_buffer();
	create_index_buffer();

	create_texture_resource_view( file_name );

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

void Direct3D11Mesh::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof( Index ) * index_list_.size();

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & index_list_[ 0 ];

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Direct3D11Mesh::create_texture_resource_view( const char* file_name )
{
	// std::string texture_file_name = "media/model/robot.png"; // boost::filesystem::basename( boost::filesystem::path( file_name ) ) + ".png";
	std::string texture_file_name = "media/texture/lines.png";
	// std::string texture_file_name = "media/texture/pencil-face-3.png";
	
	DIRECT_X_FAIL_CHECK( D3DX11CreateShaderResourceViewFromFile( direct_3d_->getDevice(), texture_file_name.c_str(), 0, 0, & texture_resource_view_, 0 ) );
}

void Direct3D11Mesh::render() const
{
	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );

	// direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & texture_resource_view_ );

	// debug !!!
	ID3D11ShaderResourceView* text_view = direct_3d_->getTextView();
	direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & text_view );

	direct_3d_->getImmediateContext()->DrawIndexed( index_list_.size(), 0, 0 );
}
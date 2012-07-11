#include "DrawingModel.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

// #include <boost/filesystem.hpp>

#include <map>

#include <fstream>
#include <sstream>

DrawingModel::DrawingModel( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
	, index_buffer_( 0 )

	, texture_resource_view_( 0 )
{

}

DrawingModel::~DrawingModel()
{
	DIRECT_X_RELEASE( texture_resource_view_ );

	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

bool DrawingModel::load_obj( const char* file_name )
{
	std::ifstream in( file_name );

	typedef std::map< Vertex, Index > VertexIndexMap;
	VertexIndexMap vertex_index_map;

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

			ss >> v.Position.x >> v.Position.y >> v.Position.z;
			ss >> v.Color.x >> v.Color.y >> v.Color.z >> v.Color.w;

			vertex_list_.push_back( v );
		}
		else if ( command == "f" )
		{
			while ( ss.good() )
			{
				PositionList::size_type index;

				ss >> index;

				// 1 origin -> 0 origin
				index--;

				index_list_.push_back( index );
			}
		}
	}

	create_vertex_buffer();
	create_index_buffer();

	create_texture_resource_view( file_name );

	return true;
}

void DrawingModel::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & vertex_list_[ 0 ];
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void DrawingModel::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof( Index ) * index_list_.size();

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & index_list_[ 0 ];

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void DrawingModel::create_texture_resource_view( const char* file_name )
{
	// @todo Manager
	std::string texture_file_name = "media/texture/lines.png";
	
	DIRECT_X_FAIL_CHECK( D3DX11CreateShaderResourceViewFromFile( direct_3d_->getDevice(), texture_file_name.c_str(), 0, 0, & texture_resource_view_, 0 ) );
}

void DrawingModel::render( int level ) const
{
	if ( level < 1 )
	{
		return;
	}

	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );

	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & texture_resource_view_ );

	// debug !!!
	// ID3D11ShaderResourceView* text_view = direct_3d_->getTextView();
	// direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & text_view );

	direct_3d_->getImmediateContext()->DrawIndexed( std::min< int >( level, index_list_.size() ), 0, 0 );
}
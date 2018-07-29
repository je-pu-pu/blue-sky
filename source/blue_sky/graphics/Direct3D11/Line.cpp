#include "Line.h"

#include <GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

#include <common/exception.h>
#include <common/math.h>

#include <fstream>
#include <sstream>

#include <map>

namespace blue_sky::graphics::direct_3d_11
{

Line::Line( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
	, index_buffer_( 0 )
	, texture_( 0 )
{

}

Line::~Line()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void Line::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & vertex_list_[ 0 ];
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void Line::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof( Index ) * index_list_.size();

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & index_list_[ 0 ];

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Line::create_texture()
{
	texture_ = blue_sky::GameMain::get_instance()->get_graphics_manager()->load_named_texture( "lines", "media/texture/lines.png" );
	// texture_ = blue_sky::GameMain::get_instance()->get_graphics_manager()->load_named_texture( "lines", "media/texture/test.png" );
	// texture_ = blue_sky::GameMain::get_instance()->get_graphics_manager()->load_named_texture( "lines", "media/model/building-roof-3.png" );
}

void Line::render_part( int part_count ) const
{
	if ( part_count < 1 )
	{
		return;
	}

	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );

	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	texture_->bind_to_ps( 0 );
	// direct_3d_->bind_texture_to_ps( 0, texture_ );

	// debug !!!
	// ID3D11ShaderResourceView* text_view = direct_3d_->getTextView();
	// direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & text_view );

	direct_3d_->getImmediateContext()->DrawIndexed( std::min< int >( part_count * 2, index_size_ ), 0, 0 );

	GameMain::get_instance()->get_graphics_manager()->count_draw();
}

} // namespace blue_sky::graphics::direct_3d_11

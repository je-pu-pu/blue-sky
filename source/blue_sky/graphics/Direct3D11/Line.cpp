#include "Line.h"

#include <core/graphics/GraphicsManager.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

#include <common/exception.h>
#include <common/math.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace blue_sky::graphics::direct_3d_11
{

Line::Line( Direct3D* direct_3d, blue_sky::graphics::GraphicsManager* graphics_manager )
	: direct_3d_( direct_3d )
	, graphics_manager_( graphics_manager )
	, vertex_buffer_( nullptr )
	, index_buffer_( nullptr )
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
    
	D3D11_SUBRESOURCE_DATA data = { nullptr };
	data.pSysMem = & vertex_list_[ 0 ];
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void Line::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof( Index ) * index_list_.size();

	D3D11_SUBRESOURCE_DATA data = { nullptr };
	data.pSysMem = & index_list_[ 0 ];

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Line::create_texture()
{
	texture_ = graphics_manager_->get_texture_handle_of( graphics_manager_->load_named_texture( "lines", "media/texture/lines.png" ) );
}

void Line::render_part( int part_count ) const
{
	if ( part_count < 1 )
	{
		return;
	}

	// 画像はハンドルから引く。解放されていれば描かない（理由は 1 回だけログに出す）
	const Texture* texture = graphics_manager_->resolve_texture( texture_ );

	if ( ! texture )
	{
		if ( ! texture_lost_logged_ )
		{
			texture_lost_logged_ = true;
			std::cout << "[Line] 線の画像（lines）を引けません。線を描きません" << std::endl;
		}

		return;
	}

	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );

	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	texture->bind_to_ps( 0 );
	// direct_3d_->bind_texture_to_ps( 0, texture_ );

	// debug !!!
	// ID3D11ShaderResourceView* text_view = direct_3d_->getTextView();
	// direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & text_view );

	direct_3d_->getImmediateContext()->DrawIndexed( std::min< int >( part_count * 2, static_cast< int >( index_size_ ) ), 0, 0 );

	graphics_manager_->count_draw();
}

} // namespace blue_sky::graphics::direct_3d_11

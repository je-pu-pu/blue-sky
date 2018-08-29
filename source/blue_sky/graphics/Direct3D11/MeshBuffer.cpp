#include "MeshBuffer.h"
#include <GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

#include <cstring>

namespace blue_sky::graphics::direct_3d_11
{

/**
 * 頂点バッファを作成する
 *
 */
void MeshBuffer::create_vertex_buffer()
{
	///@todo type によってバッファを作り分ける

	if ( get_vertex_list().empty() )
	{
		return;
	}

	vertex_buffer_list_.clear();

	{
		ID3D11Buffer* buffer = 0;
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Mesh::Vertex ) * get_vertex_list().size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & get_vertex_list()[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & buffer ) );

		vertex_buffer_list_.push_back( com_ptr< ID3D11Buffer >( buffer ) );
	}

	if ( ! get_vertex_weight_list().empty() )
	{
		ID3D11Buffer* buffer = 0;
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Mesh::VertexWeight ) * get_vertex_weight_list().size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & get_vertex_weight_list()[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & buffer ) );

		vertex_buffer_list_.push_back( com_ptr< ID3D11Buffer >( buffer ) );
	}
}

/**
 * 全ての頂点グループのインデックスバッファを作成する
 *
 */
void MeshBuffer::create_index_buffer()
{
	index_buffer_list_.clear();
	get_index_count_list().clear();

	index_buffer_list_.resize( get_vertex_group_list().size() );

	for ( uint_t n = 0; n < get_vertex_group_list().size(); n++ )
	{
		if ( get_vertex_group_list()[ n ]->empty() )
		{
			get_index_count_list().push_back( 0 );
			continue;
		}

		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Mesh::Index ) * get_vertex_group_list()[ n ]->size();

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = get_vertex_group_list()[ n ]->get_pointer();

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_list_[ n ] ) );

		get_index_count_list().push_back( get_vertex_group_list()[ n ]->size() );
	}
}

/**
 * 頂点バッファを更新する
 *
 */
void MeshBuffer::update_vertex_buffer()
{
	/// @todo 実装する

	/*
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;

	DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_list_[ 0 ].get(), 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

	std::memcpy( mapped_subresource.pData, & get_vertex_list()[ 0 ], sizeof( Mesh::Vertex ) * get_vertex_list().size() );

	direct_3d_->getImmediateContext()->Unmap( vertex_buffer_list_[ 0 ].get(), 0 );
	*/
}

/**
 * インデックスバッファを更新する
 *
 */
void MeshBuffer::update_index_buffer()
{
	/// @todo 実装する
}

/**
 * 入力アセンブラに設定する
 *
 */
void MeshBuffer::bind() const
{
	if ( vertex_buffer_list_.empty() )
	{
		return;
	}

	UINT buffer_count = vertex_buffer_list_.size();
	UINT stride[] = { sizeof( Mesh::Vertex ), sizeof( Mesh::VertexWeight ) };
	UINT offset[] = { 0, 0 };

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, buffer_count, & vertex_buffer_list_[ 0 ], stride, offset );
}

/**
 * メッシュ内のインデックスで指定した頂点グループのみを描画する
 *
 * @param n 描画する頂点グループのインデックス
 */
void MeshBuffer::render( uint_t n ) const
{
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_list_[ n ].get(), IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->DrawIndexed( get_index_count_list()[ n ], 0, 0 );
	
	GameMain::get_instance()->get_graphics_manager()->count_draw();
}

} // namespace blue_sky::graphics::direct_3d_11

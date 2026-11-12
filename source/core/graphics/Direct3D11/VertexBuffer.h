#pragma once

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/VertexBuffer.h>

#include <d3d11.h>

#include <cassert>
#include <cstring>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 の CPU から書き換えられる頂点バッファ（core::graphics::VertexBuffer の実装）
 *
 * 書き換えは前の中身を捨てて書き直す（DYNAMIC + Map(WRITE_DISCARD)）。
 * 制限 : IASetVertexBuffers() で複数の頂点バッファを同時に設定することはできない
 */
class VertexBuffer : public core::graphics::VertexBuffer
{
private:
	Direct3D11*					direct_3d_;		///< バッファを作る・更新する・描く先（作り手が渡す）
	com_ptr< ID3D11Buffer >		buffer_;
	UINT						vertex_size_;	///< 1 頂点の大きさ（バイト）
	size_t						vertex_count_;	///< 置ける頂点の数

public:
	VertexBuffer( Direct3D11* direct_3d, size_t vertex_size, size_t vertex_count )
		: direct_3d_( direct_3d )
		, vertex_size_( static_cast< UINT >( vertex_size ) )
		, vertex_count_( vertex_count )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.ByteWidth = static_cast< UINT >( vertex_size * vertex_count );

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, nullptr, & buffer_ ) );
	}

	void update( const void* vertices, size_t vertex_count ) override
	{
		assert( vertex_count <= vertex_count_ );

		D3D11_MAPPED_SUBRESOURCE mapped_subresource;

		DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( buffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

		if ( vertex_count > 0 )
		{
			std::memcpy( mapped_subresource.pData, vertices, static_cast< size_t >( vertex_size_ ) * vertex_count );
		}

		direct_3d_->getImmediateContext()->Unmap( buffer_.get(), 0 );
	}

	void draw( size_t vertex_count ) const override
	{
		UINT stride = vertex_size_;
		UINT offset = 0;

		auto* context = direct_3d_->getImmediateContext();
		ID3D11Buffer* buffer = buffer_.get();

		context->IASetVertexBuffers( 0, 1, & buffer, & stride, & offset );
		context->Draw( static_cast< UINT >( vertex_count ), 0 );
	}

}; // class VertexBuffer

} // namespace core::graphics::direct_3d_11

#pragma once

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/BufferType.h>
#include <core/type.h>

#include <d3d11.h>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 の頂点バッファのラッパークラス
 *
 * コンストラクタに任意のコンテナを指定することで、そのコンテナの要素の型と、その時点でのコンテナのサイズで頂点バッファを作成する
 * 
 * 制限 : IASetVertexBuffers() で複数の頂点バッファを同時に設定することはできない
 */
template< typename ContainerType >
class VertexBuffer
{
public:

private:
	com_ptr< ID3D11Buffer > buffer_;
	D3D11_BUFFER_DESC buffer_desc_ = { 0 };

public:
	/**
	 * @brief コンテナを使用して頂点バッファを作成する
	 * @param container 頂点バッファに設定するデータの入ったコンテナ
	 * @param type 作成する頂点バッファの種類
	 */
	VertexBuffer( const ContainerType& container, BufferType type = BufferType::DEFAULT )
	{
		buffer_desc_.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc_.Usage = ( type == BufferType::UPDATABLE ) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		buffer_desc_.CPUAccessFlags = ( type == BufferType::UPDATABLE ) ? D3D11_CPU_ACCESS_WRITE : 0;
		buffer_desc_.ByteWidth = sizeof( ContainerType::value_type ) * container.size();
    
		D3D11_SUBRESOURCE_DATA data = { nullptr };
		data.pSysMem = & container[ 0 ];

		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateBuffer( & buffer_desc_, & data, & buffer_ ) );
	}

	/**
	 * @brief 頂点バッファを指定したコンテナで更新する
	 * 
	 * @param container 頂点バッファを更新するデータの入ったコンテナ
	 */
	void update( const ContainerType& container )
	{
		assert( buffer_desc_.Usage == D3D11_USAGE_DYNAMIC );
		assert( buffer_desc_.ByteWidth == sizeof( ContainerType::value_type ) * container.size() );

		D3D11_MAPPED_SUBRESOURCE mapped_subresource;

		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getImmediateContext()->Map( buffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

		memcpy( mapped_subresource.pData, & container[ 0 ], sizeof( ContainerType::value_type ) * container.size() );

		Direct3D11::get_instance()->getImmediateContext()->Unmap( buffer_.get(), 0 );
	}

	/**
	 * @brief 頂点バッファを入力アセンブラに設定する
	 * 
	 */
	void bind()
	{
		UINT stride = sizeof( ContainerType::value_type );
		UINT offset = 0;

		Direct3D11::get_instance()->getImmediateContext()->IASetVertexBuffers( 0, 1, & buffer_, & stride, & offset );
	}

}; // class VertexBuffer

} // namespace core::graphics::direct_3d_11

#pragma once

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/type.h>
#include <vector>

namespace core::graphics::direct_3d_11
{

/**
 * インスタンスバッファ
 *
 * 複数のインスタンスのワールド行列を格納する StructuredBuffer
 */
class InstanceBuffer
{
public:
	static constexpr int MAX_INSTANCES = 1024;
	static constexpr int SLOT = 10; // t10 に対応

private:
	ID3D11Buffer* buffer_ = nullptr;
	ID3D11ShaderResourceView* srv_ = nullptr;
	size_t current_count_ = 0;

public:
	InstanceBuffer()
	{
		create_buffer();
	}

	~InstanceBuffer()
	{
		DIRECT_X_RELEASE( srv_ );
		DIRECT_X_RELEASE( buffer_ );
	}

	InstanceBuffer( const InstanceBuffer& ) = delete;
	InstanceBuffer& operator=( const InstanceBuffer& ) = delete;

	using Matrix = core::Matrix;

	/**
	 * バッファを作成
	 */
	void create_buffer()
	{
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.ByteWidth = sizeof( Matrix ) * MAX_INSTANCES;
		buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffer_desc.StructureByteStride = sizeof( Matrix );

		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateBuffer( &buffer_desc, nullptr, &buffer_ ) );

		// SRV を作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = MAX_INSTANCES;

		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateShaderResourceView( buffer_, &srv_desc, &srv_ ) );
	}

	/**
	 * インスタンスデータを更新
	 *
	 * @param matrices ワールド行列の配列
	 * @param count インスタンス数
	 */
	void update( const Matrix* matrices, size_t count )
	{
		if ( count == 0 || count > MAX_INSTANCES )
		{
			return;
		}

		current_count_ = count;

		D3D11_MAPPED_SUBRESOURCE mapped;
		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getImmediateContext()->Map( buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );

		memcpy( mapped.pData, matrices, sizeof( Matrix ) * count );

		Direct3D11::get_instance()->getImmediateContext()->Unmap( buffer_, 0 );
	}

	/**
	 * 頂点シェーダーにバインド
	 */
	void bind_to_vs() const
	{
		Direct3D11::get_instance()->getImmediateContext()->VSSetShaderResources( SLOT, 1, &srv_ );
	}

	/**
	 * バインドを解除
	 */
	void unbind_from_vs() const
	{
		ID3D11ShaderResourceView* null_srv = nullptr;
		Direct3D11::get_instance()->getImmediateContext()->VSSetShaderResources( SLOT, 1, &null_srv );
	}

	/**
	 * 現在のインスタンス数を取得
	 */
	size_t get_current_count() const { return current_count_; }

}; // class InstanceBuffer

} // namespace core::graphics::direct_3d_11

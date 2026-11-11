#pragma once

#include <core/graphics/ObjectInstanceBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/ConstantBuffer.h>

#include <wrl/client.h>

#include <cstring>

namespace core::graphics::direct_3d_11
{

/**
 * 物体ごとの定数の入れ物（D3D11 の実装）
 *
 * 物体の定数の表（t11）と番号の表（t12）を、CPU から書ける構造化バッファで持つ。足りなくなったら 2 倍に作り直す。
 * まとめの先頭の位置は小さな定数バッファ（b6）で渡す（SV_InstanceID は描く命令の StartInstanceLocation を足さないので）
 */
class ObjectInstanceBuffer : public core::graphics::ObjectInstanceBuffer
{
public:
	static constexpr UINT InstanceSlot = 11;		///< 物体の定数の表（object_instance.hlsl の ObjectInstances）
	static constexpr UINT IndexSlot = 12;			///< 番号の表（ObjectInstanceIndices）
	static constexpr UINT OffsetSlot = 6;			///< まとめの先頭の位置（ObjectInstanceConstantBuffer）

	static_assert( sizeof( Data ) == 144, "物体の定数の並びは、シェーダーの ObjectInstanceData（144 バイト）と同じにする" );

private:
	template< typename T > using ComPtr = Microsoft::WRL::ComPtr< T >;

	/// 構造化バッファ 1 つ（バッファとシェーダーから読むためのビュー）
	struct Buffer
	{
		ComPtr< ID3D11Buffer >				buffer;
		ComPtr< ID3D11ShaderResourceView >	view;
		size_t								capacity = 0;	///< 入る要素の数
	};

	Direct3D11*					direct_3d_;		///< バッファを作る・書く先（作り手が渡す）
	Buffer						instances_;
	Buffer						indices_;
	ComPtr< ID3D11Buffer >		offset_buffer_;

	/// 要素 count 個（1 つ stride バイト）が入るように、足りなければ 2 倍ずつ大きくして作り直す
	void reserve( Buffer& b, size_t count, UINT stride )
	{
		if ( count <= b.capacity )
		{
			return;
		}

		size_t capacity = b.capacity ? b.capacity : 256;

		while ( capacity < count )
		{
			capacity *= 2;
		}

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = static_cast< UINT >( stride * capacity );
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;

		ComPtr< ID3D11Buffer > buffer;
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & desc, nullptr, buffer.GetAddressOf() ) );

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = DXGI_FORMAT_UNKNOWN;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		view_desc.Buffer.FirstElement = 0;
		view_desc.Buffer.NumElements = static_cast< UINT >( capacity );

		ComPtr< ID3D11ShaderResourceView > view;
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateShaderResourceView( buffer.Get(), & view_desc, view.GetAddressOf() ) );

		b.buffer = buffer;
		b.view = view;
		b.capacity = capacity;
	}

	/// 中身を data で置き換える（前の中身は捨てる）
	void write( Buffer& b, const void* data, size_t count, UINT stride )
	{
		if ( count == 0 )
		{
			return;
		}

		reserve( b, count, stride );

		auto* context = direct_3d_->getImmediateContext();

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		DIRECT_X_FAIL_CHECK( context->Map( b.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped ) );

		std::memcpy( mapped.pData, data, stride * count );

		context->Unmap( b.buffer.Get(), 0 );
	}

public:
	explicit ObjectInstanceBuffer( Direct3D11* direct_3d )
		: direct_3d_( direct_3d )
	{
		offset_buffer_.Attach( create_d3d11_constant_buffer( direct_3d_, 16, core::graphics::ConstantBufferUsage::DYNAMIC ) );
	}

	ObjectInstanceBuffer( const ObjectInstanceBuffer& ) = delete;
	ObjectInstanceBuffer& operator = ( const ObjectInstanceBuffer& ) = delete;

	void update_instances( const Data* data, size_t count ) override
	{
		write( instances_, data, count, sizeof( Data ) );
	}

	void update_indices( const Index* indices, size_t count ) override
	{
		write( indices_, indices, count, sizeof( Index ) );
	}

	void bind_to_vs( Index offset ) const override
	{
		const Index offset_data[ 4 ] = { offset, 0, 0, 0 };
		update_d3d11_constant_buffer( direct_3d_, offset_buffer_.Get(), offset_data, sizeof( offset_data ), core::graphics::ConstantBufferUsage::DYNAMIC );

		auto* context = direct_3d_->getImmediateContext();

		ID3D11Buffer* constant_buffer = offset_buffer_.Get();
		context->VSSetConstantBuffers( OffsetSlot, 1, & constant_buffer );

		ID3D11ShaderResourceView* views[] = { instances_.view.Get(), indices_.view.Get() };
		context->VSSetShaderResources( InstanceSlot, 2, views );
	}

}; // class ObjectInstanceBuffer

} // namespace core::graphics::direct_3d_11

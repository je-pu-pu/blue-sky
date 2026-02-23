#pragma once

#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

namespace core::graphics::direct_3d_11
{

/**
 * ランタイム slot 指定の Direct3D11 定数バッファ
 *
 * テンプレート版 ConstantBuffer<Slot> と同じ機能を提供するが、
 * slot をコンストラクタ引数で受け取ることでランタイムに決定できる。
 */
class DynamicSlotConstantBuffer : public core::graphics::ConstantBuffer
{
private:
	ID3D11Buffer* constant_buffer_;
	int slot_;

public:
	DynamicSlotConstantBuffer( size_t size, int slot )
		: constant_buffer_( nullptr )
		, slot_( slot )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = size + ( size % 16 == 0 ? 0 : 16 - size % 16 );

		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
	}

	~DynamicSlotConstantBuffer()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	void update( const void* data ) const override
	{
		Direct3D11::get_instance()->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
	}

	void bind_to_vs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->VSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_hs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->HSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_ds() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->DSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_cs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->CSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_gs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->GSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_ps() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->PSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

}; // class DynamicSlotConstantBuffer

} // namespace core::graphics::direct_3d_11

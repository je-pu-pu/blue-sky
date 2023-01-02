#pragma once

#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D11 定数バッファ
 * 
` * バッファの型をを持たず、データも保持しない
 */
template< int Slot >
class ConstantBuffer : public core::graphics::ConstantBuffer
{
private:
	ID3D11Buffer*	constant_buffer_;

protected:
	void create_constant_buffer( size_t size )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };
		
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT; /// @todo D3D11_USAGE_DYNAMIC と使い分ける
		buffer_desc.ByteWidth = size + ( size % 16 == 0 ? 0 : 16 - size % 16 );
		
		DIRECT_X_FAIL_CHECK( Direct3D11::get_instance()->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
	}

public:
	ConstantBuffer( size_t size )
		: constant_buffer_( nullptr )
	{
		create_constant_buffer( size );
	}

	ConstantBuffer( const ConstantBuffer& buffer )
	{
		D3D11_BUFFER_DESC desc;
		constant_buffer_->GetDesc( & desc );

		create_constant_buffer( desc.ByteWidth );
	}

	~ConstantBuffer()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	virtual void update( const void* data ) const
	{
		/// @todo Map() と使い分ける
		Direct3D11::get_instance()->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
	}

	void bind_to_vs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->VSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_hs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->HSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_ds() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->DSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_cs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->CSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_gs() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->GSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_ps() const override
	{
		Direct3D11::get_instance()->getImmediateContext()->PSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

}; // class ConstantBuffer

} // namespace core::graphics::direct_3d_11

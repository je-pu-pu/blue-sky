#pragma once

#include "Direct3D11.h"

class Direct3D11;

/**
 * Direct3D11 定数バッファ
 *
 */
template< typename T >
class Direct3D11ConstantBuffer
{
public:
	typedef T Data;
	static const UINT Slot = T::DEFAULT_SLOT;

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	constant_buffer_;

	void create_constant_buffer( size_t size )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };
		
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT; /// @todo D3D11_USAGE_DYNAMIC と使い分ける
		buffer_desc.ByteWidth = size;
		
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
	}

public:
	Direct3D11ConstantBuffer( Direct3D11* direct_3d )
		: direct_3d_( direct_3d )
		, constant_buffer_( 0 )
	{
		create_constant_buffer( sizeof( T ) );
	}

	virtual ~Direct3D11ConstantBuffer()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	void update( const T* data ) const
	{
		/// @todo Map() と使い分ける
		direct_3d_->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
	}

	void bind_to_vs( UINT slot = Slot ) const
	{
		direct_3d_->getImmediateContext()->VSSetConstantBuffers( slot, 1, & constant_buffer_ );
	}

	void bind_to_gs( UINT slot = Slot ) const
	{
		direct_3d_->getImmediateContext()->GSSetConstantBuffers( slot, 1, & constant_buffer_ );
	}

	void bind_to_ps( UINT slot = Slot ) const
	{
		direct_3d_->getImmediateContext()->PSSetConstantBuffers( slot, 1, & constant_buffer_ );
	}

	void bind_to_all( UINT slot = Slot ) const
	{
		bind_to_vs( slot );
		bind_to_gs( slot );
		bind_to_ps( slot );
	}

}; // class Direct3D11ConstantBuffer

/**
 * Direct3D11 定数バッファ ( 更新用のデータも内部に保持するバージョン )
 *
 */
template< typename T >
class alignas( 16 ) Direct3D11ConstantBufferWithData : public Direct3D11ConstantBuffer< T >
{
protected:
	Data data_;

public:
	Direct3D11ConstantBufferWithData( Direct3D11* direct_3d )
		: Direct3D11ConstantBuffer( direct_3d )
	{

	}

	Data& data() { return data_; }
	const Data& data() const { return data_; }

	void update() const { Direct3D11ConstantBuffer::update( & data_ ); }

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}
};

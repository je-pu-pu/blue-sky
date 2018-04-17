#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11.h"
#include "DirectX.h"

#if 0

/*
Direct3D11ConstantBuffer::Direct3D11ConstantBuffer( Direct3D11* direct_3d, size_t size, UINT slot )
	: direct_3d_( direct_3d )
	, constant_buffer_( 0 )
	, slot_( slot )
{
	create_constant_buffer( size );
}
*/

Direct3D11ConstantBuffer< typename >::~Direct3D11ConstantBuffer()
{
	DIRECT_X_RELEASE( constant_buffer_ );
}

void Direct3D11ConstantBuffer< typename >::create_constant_buffer( size_t size )
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = size;
    
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
}

template< typename T >
void Direct3D11ConstantBuffer< typename T >::update( const T* data ) const
{
	direct_3d_->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
}

void Direct3D11ConstantBuffer< typename >::bind_to_vs( UINT slot ) const
{
	direct_3d_->getImmediateContext()->VSSetConstantBuffers( slot, 1, & constant_buffer_ );
}

void Direct3D11ConstantBuffer< typename >::bind_to_gs( UINT slot ) const
{
	direct_3d_->getImmediateContext()->GSSetConstantBuffers( slot, 1, & constant_buffer_ );
}

void Direct3D11ConstantBuffer< typename >::bind_to_ps( UINT slot ) const
{
	direct_3d_->getImmediateContext()->PSSetConstantBuffers( slot, 1, & constant_buffer_ );
} 

#endif
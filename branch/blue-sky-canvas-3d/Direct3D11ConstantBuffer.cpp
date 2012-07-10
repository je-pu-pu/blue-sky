#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>

Direct3D11ConstantBuffer::Direct3D11ConstantBuffer( Direct3D11* direct_3d, size_t size )
	: direct_3d_( direct_3d )
	, constant_buffer_( 0 )
{
	create_constant_buffer( size );
}

Direct3D11ConstantBuffer::~Direct3D11ConstantBuffer()
{
	DIRECT_X_RELEASE( constant_buffer_ );
}

void Direct3D11ConstantBuffer::create_constant_buffer( size_t size )
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = size;
    
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
}

void Direct3D11ConstantBuffer::update( const void* data )
{
	direct_3d_->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
}

void Direct3D11ConstantBuffer::render()
{
	direct_3d_->getImmediateContext()->VSSetConstantBuffers( 0, 1, & constant_buffer_ );
	direct_3d_->getImmediateContext()->GSSetConstantBuffers( 0, 1, & constant_buffer_ );
}

#ifndef DIRECT_3D_11_CONSTANT_BUFFER_H
#define DIRECT_3D_11_CONSTANT_BUFFER_H

#include "Direct3D11.h"
#include "DirectX.h"

#include <d3d11.h>

class Direct3D11;


/**
 * Direct3D11 �萔�o�b�t�@
 *
 */
template< typename T >
class Direct3D11ConstantBuffer
{
public:
	static const UINT Slot = T::DEFAULT_SLOT;

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	constant_buffer_;

	void create_constant_buffer( size_t size )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };
		
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT; /// @todo D3D11_USAGE_DYNAMIC �Ǝg��������
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
		/// @todo Map() �Ǝg��������
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

#endif // DIRECT_3D_11_CONSTANT_BUFFER_H

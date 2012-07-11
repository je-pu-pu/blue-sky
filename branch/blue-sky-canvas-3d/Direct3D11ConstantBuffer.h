#ifndef DIRECT_3D_11_CONSTANT_BUFFER_H
#define DIRECT_3D_11_CONSTANT_BUFFER_H

#include <d3d11.h>

class Direct3D11;

/**
 * Direct3D11 定数バッファ
 *
 */
class Direct3D11ConstantBuffer
{
public:

protected:
	Direct3D11*		direct_3d_;
	
	ID3D11Buffer*	constant_buffer_;

	void create_constant_buffer( size_t );

public:
	Direct3D11ConstantBuffer( Direct3D11*, size_t );
	virtual ~Direct3D11ConstantBuffer();

	void update( const void* );
	void render( UINT );

}; // class Direct3D11ConstantBuffer

#endif // DIRECT_3D_11_CONSTANT_BUFFER_H

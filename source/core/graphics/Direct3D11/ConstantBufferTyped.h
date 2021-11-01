#pragma once

#include "ConstantBuffer.h"

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D11 定数バッファ ( 指定した型のデータのみを更新できるバージョン ) 
 * 
 */
template< typename DataType, int Slot >
class ConstantBufferTyped : public ConstantBuffer< Slot >
{
private:
	void update( const void* ) const override
	{

	}

public:
	ConstantBufferTyped( )
		: ConstantBuffer( sizeof( DataType ) )
	{

	}

	void update( const DataType* data ) const
	{
		ConstantBuffer< Slot >::update( data );
	}

}; // class ConstantBufferTyped

} // namespace core::graphics::direct_3d_11

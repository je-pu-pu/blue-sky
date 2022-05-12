#pragma once

#include "ConstantBufferTyped.h"

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D11 定数バッファ ( 更新用のデータも内部に保持するバージョン )
 *
 */
template< typename DataType, int Slot >
class ConstantBufferWithData : public ConstantBufferTyped< DataType, Slot >
{
public:
	using ConstantBufferTyped = ConstantBufferTyped< DataType, Slot >;

protected:
	DataType data_;

public:
	DataType& data() { return data_; }
	const DataType& data() const { return data_; }

	void update() const { ConstantBufferTyped::update( & data_ ); }

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

};

} // namespace core::graphics::direct_3d_11

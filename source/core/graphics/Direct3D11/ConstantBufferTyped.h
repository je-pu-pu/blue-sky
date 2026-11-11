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
	/// usage : 毎フレーム・毎描画で書き換えるものは DYNAMIC にする（既定は DEFAULT）
	explicit ConstantBufferTyped( Direct3D11* direct_3d, core::graphics::ConstantBufferUsage usage = core::graphics::ConstantBufferUsage::DEFAULT )
		: ConstantBuffer< Slot >( direct_3d, sizeof( DataType ), usage )
	{

	}

	void update( const DataType* data ) const
	{
		ConstantBuffer< Slot >::update( data );
	}

}; // class ConstantBufferTyped

} // namespace core::graphics::direct_3d_11

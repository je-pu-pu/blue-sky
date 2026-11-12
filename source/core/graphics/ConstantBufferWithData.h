#pragma once

#include <core/graphics/ConstantBufferTyped.h>

#include <malloc.h>

namespace core::graphics
{

/**
 * 中身の型の決まった定数バッファ（書き込む前のデータも持つ版）
 *
 * data() を書き換えてから update() で GPU へ送る。
 */
template< typename Data, int Slot >
class ConstantBufferWithData : public ConstantBufferTyped< Data, Slot >
{
public:
	using ConstantBufferTyped = ConstantBufferTyped< Data, Slot >;
	using DataType = Data;

protected:
	DataType data_;

public:
	/// buffer : 実装が sizeof( DataType ) の大きさ・Slot の場所で作った定数バッファ
	explicit ConstantBufferWithData( std::unique_ptr< ConstantBuffer > buffer )
		: ConstantBufferTyped( std::move( buffer ) )
	{

	}

	DataType& data() { return data_; }
	const DataType& data() const { return data_; }

	void update() const { ConstantBufferTyped::update( & data_ ); }

	// 行列（16 バイト境界が要る）を持つので、16 バイト境界に置く
	static void* operator new ( size_t size )
	{
		return _aligned_malloc( size, 16 );
	}

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class ConstantBufferWithData

} // namespace core::graphics

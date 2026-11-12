#pragma once

#include <core/graphics/ConstantBuffer.h>

#include <memory>
#include <utility>

namespace core::graphics
{

/**
 * 中身の型の決まった定数バッファ
 *
 * 中身の型（DataType）とシェーダーのレジスタ番号（Slot）を型で持ち、型の合ったデータだけを書けるようにする。
 * GPU のバッファそのものは実装が作る ConstantBuffer（GraphicsManager::create_constant_buffer と同じもの）が持ち、
 * これは型を付けて取り次ぐだけ。使う側（ゲームの定数バッファ）が描画の実装（Direct3D 11）を知らずに済む。
 * 以前は Direct3D 11 の実装のテンプレートそのもので、ゲームの定数バッファの型が実装を知っていた
 */
template< typename Data, int Slot >
class ConstantBufferTyped : public ConstantBuffer
{
public:
	using DataType = Data;

	static constexpr int SLOT = Slot;	///< シェーダーのレジスタ番号

private:
	std::unique_ptr< ConstantBuffer > buffer_;	///< 実装のバッファ（大きさは sizeof( DataType )、場所は Slot で作ったもの）

	/// 型の無いデータでは書かせない（型の合う update( const DataType* ) を使う）
	void update( const void* data ) const override { buffer_->update( data ); }

public:
	/// buffer : 実装が sizeof( DataType ) の大きさ・Slot の場所で作った定数バッファ
	explicit ConstantBufferTyped( std::unique_ptr< ConstantBuffer > buffer )
		: ConstantBuffer( buffer->get_usage() )
		, buffer_( std::move( buffer ) )
	{

	}

	void update( const DataType* data ) const { buffer_->update( data ); }

	void bind_to_vs() const override { buffer_->bind_to_vs(); }
	void bind_to_hs() const override { buffer_->bind_to_hs(); }
	void bind_to_ds() const override { buffer_->bind_to_ds(); }
	void bind_to_cs() const override { buffer_->bind_to_cs(); }
	void bind_to_gs() const override { buffer_->bind_to_gs(); }
	void bind_to_ps() const override { buffer_->bind_to_ps(); }

}; // class ConstantBufferTyped

} // namespace core::graphics

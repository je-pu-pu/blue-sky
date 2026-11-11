#pragma once

#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Direct3D11/ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

namespace core::graphics::direct_3d_11
{

/**
 * ランタイム slot 指定の Direct3D11 定数バッファ
 *
 * テンプレート版 ConstantBuffer<Slot> と同じ機能を提供するが、
 * slot をコンストラクタ引数で受け取ることでランタイムに決定できる。
 */
class DynamicSlotConstantBuffer : public core::graphics::ConstantBuffer
{
private:
	Direct3D11* direct_3d_;		///< バッファを作る・更新する先（作り手が渡す。以前は Direct3D11 の get_instance() から取っていた）
	ID3D11Buffer* constant_buffer_;
	int slot_;
	size_t size_;				///< データの大きさ（DYNAMIC で書き込むときに使う）

public:
	/// usage : 毎フレーム・毎描画で書き換えるものは DYNAMIC にする（既定は DEFAULT）
	DynamicSlotConstantBuffer( Direct3D11* direct_3d, size_t size, int slot, Usage usage = Usage::DEFAULT )
		: core::graphics::ConstantBuffer( usage )
		, direct_3d_( direct_3d )
		, constant_buffer_( create_d3d11_constant_buffer( direct_3d, size, usage ) )
		, slot_( slot )
		, size_( size )
	{

	}

	~DynamicSlotConstantBuffer()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	void update( const void* data ) const override
	{
		update_d3d11_constant_buffer( direct_3d_, constant_buffer_, data, size_, get_usage() );
	}

	void bind_to_vs() const override
	{
		direct_3d_->getImmediateContext()->VSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_hs() const override
	{
		direct_3d_->getImmediateContext()->HSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_ds() const override
	{
		direct_3d_->getImmediateContext()->DSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_cs() const override
	{
		direct_3d_->getImmediateContext()->CSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_gs() const override
	{
		direct_3d_->getImmediateContext()->GSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

	void bind_to_ps() const override
	{
		direct_3d_->getImmediateContext()->PSSetConstantBuffers( slot_, 1, & constant_buffer_ );
	}

}; // class DynamicSlotConstantBuffer

} // namespace core::graphics::direct_3d_11

#pragma once

#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D11 定数バッファ
 * 
` * バッファの型をを持たず、データも保持しない
 */
template< int Slot >
class ConstantBuffer : public core::graphics::ConstantBuffer
{
private:
	Direct3D11*		direct_3d_;			///< バッファを作る・更新する先（作り手が渡す。以前は Direct3D11 の get_instance() から取っていた）
	ID3D11Buffer*	constant_buffer_;

protected:
	void create_constant_buffer( size_t size )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };
		
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT; /// @todo D3D11_USAGE_DYNAMIC と使い分ける
		buffer_desc.ByteWidth = size + ( size % 16 == 0 ? 0 : 16 - size % 16 );
		
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
	}

public:
	static constexpr int SLOT = Slot;	///< シェーダーのレジスタ番号（同じ場所へ汎用の定数バッファを作るときに使う）

	ConstantBuffer( Direct3D11* direct_3d, size_t size )
		: direct_3d_( direct_3d )
		, constant_buffer_( nullptr )
	{
		create_constant_buffer( size );
	}

	// GPU のバッファをコピーする意味はないので禁止する
	// （以前のコピーコンストラクタは、コピー元ではなく初期化していない自分のポインタに GetDesc していた）
	ConstantBuffer( const ConstantBuffer& ) = delete;
	ConstantBuffer& operator = ( const ConstantBuffer& ) = delete;

	~ConstantBuffer()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	void update( const void* data ) const override
	{
		/// @todo Map() と使い分ける
		direct_3d_->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
	}

	void bind_to_vs() const override
	{
		direct_3d_->getImmediateContext()->VSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_hs() const override
	{
		direct_3d_->getImmediateContext()->HSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_ds() const override
	{
		direct_3d_->getImmediateContext()->DSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_cs() const override
	{
		direct_3d_->getImmediateContext()->CSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_gs() const override
	{
		direct_3d_->getImmediateContext()->GSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

	void bind_to_ps() const override
	{
		direct_3d_->getImmediateContext()->PSSetConstantBuffers( Slot, 1, & constant_buffer_ );
	}

}; // class ConstantBuffer

} // namespace core::graphics::direct_3d_11

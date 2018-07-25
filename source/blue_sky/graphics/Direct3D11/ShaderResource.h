#pragma once

#include <blue_sky/graphics/ShaderResource.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

namespace blue_sky::graphics::direct_3d_11
{

/**
 * Direct3D11 定数バッファを使った ShaderResource
 *
 * @todo core に移動する
 */
template< typename DataType >
class ShaderResource : public blue_sky::graphics::ShaderResource
{
public:
	typedef DataType Data;
	static const UINT SLOT = DataType::SLOT;

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	constant_buffer_;

	void create_constant_buffer( size_t size )
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };
		
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT; /// @todo D3D11_USAGE_DYNAMIC と使い分ける
		buffer_desc.ByteWidth = size;
		
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & constant_buffer_ ) );
	}

public:
	ShaderResource( Direct3D11* direct_3d )
		: direct_3d_( direct_3d )
		, constant_buffer_( 0 )
	{
		create_constant_buffer( sizeof( DataType ) );
	}

	~ShaderResource()
	{
		DIRECT_X_RELEASE( constant_buffer_ );
	}

	void update( const DataType* data ) const
	{
		/// @todo Map() と使い分ける
		direct_3d_->getImmediateContext()->UpdateSubresource( constant_buffer_, 0, 0, data, 0, 0 );
	}

	void bind_to_vs() const override
	{
		direct_3d_->getImmediateContext()->VSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

	void bind_to_hs() const override
	{
		direct_3d_->getImmediateContext()->HSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

	void bind_to_ds() const override
	{
		direct_3d_->getImmediateContext()->DSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

	void bind_to_cs() const override
	{
		direct_3d_->getImmediateContext()->CSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

	void bind_to_gs() const override
	{
		direct_3d_->getImmediateContext()->GSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

	void bind_to_ps() const override
	{
		direct_3d_->getImmediateContext()->PSSetConstantBuffers( SLOT, 1, & constant_buffer_ );
	}

}; // class ShaderResource

/**
 * Direct3D11 定数バッファ ( 更新用のデータも内部に保持するバージョン )
 *
 */
template< typename DataType >
class ShaderResourceWithData : public ShaderResource< DataType >
{
protected:
	DataType data_;

public:
	ShaderResourceWithData( Direct3D11* direct_3d )
		: ShaderResource( direct_3d )
	{

	}

	DataType& data() { return data_; }
	const DataType& data() const { return data_; }

	void update() const { ShaderResource::update( & data_ ); }

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

};

} // namespace blue_sky::graphics::direct_3d_11
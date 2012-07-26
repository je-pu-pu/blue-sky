#include "Direct3D11Sprite.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11.h"
#include "Direct3D11Color.h"
#include "DirectX.h"

#include <win/Rect.h>

Direct3D11Sprite::Color Direct3D11Sprite::white_( 1, 1, 1, 1 );

struct SpriteConstantBuffer
{
	XMMATRIX transform;
};

Direct3D11Sprite::Direct3D11Sprite( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, constant_buffer_( 0 )
	, vertex_buffer_( 0 )
	, index_buffer_( 0 )
{
	constant_buffer_ = new ConstantBuffer( direct_3d, sizeof( SpriteConstantBuffer ), 3 );
	
	create_vertex_buffer();
	create_index_buffer();
}

Direct3D11Sprite::~Direct3D11Sprite()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );

	delete constant_buffer_;
}

void Direct3D11Sprite::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.ByteWidth = sizeof( Vertex ) * 4;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & vertex_buffer_ ) );
}

void Direct3D11Sprite::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.ByteWidth = sizeof( Index ) * 6;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	Index index_list[ 6 ] = { 0, 1, 2, 2, 1, 3 };

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = index_list;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Direct3D11Sprite::begin()
{
	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	direct_3d_->setInputLayout( "sprite" );

	SpriteConstantBuffer sprite_constant_buffer;
	sprite_constant_buffer.transform = XMMatrixIdentity();
	sprite_constant_buffer.transform = XMMatrixTranspose( sprite_constant_buffer.transform );
	constant_buffer_->update( & sprite_constant_buffer );
}

void Direct3D11Sprite::end()
{

}

void Direct3D11Sprite::draw( const Rect* dst, Texture* texture, const Rect* src, const Color* color )
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;

	DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

	Vertex* vertex_list = static_cast< Vertex* >( mapped_subresource.pData );

	vertex_list[ 0 ].Color = * color;
	vertex_list[ 1 ].Color = * color;
	vertex_list[ 2 ].Color = * color;
	vertex_list[ 3 ].Color = * color;

	{
		com_ptr< ID3D11Resource > texture_2d_resource;
		texture->GetResource( & texture_2d_resource );

		ID3D11Texture2D* texture_2d = static_cast< ID3D11Texture2D* >( texture_2d_resource.get() );

		D3D11_TEXTURE2D_DESC texture_2d_desc;

		texture_2d->GetDesc( & texture_2d_desc );

		if ( src )
		{
			FLOAT l = src->left()   / static_cast< FLOAT >( texture_2d_desc.Width );
			FLOAT r = src->right()  / static_cast< FLOAT >( texture_2d_desc.Width );
			FLOAT t = src->top()    / static_cast< FLOAT >( texture_2d_desc.Height );
			FLOAT b = src->bottom() / static_cast< FLOAT >( texture_2d_desc.Height );
	
			vertex_list[ 0 ].TexCoord = Vector2( l, t );
			vertex_list[ 1 ].TexCoord = Vector2( r, t );
			vertex_list[ 2 ].TexCoord = Vector2( l, b );
			vertex_list[ 3 ].TexCoord = Vector2( r, b );
		}
		else
		{
			vertex_list[ 0 ].TexCoord = Vector2( 0, 0 );
			vertex_list[ 1 ].TexCoord = Vector2( 1, 0 );
			vertex_list[ 2 ].TexCoord = Vector2( 0, 1 );
			vertex_list[ 3 ].TexCoord = Vector2( 1, 1 );
		}
	}

	{
		DXGI_SURFACE_DESC surface_desc;

		direct_3d_->getBackbufferSurface()->GetDesc( & surface_desc );

		FLOAT l = dst->left()   * 2.f / static_cast< FLOAT >( surface_desc.Width ) - 1.f;
		FLOAT r = dst->right()  * 2.f / static_cast< FLOAT >( surface_desc.Width ) - 1.f;
		FLOAT t = -( dst->top()    * 2.f / static_cast< FLOAT >( surface_desc.Height ) - 1.f );
		FLOAT b = -( dst->bottom() * 2.f / static_cast< FLOAT >( surface_desc.Height ) - 1.f );

		vertex_list[ 0 ].Position = Vector3( l, t, 0 );
		vertex_list[ 1 ].Position = Vector3( r, t, 0 );
		vertex_list[ 2 ].Position = Vector3( l, b, 0 );
		vertex_list[ 3 ].Position = Vector3( r, b, 0 );
	}

	direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );

	constant_buffer_->render();

	direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & texture );
	direct_3d_->getImmediateContext()->DrawIndexed( 6, 0, 0 );
}

void Direct3D11Sprite::draw( const Rect& dst, Texture* texture, const Rect& src, const Color& color )
{
	draw( & dst, texture, & src, & color );
}

void Direct3D11Sprite::draw( const Rect& dst, Texture* texture, const Color& color )
{
	draw( & dst, texture, 0, & color );
}
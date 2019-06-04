#include "Sprite.h"
#include "Effect.h"
#include "EffectTechnique.h"
#include "EffectPass.h"
#include "Direct3D11.h"
#include <GameMain.h>
#include <core/graphics/Direct3D11/ShaderResource.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <game/Texture.h>
#include <win/Rect.h>

namespace core::graphics::direct_3d_11
{

Sprite::Sprite( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, constant_buffer_( 0 )
	, vertex_buffer_( 0 )
	, index_buffer_( 0 )
	, input_layout_( direct_3d->get_input_layout( "sprite" ) )
	, effect_technique_( direct_3d_->get_effect()->get_technique( "sprite" ) )
	, ortho_offset_( 0.f )
{
	constant_buffer_ = new ConstantBuffer( direct_3d );
	
	create_vertex_buffer();
	create_index_buffer();
}

Sprite::~Sprite()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );

	delete constant_buffer_;
}

void Sprite::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.ByteWidth = sizeof( Vertex ) * 4;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & vertex_buffer_ ) );
}

void Sprite::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.ByteWidth = sizeof( Index ) * 4;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	Index index_list[ 4 ] = { 0, 1, 2, 3 };

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = index_list;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Sprite::begin()
{
	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	direct_3d_->set_input_layout( input_layout_ );

	set_transform( Matrix::identity() );
}

void Sprite::end()
{
	direct_3d_->bind_texture_to_ps( 0, nullptr );
}

void Sprite::set_transform( const Matrix& m )
{
	ConstantBufferData constant_buffer_data;
	constant_buffer_data.transform = m;
	constant_buffer_data.transform *= Matrix().set_orthographic( 2.f * static_cast< float >( direct_3d_->get_width() ) / static_cast< float >( direct_3d_->get_height() ), 2.f, -1.f, 1.f );
	constant_buffer_data.transform *= Matrix().set_translation( ortho_offset_, 0.f, 0.f );
	constant_buffer_data.transform = constant_buffer_data.transform.transpose();
	constant_buffer_->update( & constant_buffer_data );
}

void Sprite::draw( const Rect* dst, const Texture* texture, const Rect* src, const Color* color )
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;

	DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

	Vertex* vertex_list = static_cast< Vertex* >( mapped_subresource.pData );

	vertex_list[ 0 ].Color = * color;
	vertex_list[ 1 ].Color = * color;
	vertex_list[ 2 ].Color = * color;
	vertex_list[ 3 ].Color = * color;

	UINT src_width;
	UINT src_height;

	{
		if ( src )
		{
			FLOAT l = src->left()   / static_cast< FLOAT >( texture->get_width() );
			FLOAT r = src->right()  / static_cast< FLOAT >( texture->get_width() );
			FLOAT t = src->top()    / static_cast< FLOAT >( texture->get_height() );
			FLOAT b = src->bottom() / static_cast< FLOAT >( texture->get_height() );
	
			vertex_list[ 0 ].TexCoord = Vector2( l, t );
			vertex_list[ 1 ].TexCoord = Vector2( r, t );
			vertex_list[ 2 ].TexCoord = Vector2( l, b );
			vertex_list[ 3 ].TexCoord = Vector2( r, b );

			src_width = src->width();
			src_height = src->height();
		}
		else
		{
			vertex_list[ 0 ].TexCoord = Vector2( 0, 0 );
			vertex_list[ 1 ].TexCoord = Vector2( 1, 0 );
			vertex_list[ 2 ].TexCoord = Vector2( 0, 1 );
			vertex_list[ 3 ].TexCoord = Vector2( 1, 1 );

			src_width = texture->get_width();
			src_height = texture->get_height();
		}
	}

	{
		const auto screen_width = direct_3d_->get_width();
		const auto screen_height = direct_3d_->get_height();

		const FLOAT surface_ratio = static_cast< FLOAT >( screen_width ) / static_cast< FLOAT >( screen_height );

		FLOAT l, r, t, b;

		if ( dst )
		{
			l = +( dst->left()   * 2.f / static_cast< FLOAT >( screen_height ) - surface_ratio );
			r = +( dst->right()  * 2.f / static_cast< FLOAT >( screen_height ) - surface_ratio );
			t = -( dst->top()    * 2.f / static_cast< FLOAT >( screen_height ) - 1.f );
			b = -( dst->bottom() * 2.f / static_cast< FLOAT >( screen_height ) - 1.f );
		}
		else
		{
			l = +( ( ( static_cast< FLOAT >( screen_height ) - src_width  ) / 2               ) / screen_height * 2.f - 1.f );
			r = +( ( ( static_cast< FLOAT >( screen_height ) - src_width  ) / 2 + src_width   ) / screen_height * 2.f - 1.f );
			t = -( ( ( static_cast< FLOAT >( screen_height ) - src_height ) / 2               ) / screen_height * 2.f - 1.f );
			b = -( ( ( static_cast< FLOAT >( screen_height ) - src_height ) / 2 + src_height  ) / screen_height * 2.f - 1.f );

			/*
			l = ( static_cast< FLOAT >( screen_width  ) - src_width  ) / 2;
			r = ( static_cast< FLOAT >( screen_width  ) - src_width  ) / 2 + src_width;
			t = ( static_cast< FLOAT >( screen_height ) - src_height ) / 2;
			b = ( static_cast< FLOAT >( screen_height ) - src_height ) / 2 + src_height;
			*/
		}

		vertex_list[ 0 ].Position = Vector3( l, t, 0 );
		vertex_list[ 1 ].Position = Vector3( r, t, 0 );
		vertex_list[ 2 ].Position = Vector3( l, b, 0 );
		vertex_list[ 3 ].Position = Vector3( r, b, 0 );
	}

	direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );

	for ( auto pass : effect_technique_->get_pass_list() )
	{
		pass->apply();

		constant_buffer_->bind_to_vs();

		texture->bind_to_ps( 0 );

		direct_3d_->getImmediateContext()->DrawIndexed( 4, 0, 0 );
	}

	blue_sky::GameMain::get_instance()->get_graphics_manager()->count_draw();
}

} // namespace core::graphics::direct_3d_11

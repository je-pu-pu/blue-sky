#include "Sprite.h"
#include "Effect.h"
#include "EffectTechnique.h"
#include "EffectPass.h"
#include "Direct3D11.h"
#include "ConstantBufferTyped.h"
#include <core/Service.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Texture.h>
#include <win/Rect.h>

namespace core::graphics::direct_3d_11
{

Sprite::Sprite( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, constant_buffer_( nullptr )
	, vertex_buffer_( nullptr )
	, index_buffer_( nullptr )
	, input_layout_( direct_3d->get_input_layout( "sprite" ) )
	, effect_technique_( direct_3d_->get_effect()->get_technique( "sprite" ) )
	, color_technique_( direct_3d_->get_effect()->get_technique( "sprite_color" ) )
	, premultiplied_technique_( direct_3d_->get_effect()->get_technique( "sprite_premultiplied" ) )
	, ortho_offset_( 0.f )
{
	constant_buffer_ = std::make_unique< ConstantBuffer >();
	
	create_vertex_buffer();
	create_index_buffer();
}

Sprite::~Sprite()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void Sprite::create_vertex_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.ByteWidth = sizeof( Vertex ) * 4;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, nullptr, & vertex_buffer_ ) );
}

void Sprite::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.ByteWidth = sizeof( Index ) * 4;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	Index index_list[ 4 ] = { 0, 1, 2, 3 };

	D3D11_SUBRESOURCE_DATA data = { nullptr };
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
	if ( direct_3d_->get_height() == 0 )
	{
		return;		// 縦横比が求まらない（最小化中など）。draw_quad も描かない
	}

	ConstantBufferData constant_buffer_data;
	constant_buffer_data.transform = m;
	constant_buffer_data.transform *= Matrix().set_orthographic( 2.f * static_cast< float >( direct_3d_->get_width() ) / static_cast< float >( direct_3d_->get_height() ), 2.f, -1.f, 1.f );
	constant_buffer_data.transform *= Matrix().set_translation( ortho_offset_, 0.f, 0.f );
	constant_buffer_->update( & constant_buffer_data );
}

/**
 * 画面のピクセル座標の矩形に、UV 範囲と色を指定して四角形を 1 枚描く
 *
 * 頂点の座標は「高さを 2 とし、横は縦横比に合わせて広げた空間」（中心が原点）で書き込み、
 * set_transform() の正射影で画面に写す。
 *
 * @param texture nullptr ならテクスチャを使わない（technique 側も頂点カラーだけのものを渡す）
 */
void Sprite::draw_quad( const Rect& dst, const UVRect& uv, const Color& color, const EffectTechnique* technique, const Texture* texture )
{
	const FLOAT screen_height = static_cast< FLOAT >( direct_3d_->get_height() );

	// 最小化中など。座標が無限大になるので描かない（Map する前に判定する。Map した後で抜けると Unmap されずに残る）
	if ( screen_height <= 0.f )
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped_subresource;

	DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

	Vertex* vertex_list = static_cast< Vertex* >( mapped_subresource.pData );

	const FLOAT surface_ratio = static_cast< FLOAT >( direct_3d_->get_width() ) / screen_height;

	const FLOAT l = +( static_cast< FLOAT >( dst.left()   ) * 2.f / screen_height - surface_ratio );
	const FLOAT r = +( static_cast< FLOAT >( dst.right()  ) * 2.f / screen_height - surface_ratio );
	const FLOAT t = -( static_cast< FLOAT >( dst.top()    ) * 2.f / screen_height - 1.f );
	const FLOAT b = -( static_cast< FLOAT >( dst.bottom() ) * 2.f / screen_height - 1.f );

	vertex_list[ 0 ].Position = Vector3( l, t, 0 );
	vertex_list[ 1 ].Position = Vector3( r, t, 0 );
	vertex_list[ 2 ].Position = Vector3( l, b, 0 );
	vertex_list[ 3 ].Position = Vector3( r, b, 0 );

	vertex_list[ 0 ].TexCoord = Vector2( uv.u0, uv.v0 );
	vertex_list[ 1 ].TexCoord = Vector2( uv.u1, uv.v0 );
	vertex_list[ 2 ].TexCoord = Vector2( uv.u0, uv.v1 );
	vertex_list[ 3 ].TexCoord = Vector2( uv.u1, uv.v1 );

	for ( int i = 0; i < 4; i++ )
	{
		vertex_list[ i ].Color = color;
	}

	direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );

	for ( const auto& pass : technique->get_pass_list() )
	{
		pass->apply();

		constant_buffer_->bind_to_vs();

		if ( texture )
		{
			texture->bind_to_ps( 0 );
		}

		direct_3d_->getImmediateContext()->DrawIndexed( 4, 0, 0 );
	}

	core::get_graphics_manager()->count_draw();
}

void Sprite::draw( const Rect* dst, const Texture* texture, const Rect* src, const Color* color )
{
	if ( ! texture )
	{
		return;
	}

	const FLOAT texture_width = static_cast< FLOAT >( texture->get_width() );
	const FLOAT texture_height = static_cast< FLOAT >( texture->get_height() );

	UVRect uv;

	if ( src && texture_width > 0.f && texture_height > 0.f )
	{
		uv.u0 = static_cast< FLOAT >( src->left()   ) / texture_width;
		uv.u1 = static_cast< FLOAT >( src->right()  ) / texture_width;
		uv.v0 = static_cast< FLOAT >( src->top()    ) / texture_height;
		uv.v1 = static_cast< FLOAT >( src->bottom() ) / texture_height;
	}

	if ( dst )
	{
		draw_quad( * dst, uv, * color, effect_technique_, texture );
		return;
	}

	// 描画先の指定がなければ、切り出した大きさのまま画面中央に描く
	const LONG w = src ? src->width()  : static_cast< LONG >( texture->get_width() );
	const LONG h = src ? src->height() : static_cast< LONG >( texture->get_height() );
	const LONG x = ( static_cast< LONG >( direct_3d_->get_width() ) - w ) / 2;
	const LONG y = ( static_cast< LONG >( direct_3d_->get_height() ) - h ) / 2;

	draw_quad( Rect( x, y, x + w, y + h ), uv, * color, effect_technique_, texture );
}

void Sprite::draw_uv_impl( const Rect& dst, const Texture* texture, const UVRect& uv, const Color& color, AlphaMode mode )
{
	if ( ! texture )
	{
		return;
	}

	draw_quad( dst, uv, color, mode == AlphaMode::PREMULTIPLIED ? premultiplied_technique_ : effect_technique_, texture );
}

void Sprite::draw_color( const Rect& dst, const Color& color )
{
	draw_quad( dst, UVRect(), color, color_technique_, nullptr );
}

} // namespace core::graphics::direct_3d_11

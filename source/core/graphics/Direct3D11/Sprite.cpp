#include "Sprite.h"
#include "Effect.h"
#include "EffectTechnique.h"
#include "EffectPass.h"
#include "Direct3D11.h"
#include "ConstantBufferTyped.h"
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Texture.h>
#include <win/Rect.h>

#include <cstring>

namespace core::graphics::direct_3d_11
{

Sprite::Sprite( Direct3D* direct_3d, const core::graphics::GraphicsManager* graphics_manager )
	: direct_3d_( direct_3d )
	, constant_buffer_( nullptr )
	, vertex_buffer_( nullptr )
	, index_buffer_( nullptr )
	, input_layout_( direct_3d->get_input_layout( "sprite" ) )
	, effect_technique_( direct_3d_->get_effect()->get_technique( "sprite" ) )
	, color_technique_( direct_3d_->get_effect()->get_technique( "sprite_color" ) )
	, premultiplied_technique_( direct_3d_->get_effect()->get_technique( "sprite_premultiplied" ) )
	, graphics_manager_( graphics_manager )
{
	constant_buffer_ = std::make_unique< ConstantBuffer >( direct_3d_ );
	
	create_vertex_buffer();
	create_index_buffer();

	pending_vertices_.reserve( MAX_QUADS * 4 );
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
	buffer_desc.ByteWidth = static_cast< UINT >( sizeof( Vertex ) * 4 * MAX_QUADS );
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, nullptr, & vertex_buffer_ ) );
}

void Sprite::create_index_buffer()
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.ByteWidth = static_cast< UINT >( sizeof( Index ) * 6 * MAX_QUADS );
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	// 四角形ごとに 2 枚の三角形（左上・右上・左下 / 右上・右下・左下）
	std::vector< Index > index_list;
	index_list.reserve( 6 * MAX_QUADS );

	for ( size_t q = 0; q < MAX_QUADS; q++ )
	{
		const Index base = static_cast< Index >( q * 4 );
		index_list.insert( index_list.end(), { base, static_cast< Index >( base + 1 ), static_cast< Index >( base + 2 ), static_cast< Index >( base + 1 ), static_cast< Index >( base + 3 ), static_cast< Index >( base + 2 ) } );
	}

	D3D11_SUBRESOURCE_DATA data = { nullptr };
	data.pSysMem = index_list.data();

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );
}

void Sprite::bind_input_assembler()
{
	UINT stride = sizeof( Vertex );
	UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	direct_3d_->set_input_layout( input_layout_ );
}

void Sprite::begin()
{
	bind_input_assembler();
	set_transform( Matrix::identity() );
}

void Sprite::end()
{
	flush();
	direct_3d_->bind_texture_to_ps( 0, nullptr );
}

void Sprite::flush()
{
	if ( pending_vertices_.empty() )
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );
	std::memcpy( mapped_subresource.pData, pending_vertices_.data(), sizeof( Vertex ) * pending_vertices_.size() );
	direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );

	// 入力アセンブラは描画のたびに結び直す（begin と end の間に文字などを描くと、別の頂点バッファに替わっているため）
	bind_input_assembler();

	const UINT index_count = static_cast< UINT >( pending_vertices_.size() / 4 * 6 );

	for ( const auto& pass : pending_technique_->get_pass_list() )
	{
		pass->apply();

		constant_buffer_->bind_to_vs();

		if ( pending_texture_ )
		{
			pending_texture_->bind_to_ps( 0 );
		}

		direct_3d_->getImmediateContext()->DrawIndexed( index_count, 0, 0 );
	}

	if ( graphics_manager_ )
	{
		graphics_manager_->count_draw();
	}

	pending_vertices_.clear();
	pending_technique_ = nullptr;
	pending_texture_ = nullptr;
}

void Sprite::set_ortho_offset( float_t ortho_offset )
{
	direct_3d_->set_2d_offset_x( ortho_offset );
}

void Sprite::set_transform( const Matrix& m )
{
	flush();		// 溜めている四角形は、変える前の変換で描く

	const Pixel2DSpace space = direct_3d_->get_2d_space();

	if ( ! space.is_valid() )
	{
		return;		// 縦横比が求まらない（最小化中など）。draw_quad も描かない
	}

	ConstantBufferData constant_buffer_data;
	constant_buffer_data.transform = m;
	constant_buffer_data.transform *= Matrix().set_orthographic( space.ortho_width(), space.ortho_height(), -1.f, 1.f );
	constant_buffer_data.transform *= Matrix().set_translation( space.offset_x, 0.f, 0.f );
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
	const Pixel2DSpace space = direct_3d_->get_2d_space();

	// 最小化中など。座標が無限大になるので描かない（Map する前に判定する。Map した後で抜けると Unmap されずに残る）
	if ( ! space.is_valid() )
	{
		return;
	}

	// テクスチャや描き方が変わる・溜めきったときは、溜めている分を先に描く
	if ( ! pending_vertices_.empty() && ( technique != pending_technique_ || texture != pending_texture_ || pending_vertices_.size() >= MAX_QUADS * 4 ) )
	{
		flush();
	}

	pending_technique_ = technique;
	pending_texture_ = texture;

	Vertex vertex_list[ 4 ];

	const FLOAT l = space.to_view_x( static_cast< FLOAT >( dst.left()   ) );
	const FLOAT r = space.to_view_x( static_cast< FLOAT >( dst.right()  ) );
	const FLOAT t = space.to_view_y( static_cast< FLOAT >( dst.top()    ) );
	const FLOAT b = space.to_view_y( static_cast< FLOAT >( dst.bottom() ) );

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
		pending_vertices_.push_back( vertex_list[ i ] );
	}
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

	// 描き方はテクスチャ自身の属性で決める（乗算済みのテクスチャをストレートで描くと縁が暗くなる。指定は互換のために残す）
	const bool premultiplied = texture->is_premultiplied() || mode == AlphaMode::PREMULTIPLIED;
	draw_quad( dst, uv, color, premultiplied ? premultiplied_technique_ : effect_technique_, texture );
}

void Sprite::draw_color( const Rect& dst, const Color& color )
{
	draw_quad( dst, UVRect(), color, color_technique_, nullptr );
}

} // namespace core::graphics::direct_3d_11

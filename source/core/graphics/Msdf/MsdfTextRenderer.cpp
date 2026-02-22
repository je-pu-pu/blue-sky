#include "MsdfTextRenderer.h"
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/ConstantBufferTyped.h>
#include <core/graphics/Direct3D11/Effect.h>
#include <core/graphics/Direct3D11/EffectTechnique.h>
#include <core/graphics/Direct3D11/EffectPass.h>

namespace core::graphics {

MsdfTextRenderer::MsdfTextRenderer( direct_3d_11::Direct3D11* direct_3d, const char* font_path )
	: direct_3d_( direct_3d )
	, vertex_buffer_( nullptr )
	, index_buffer_( nullptr )
	, input_layout_( direct_3d->get_input_layout( "sprite" ) )
	, effect_technique_( direct_3d->get_effect()->get_technique( "msdf_text" ) )
{
	font_ = std::make_unique< MsdfFont >();
	font_->load( font_path );

	atlas_ = std::make_unique< GlyphAtlas >(
		direct_3d->getDevice(),
		direct_3d->getImmediateContext(),
		ATLAS_SIZE,
		CELL_SIZE
	);

	cache_ = std::make_unique< GlyphCache >( font_.get(), atlas_.get(), CELL_SIZE );

	msdf_constant_buffer_ = std::make_unique< MsdfConstantBuffer >();
	transform_constant_buffer_ = std::make_unique< TransformConstantBuffer >();

	create_vertex_buffer();
	create_index_buffer();

	vertices_.reserve( MAX_CHARS * 4 );
	indices_.reserve( MAX_CHARS * 6 );
}

MsdfTextRenderer::~MsdfTextRenderer()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void MsdfTextRenderer::create_vertex_buffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof( Vertex ) * MAX_CHARS * 4;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( &desc, nullptr, &vertex_buffer_ ) );
}

void MsdfTextRenderer::create_index_buffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof( uint16_t ) * MAX_CHARS * 6;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( &desc, nullptr, &index_buffer_ ) );
}

void MsdfTextRenderer::update_transform()
{
	TransformConstantBufferData data;
	data.transform = Matrix().set_orthographic(
		2.f * static_cast< float >( direct_3d_->get_width() ) / static_cast< float >( direct_3d_->get_height() ),
		2.f, -1.f, 1.f
	);

	transform_constant_buffer_->update( &data );
}

void MsdfTextRenderer::append_quad( float x, float y, float w, float h, const UVRect& uv, const Color& color )
{
	if ( vertices_.size() + 4 > MAX_CHARS * 4 )
	{
		return;
	}

	const auto screen_width = direct_3d_->get_width();
	const auto screen_height = direct_3d_->get_height();
	const float ratio = static_cast< float >( screen_width ) / static_cast< float >( screen_height );

	// ピクセル座標 → NDC 座標
	float l = +( x * 2.f / static_cast< float >( screen_height ) - ratio );
	float r = +( ( x + w ) * 2.f / static_cast< float >( screen_height ) - ratio );
	float t = -( y * 2.f / static_cast< float >( screen_height ) - 1.f );
	float b = -( ( y + h ) * 2.f / static_cast< float >( screen_height ) - 1.f );

	uint16_t base = static_cast< uint16_t >( vertices_.size() );

	vertices_.push_back( { Vector3( l, t, 0 ), Vector2( uv.u0, uv.v0 ), color } );
	vertices_.push_back( { Vector3( r, t, 0 ), Vector2( uv.u1, uv.v0 ), color } );
	vertices_.push_back( { Vector3( l, b, 0 ), Vector2( uv.u0, uv.v1 ), color } );
	vertices_.push_back( { Vector3( r, b, 0 ), Vector2( uv.u1, uv.v1 ), color } );

	indices_.push_back( base + 0 );
	indices_.push_back( base + 1 );
	indices_.push_back( base + 2 );
	indices_.push_back( base + 1 );
	indices_.push_back( base + 3 );
	indices_.push_back( base + 2 );
}

void MsdfTextRenderer::draw_text( float x, float y, const wchar_t* text, const TextStyle& style )
{
	if ( ! font_->is_loaded() )
	{
		return;
	}

	const auto& fm = font_->get_font_metrics();
	float scale = style.font_size / static_cast< float >( fm.emSize );
	float cursor_x = x;

	for ( const wchar_t* p = text; *p; ++p )
	{
		uint32_t codepoint = static_cast< uint32_t >( *p );

		if ( codepoint == '\n' )
		{
			cursor_x = x;
			y += style.font_size * static_cast< float >( fm.lineHeight / fm.emSize );
			continue;
		}

		const CachedGlyph* glyph = cache_->get( codepoint );

		if ( ! glyph )
		{
			continue;
		}

		if ( glyph->cell_index >= 0 )
		{
			// グリフのピクセル位置を計算
			float glyph_w = static_cast< float >( glyph->metrics.width ) * scale;
			float glyph_h = static_cast< float >( glyph->metrics.height ) * scale;
			float bearing_x = static_cast< float >( glyph->metrics.bearing_x ) * scale;
			float bearing_y = static_cast< float >( glyph->metrics.bearing_y ) * scale;

			// MSDF のパディングを考慮したサイズ
			float cell_scale = style.font_size / static_cast< float >( CELL_SIZE );
			float quad_size = static_cast< float >( CELL_SIZE ) * cell_scale;

			// グリフの中心に合わせてオフセット
			float offset_x = bearing_x * scale - ( quad_size - glyph_w ) * 0.5f;
			float offset_y = ( static_cast< float >( fm.ascenderY ) * scale ) - bearing_y * scale - ( quad_size - glyph_h ) * 0.5f;

			// 定数バッファ更新
			MsdfConstantBufferData cb_data;
			cb_data.text_color = style.text_color;
			cb_data.outline_color = style.outline_color;
			cb_data.outline_width = style.outline_width;
			cb_data.smoothing = 1.f / ( style.font_size * 0.5f );
			cb_data.padding_0 = 0.f;
			cb_data.padding_1 = 0.f;

			msdf_constant_buffer_->update( &cb_data );

			append_quad( cursor_x + offset_x, y + offset_y, quad_size, quad_size, glyph->uv, style.text_color );
		}

		cursor_x += static_cast< float >( glyph->metrics.advance ) * scale;
	}
}

void MsdfTextRenderer::draw_text( float x, float y, const char* text, const TextStyle& style )
{
	// UTF-8 → wchar_t 変換
	int len = MultiByteToWideChar( CP_UTF8, 0, text, -1, nullptr, 0 );

	if ( len <= 0 )
	{
		return;
	}

	std::vector< wchar_t > wtext( len );
	MultiByteToWideChar( CP_UTF8, 0, text, -1, wtext.data(), len );

	draw_text( x, y, wtext.data(), style );
}

void MsdfTextRenderer::flush()
{
	if ( vertices_.empty() )
	{
		return;
	}

	// 頂点バッファ更新
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		memcpy( mapped.pData, vertices_.data(), vertices_.size() * sizeof( Vertex ) );
		direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );
	}

	// インデックスバッファ更新
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( index_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		memcpy( mapped.pData, indices_.data(), indices_.size() * sizeof( uint16_t ) );
		direct_3d_->getImmediateContext()->Unmap( index_buffer_, 0 );
	}

	// レンダリング
	update_transform();

	UINT stride = sizeof( Vertex );
	UINT offset = 0;

	auto* context = direct_3d_->getImmediateContext();

	context->IASetVertexBuffers( 0, 1, &vertex_buffer_, &stride, &offset );
	context->IASetIndexBuffer( index_buffer_, DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	direct_3d_->set_input_layout( input_layout_ );

	auto* srv = atlas_->get_view();
	context->PSSetShaderResources( 0, 1, &srv );

	for ( const auto& pass : effect_technique_->get_pass_list() )
	{
		pass->apply();

		msdf_constant_buffer_->bind_to_ps();
		transform_constant_buffer_->bind_to_vs();

		context->DrawIndexed( static_cast< UINT >( indices_.size() ), 0, 0 );
	}

	// バッファクリア
	vertices_.clear();
	indices_.clear();
}

} // namespace core::graphics

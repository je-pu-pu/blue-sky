#include "MsdfTextRenderer.h"
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/ConstantBufferTyped.h>
#include <core/graphics/Direct3D11/Effect.h>
#include <core/graphics/Direct3D11/EffectTechnique.h>
#include <core/graphics/Direct3D11/EffectPass.h>
#include <common/string.h>
#include <algorithm>

namespace core::graphics {

/**
 * @brief コンストラクタ: MSDF テキスト描画に必要な全リソースを初期化する
 *
 * 初期化順序:
 * 1. FreeType でフォント読み込み (MsdfFont)
 * 2. グリフアトラステクスチャ作成 (GlyphAtlas: ATLAS_SIZE x ATLAS_SIZE, CELL_SIZE x CELL_SIZE セル)
 * 3. グリフキャッシュ作成 (GlyphCache: フォント + アトラスを参照)
 * 4. MSDF シェーダー用・射影変換用の定数バッファ作成
 * 5. 動的頂点/インデックスバッファ作成
 *
 * @param direct_3d D3D11 デバイスラッパー（テクスチャ・バッファ作成に使用）
 * @param font_path フォントファイルパス (.ttf / .otf)
 */
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

	// 1 文字 = 4 頂点 + 6 インデックスなので、MAX_CHARS 分を事前確保
	vertices_.reserve( MAX_CHARS * 4 );
	indices_.reserve( MAX_CHARS * 6 );
}

MsdfTextRenderer::~MsdfTextRenderer()
{
	DIRECT_X_RELEASE( index_buffer_ );
	DIRECT_X_RELEASE( vertex_buffer_ );
}

/**
 * @brief 動的頂点バッファを作成する
 *
 * D3D11_USAGE_DYNAMIC + D3D11_CPU_ACCESS_WRITE で作成し、
 * 毎フレーム Map/Unmap で書き換える。
 * サイズは MAX_CHARS * 4 頂点分。
 */
void MsdfTextRenderer::create_vertex_buffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof( Vertex ) * MAX_CHARS * 4;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( &desc, nullptr, &vertex_buffer_ ) );
}

/**
 * @brief 動的インデックスバッファを作成する
 *
 * 1 文字 = 2 三角形 = 6 インデックス (uint16)。
 * サイズは MAX_CHARS * 6 インデックス分。
 */
void MsdfTextRenderer::create_index_buffer()
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof( uint16_t ) * MAX_CHARS * 6;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( &desc, nullptr, &index_buffer_ ) );
}

/**
 * @brief スクリーンサイズから正射影行列を計算して定数バッファを更新する
 *
 * 正射影の幅は 2 * (width / height)、高さは 2.0 で固定。
 * これにより、append_quad() で計算した NDC 座標がそのままスクリーンに対応する。
 *
 * @note この正射影行列は、画面の高さを基準とした座標系を使う。
 *       幅方向はアスペクト比で自動調整される。
 */
void MsdfTextRenderer::update_transform()
{
	TransformConstantBufferData data;
	data.transform = Matrix().set_orthographic(
		2.f * static_cast< float >( direct_3d_->get_width() ) / static_cast< float >( direct_3d_->get_height() ),
		2.f, -1.f, 1.f
	);

	transform_constant_buffer_->update( &data );
}

/**
 * @brief 1 文字分のクワッド（2 三角形）を頂点/インデックスバッファに蓄積する
 *
 * ピクセル座標を NDC (Normalized Device Coordinates) に変換する。
 * 変換式は update_transform() の正射影行列と対応:
 *
 *   NDC_x = pixel_x * 2 / screen_height - aspect_ratio
 *   NDC_y = -(pixel_y * 2 / screen_height - 1)
 *
 * screen_height を基準にすることで、アスペクト比が変わっても
 * Y 方向のフォントサイズが一定に保たれる。
 *
 * クワッドの頂点順序:
 *   0(左上) --- 1(右上)
 *   |    ＼      |
 *   |      ＼    |
 *   2(左下) --- 3(右下)
 *
 * 三角形: [0,1,2] と [1,3,2]
 */
void MsdfTextRenderer::append_quad( float x, float y, float w, float h, const UVRect& uv, const Color& color )
{
	if ( vertices_.size() + 4 > MAX_CHARS * 4 )
	{
		return;
	}

	const auto screen_width = direct_3d_->get_width();
	const auto screen_height = direct_3d_->get_height();
	const float ratio = static_cast< float >( screen_width ) / static_cast< float >( screen_height );

	// ピクセル座標 → NDC 座標に変換
	float l = +( x * 2.f / static_cast< float >( screen_height ) - ratio );			// 左端
	float r = +( ( x + w ) * 2.f / static_cast< float >( screen_height ) - ratio );	// 右端
	float t = -( y * 2.f / static_cast< float >( screen_height ) - 1.f );				// 上端（Y 反転）
	float b = -( ( y + h ) * 2.f / static_cast< float >( screen_height ) - 1.f );		// 下端（Y 反転）

	uint16_t base = static_cast< uint16_t >( vertices_.size() );

	// 4 頂点: 左上、右上、左下、右下
	vertices_.push_back( { Vector3( l, t, 0 ), Vector2( uv.u0, uv.v0 ), color } );
	vertices_.push_back( { Vector3( r, t, 0 ), Vector2( uv.u1, uv.v0 ), color } );
	vertices_.push_back( { Vector3( l, b, 0 ), Vector2( uv.u0, uv.v1 ), color } );
	vertices_.push_back( { Vector3( r, b, 0 ), Vector2( uv.u1, uv.v1 ), color } );

	// 2 三角形: [左上, 右上, 左下] と [右上, 右下, 左下]
	indices_.push_back( base + 0 );
	indices_.push_back( base + 1 );
	indices_.push_back( base + 2 );
	indices_.push_back( base + 1 );
	indices_.push_back( base + 3 );
	indices_.push_back( base + 2 );
}

/**
 * @brief テキストを 1 文字ずつ走査し、グリフクワッドを頂点バッファに蓄積する（wchar_t 版）
 *
 * @par 描画座標の計算フロー（1 文字ごと）
 *
 * 1. EM 正規化メトリクスに scale (= font_size / emSize) を掛けてスクリーンピクセルに変換
 * 2. クワッドサイズを計算: MSDF ビットマップ全体がスクリーン上でどのサイズになるか
 *    - GlyphCache 側: msdf_scale = available / max(max_dim, 1.0)
 *    - こちら側: quad_size = CELL_SIZE * scale * max_dim_capped / available
 *    → msdf_scale × quad_size = CELL_SIZE × scale（逆数の関係で一致）
 * 3. グリフ本体をクワッド中央に配置するオフセットを計算
 *    - offset_x: bearing_x から左パディング分を引く
 *    - offset_y: ascender ラインからの下方オフセット
 *
 * @param x 描画開始位置の X ピクセル座標
 * @param y 描画開始位置の Y ピクセル座標（ベースラインではなく、ascender ラインの位置）
 * @param text 描画するテキスト（null 終端 wchar_t）
 * @param style テキストスタイル（フォントサイズ、色、アウトライン等）
 */
void MsdfTextRenderer::draw_text( float x, float y, const wchar_t* text, const TextStyle& style )
{
	if ( ! font_->is_loaded() )
	{
		return;
	}

	const auto& fm = font_->get_font_metrics();

	// EM 正規化座標 → スクリーンピクセルへの変換スケール
	float scale = style.font_size / static_cast< float >( fm.emSize );
	float cursor_x = x;

	// MSDF ビットマップ内でグリフ本体に使えるピクセル数（パディングを除く）
	float available = static_cast< float >( CELL_SIZE ) - 2.f * MSDF_RANGE;

	// MSDF シェーダー用の定数バッファを更新（全グリフ共通のパラメータ）
	MsdfConstantBufferData cb_data;
	cb_data.text_color = style.text_color;
	cb_data.outline_color = style.outline_color;
	cb_data.outline_width = style.outline_width;
	cb_data.px_range = 2.f * MSDF_RANGE;	// GlyphCache の Range(2.0 * msdf_range_ / scale) と対応
	cb_data.atlas_texel_size = 1.f / static_cast< float >( ATLAS_SIZE );
	cb_data.padding_1 = 0.f;

	msdf_constant_buffer_->update( &cb_data );

	for ( const wchar_t* p = text; *p; ++p )
	{
		uint32_t codepoint = static_cast< uint32_t >( *p );

		if ( codepoint == '\n' )
		{
			// 改行: カーソルを次の行の先頭に移動
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
			// グリフの EM 正規化メトリクスをスクリーンピクセルに変換
			float glyph_w = static_cast< float >( glyph->metrics.width ) * scale;
			float glyph_h = static_cast< float >( glyph->metrics.height ) * scale;
			float bearing_x = static_cast< float >( glyph->metrics.bearing_x ) * scale;
			float bearing_y = static_cast< float >( glyph->metrics.bearing_y ) * scale;

			// 統一クワッドサイズ: MSDF ビットマップ全体のスクリーンサイズ
			// max(max_dim, 1.0) は GlyphCache::generate_glyph() のスケール計算と対応
			// これにより全グリフで texel-to-screen 比率が統一され、アウトライン太さが一定になる
			float max_dim_capped = static_cast< float >( std::max( { glyph->metrics.width, glyph->metrics.height, 1.0 } ) );
			float quad_size = static_cast< float >( CELL_SIZE ) * scale * max_dim_capped / available;

			// グリフ本体がクワッド中央に来るようにオフセットを計算
			// offset_x: グリフ左端の bearing から、パディング分（クワッドとグリフの幅の差の半分）を引く
			float offset_x = bearing_x - ( quad_size - glyph_w ) * 0.5f;
			// offset_y: ascender ライン (y=0) からグリフ上端 (bearing_y) までの距離に、上パディングを加算
			float offset_y = ( static_cast< float >( fm.ascenderY ) * scale ) - bearing_y - ( quad_size - glyph_h ) * 0.5f;

			append_quad( cursor_x + offset_x, y + offset_y, quad_size, quad_size, glyph->uv, Color::White );
		}

		// カーソルを advance 分進める（空白文字もここで処理される）
		cursor_x += static_cast< float >( glyph->metrics.advance ) * scale;
	}
}

/**
 * @brief テキストを描画キューに追加する（UTF-8 版）
 *
 * UTF-8 文字列を common::convert_to_wstring() で wchar_t に変換して wchar_t 版に委譲する。
 */
void MsdfTextRenderer::draw_text( float x, float y, const char* text, const TextStyle& style )
{
	draw_text( x, y, common::convert_to_wstring( text ).c_str(), style );
}

/**
 * @brief テキストの描画幅をピクセル単位で計算する（wchar_t 版）
 *
 * 各文字の advance（送り幅）を合計する。最初の改行で計測を打ち切る。
 * カーニングは現在未対応。
 *
 * @param text 計測するテキスト
 * @param font_size フォントサイズ（ピクセル）
 * @return 最初の行のテキスト幅（ピクセル）
 */
float MsdfTextRenderer::measure_text_width( const wchar_t* text, float font_size )
{
	if ( ! font_->is_loaded() )
	{
		return 0.f;
	}

	const auto& fm = font_->get_font_metrics();
	float scale = font_size / static_cast< float >( fm.emSize );
	float width = 0.f;

	for ( const wchar_t* p = text; *p; ++p )
	{
		if ( *p == '\n' )
		{
			break;
		}

		const CachedGlyph* glyph = cache_->get( static_cast< uint32_t >( *p ) );

		if ( glyph )
		{
			width += static_cast< float >( glyph->metrics.advance ) * scale;
		}
	}

	return width;
}

/** @brief テキストの描画幅を計算する（UTF-8 版、wchar_t 版に委譲） */
float MsdfTextRenderer::measure_text_width( const char* text, float font_size )
{
	return measure_text_width( common::convert_to_wstring( text ).c_str(), font_size );
}

/**
 * @brief テキストの描画高さをピクセル単位で計算する（wchar_t 版）
 *
 * lineHeight（EM 正規化）× font_size × 行数 で計算する。
 * 空文字列でも最低 1 行分の高さを返す。
 *
 * @param text 計測するテキスト
 * @param font_size フォントサイズ（ピクセル）
 * @return テキスト全体の高さ（ピクセル）
 */
float MsdfTextRenderer::measure_text_height( const wchar_t* text, float font_size )
{
	if ( ! font_->is_loaded() )
	{
		return 0.f;
	}

	const auto& fm = font_->get_font_metrics();
	float line_height = font_size * static_cast< float >( fm.lineHeight / fm.emSize );
	int line_count = 1;

	for ( const wchar_t* p = text; *p; ++p )
	{
		if ( *p == '\n' )
		{
			line_count++;
		}
	}

	return line_height * line_count;
}

/** @brief テキストの描画高さを計算する（UTF-8 版、wchar_t 版に委譲） */
float MsdfTextRenderer::measure_text_height( const char* text, float font_size )
{
	return measure_text_height( common::convert_to_wstring( text ).c_str(), font_size );
}

/**
 * @brief 蓄積した全クワッドを一括描画する（バッチレンダリング）
 *
 * @par 描画手順
 * 1. CPU 側の頂点/インデックスデータを GPU バッファに転送 (Map/Unmap WRITE_DISCARD)
 * 2. 正射影行列を更新
 * 3. Input Assembler に頂点/インデックスバッファをバインド
 * 4. エフェクトの各パスを適用して DrawIndexed で描画
 * 5. 内部バッファをクリア（次のフレーム用）
 *
 * @note pass->apply() はシェーダーリソースのステートをリセットするため、
 *       apply() の後にアトラステクスチャを PSSetShaderResources でバインドする必要がある。
 *       順序を逆にするとテクスチャが見えなくなる。
 */
void MsdfTextRenderer::flush()
{
	if ( vertices_.empty() )
	{
		return;
	}

	// CPU → GPU: 頂点データ転送（WRITE_DISCARD で前フレームのデータを破棄）
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		memcpy( mapped.pData, vertices_.data(), vertices_.size() * sizeof( Vertex ) );
		direct_3d_->getImmediateContext()->Unmap( vertex_buffer_, 0 );
	}

	// CPU → GPU: インデックスデータ転送
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( index_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		memcpy( mapped.pData, indices_.data(), indices_.size() * sizeof( uint16_t ) );
		direct_3d_->getImmediateContext()->Unmap( index_buffer_, 0 );
	}

	// 正射影行列を現在のスクリーンサイズに合わせて更新
	update_transform();

	UINT stride = sizeof( Vertex );
	UINT offset = 0;

	auto* context = direct_3d_->getImmediateContext();

	// Input Assembler ステージの設定
	context->IASetVertexBuffers( 0, 1, &vertex_buffer_, &stride, &offset );
	context->IASetIndexBuffer( index_buffer_, DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	direct_3d_->set_input_layout( input_layout_ );

	// エフェクトの各パスで描画
	for ( const auto& pass : effect_technique_->get_pass_list() )
	{
		// パスを適用（頂点/ピクセルシェーダー、ブレンドステート等を設定）
		pass->apply();

		// 重要: apply() の後にテクスチャをバインドすること
		// apply() がシェーダーリソースのステートをリセットするため
		auto* srv = atlas_->get_view();
		context->PSSetShaderResources( 0, 1, &srv );

		// 定数バッファをバインド: MSDF パラメータ (PS slot 3)、射影行列 (VS slot 13)
		msdf_constant_buffer_->bind_to_ps();
		transform_constant_buffer_->bind_to_vs();

		context->DrawIndexed( static_cast< UINT >( indices_.size() ), 0, 0 );
	}

	// 次の描画サイクルに備えて内部バッファをクリア
	vertices_.clear();
	indices_.clear();
}

void MsdfTextRenderer::reload_font( const char* font_path )
{
	cache_->clear();
	font_->unload();
	font_->load( font_path );
}

} // namespace core::graphics

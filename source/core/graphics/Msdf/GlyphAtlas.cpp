#include "GlyphAtlas.h"
#include <algorithm>

namespace core::graphics {

GlyphAtlas::GlyphAtlas( ID3D11Device* device, ID3D11DeviceContext* context, int atlas_size, int cell_size )
	: device_( device )
	, context_( context )
	, atlas_size_( atlas_size )
	, cell_size_( cell_size )
	, cells_per_row_( atlas_size / cell_size )
	, total_cells_( cells_per_row_ * cells_per_row_ )
	, next_cell_index_( 0 )
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = atlas_size_;
	desc.Height = atlas_size_;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	// ゼロクリアした初期データで作成（未使用セルからのサンプリング防止）
	std::vector< uint8_t > zero_data( atlas_size_ * atlas_size_ * 4, 0 );

	D3D11_SUBRESOURCE_DATA init_data{};
	init_data.pSysMem = zero_data.data();
	init_data.SysMemPitch = atlas_size_ * 4;

	DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( &desc, &init_data, &texture_ ) );

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
	view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipLevels = 1;

	DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( texture_.get(), &view_desc, &view_ ) );
}

/**
 * @brief msdfgen の 3ch float ビットマップを RGBA8 に変換する
 *
 * msdfgen は R/G/B 各チャンネルに距離値 (float, 0.0〜1.0 付近) を格納する。
 * これを GPU テクスチャ用に 8bit RGBA に変換する。A チャンネルは常に 255。
 *
 * @note msdfgen のビットマップは y=0 が下端（OpenGL 座標系）なので、
 *       D3D11 のテクスチャ座標系（y=0 が上端）に合わせて上下反転する。
 *
 * @param bitmap msdfgen が生成した 3ch float ビットマップ
 * @return RGBA8 ピクセルデータ（サイズ: w * h * 4 バイト）
 */
static std::vector< uint8_t > convert_msdf_to_rgba( const msdfgen::Bitmap< float, 3 >& bitmap )
{
	int w = bitmap.width();
	int h = bitmap.height();
	std::vector< uint8_t > rgba( w * h * 4 );

	for ( int y = 0; y < h; ++y )
	{
		for ( int x = 0; x < w; ++x )
		{
			const float* pixel = bitmap( x, y );

			// msdfgen は y=0 が下端なので上下反転して D3D11 座標系に合わせる
			int dst_y = h - 1 - y;
			int dst = ( dst_y * w + x ) * 4;

			// float [0.0, 1.0] → uint8 [0, 255] に量子化（+0.5 は四捨五入）
			rgba[ dst + 0 ] = static_cast< uint8_t >( std::clamp( pixel[ 0 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 1 ] = static_cast< uint8_t >( std::clamp( pixel[ 1 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 2 ] = static_cast< uint8_t >( std::clamp( pixel[ 2 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 3 ] = 255;
		}
	}

	return rgba;
}

/**
 * @brief RGBA データをアトラステクスチャの指定セルにアップロードする
 *
 * セルインデックスから行・列を計算し、D3D11_BOX でアップロード領域を指定して
 * UpdateSubresource で GPU テクスチャを部分更新する。
 *
 * @param cell_index アップロード先のセルインデックス（0 〜 total_cells_-1）
 * @param rgba_data RGBA8 ピクセルデータ
 * @param width ビットマップ幅（ピクセル）
 * @param height ビットマップ高さ（ピクセル）
 */
void GlyphAtlas::upload_cell( int cell_index, const std::vector< uint8_t >& rgba_data, int width, int height )
{
	// セルインデックスからテクスチャ上の列・行を計算
	int col = cell_index % cells_per_row_;
	int row = cell_index / cells_per_row_;

	// アップロード先の矩形領域をピクセル座標で指定
	D3D11_BOX box{};
	box.left = col * cell_size_;
	box.top = row * cell_size_;
	box.right = box.left + width;
	box.bottom = box.top + height;
	box.front = 0;
	box.back = 1;

	// GPU テクスチャの該当領域を部分更新（CPU → GPU 転送）
	context_->UpdateSubresource( texture_.get(), 0, &box, rgba_data.data(), width * 4, 0 );
}

/**
 * @brief グリフの MSDF ビットマップをアトラスの次の空きセルに追加する
 *
 * @param bitmap msdfgen が生成した 3ch float ビットマップ
 * @param uv [out] 割り当てられたセルの UV 座標
 * @return 割り当てられたセルインデックス（アトラス満杯時は -1）
 */
int GlyphAtlas::add_glyph( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv )
{
	if ( is_full() )
	{
		return -1;
	}

	int cell_index = next_cell_index_++;

	replace_glyph( cell_index, bitmap, uv );

	return cell_index;
}

/**
 * @brief 既存セルのビットマップを上書きし、UV 座標を再計算する
 *
 * LRU 除去後のセル再利用時に呼ばれる。
 * MSDF ビットマップを RGBA8 に変換してアップロードし、
 * セル位置から UV 座標を計算する。
 *
 * UV 座標は半テクセル分インセットされる。これはバイリニアフィルタリング時に
 * 隣接セルのピクセルが混入するのを防ぐため。
 *
 * @param cell_index 上書き先のセルインデックス
 * @param bitmap msdfgen が生成した 3ch float ビットマップ
 * @param uv [out] セルの UV 座標（半テクセルインセット済み）
 */
void GlyphAtlas::replace_glyph( int cell_index, const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv )
{
	auto rgba = convert_msdf_to_rgba( bitmap );

	int w = bitmap.width();
	int h = bitmap.height();

	upload_cell( cell_index, rgba, w, h );

	// セルのテクスチャ上での位置から UV 座標を計算
	int col = cell_index % cells_per_row_;
	int row = cell_index / cells_per_row_;

	float inv = 1.f / static_cast< float >( atlas_size_ );

	// 半テクセルインセット: バイリニアフィルタリングで隣接セルが漏れるのを防ぐ
	float half_texel = 0.5f * inv;

	uv.u0 = col * cell_size_ * inv + half_texel;
	uv.v0 = row * cell_size_ * inv + half_texel;
	uv.u1 = ( col * cell_size_ + w ) * inv - half_texel;
	uv.v1 = ( row * cell_size_ + h ) * inv - half_texel;
}

} // namespace core::graphics

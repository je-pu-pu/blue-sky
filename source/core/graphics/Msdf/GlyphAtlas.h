/**
 * MSDF グリフを格納するテクスチャアトラス
 *
 * 1 枚の D3D11 テクスチャを等サイズのセルに分割し、
 * グリフの MSDF ビットマップを動的にアップロードする。
 * 各セルの UV 座標はバイリニアフィルタリングによる隣接セルへの漏れを防ぐため、
 * 半テクセル分インセットされる。
 */
#pragma once

#include <core/DirectX.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

#define MSDFGEN_PUBLIC
#define MSDFGEN_EXT_PUBLIC
#include <msdfgen/core/Bitmap.h>

namespace core::graphics {

/**
 * アトラス上のセルの UV 座標
 */
struct UVRect {
	float u0, v0;	///< 左上
	float u1, v1;	///< 右下
};

/**
 * MSDF グリフアトラス
 *
 * atlas_size x atlas_size ピクセルの RGBA テクスチャを作成し、
 * cell_size x cell_size ピクセルのセルに分割してグリフを管理する。
 * テクスチャは GPU 上に常駐し、UpdateSubresource で個別セルを更新する。
 */
class GlyphAtlas {
	com_ptr< ID3D11Texture2D > texture_;		///< アトラステクスチャ
	com_ptr< ID3D11ShaderResourceView > view_;	///< シェーダーリソースビュー

	ID3D11Device* device_;
	ID3D11DeviceContext* context_;

	int atlas_size_;		///< アトラス全体のピクセルサイズ（正方形）
	int cell_size_;			///< 1 セルのピクセルサイズ（正方形）
	int cells_per_row_;		///< 1 行あたりのセル数
	int total_cells_;		///< セルの総数
	int next_cell_index_;	///< 次に割り当てるセルのインデックス

	/**
	 * RGBA データを指定セルにアップロードする
	 *
	 * @param cell_index アップロード先のセルインデックス
	 * @param rgba_data RGBA ピクセルデータ
	 * @param width ビットマップ幅
	 * @param height ビットマップ高さ
	 */
	void upload_cell( int cell_index, const std::vector< uint8_t >& rgba_data, int width, int height );

public:
	/**
	 * コンストラクタ
	 *
	 * テクスチャをゼロクリアして作成する。
	 * ゼロクリアは未使用セルからのサンプリング時にゴミが出ないようにするため。
	 *
	 * @param device D3D11 デバイス
	 * @param context D3D11 デバイスコンテキスト
	 * @param atlas_size アトラスのピクセルサイズ（正方形、例: 2048）
	 * @param cell_size 1 セルのピクセルサイズ（正方形、例: 48）
	 */
	GlyphAtlas( ID3D11Device* device, ID3D11DeviceContext* context, int atlas_size, int cell_size );

	/**
	 * グリフの MSDF ビットマップをアトラスに追加する
	 *
	 * 次の空きセルにビットマップをアップロードし、UV 座標を返す。
	 *
	 * @param bitmap msdfgen が生成した 3ch float ビットマップ
	 * @param uv [out] 割り当てられたセルの UV 座標
	 * @return 割り当てられたセルインデックス（満杯の場合 -1）
	 */
	int add_glyph( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv );

	/**
	 * 既存セルのビットマップを上書きする（LRU 追い出し時に使用）
	 *
	 * @param cell_index 上書き先のセルインデックス
	 * @param bitmap msdfgen が生成した 3ch float ビットマップ
	 * @param uv [out] セルの UV 座標
	 */
	void replace_glyph( int cell_index, const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv );

	bool is_full() const { return next_cell_index_ >= total_cells_; }
	int get_cell_size() const { return cell_size_; }
	int get_atlas_size() const { return atlas_size_; }

	ID3D11ShaderResourceView* get_view() const { return view_.get(); }
};

} // namespace core::graphics

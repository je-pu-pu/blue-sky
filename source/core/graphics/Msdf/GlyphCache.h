/**
 * MSDF グリフの LRU キャッシュ
 *
 * Unicode コードポイントをキーとして、MSDF 生成済みグリフを管理する。
 * キャッシュミス時に MsdfFont からグリフ輪郭を取得し、msdfgen で MSDF ビットマップを生成、
 * GlyphAtlas にアップロードしてキャッシュに登録する。
 *
 * キャッシュが上限 (MAX_CACHED_GLYPHS) に達すると、LRU (Least Recently Used) 方式で
 * 最も古いエントリを除去する。アトラスが満杯の場合はセルの上書き (replace_glyph) で対応する。
 */
#pragma once

#include "MsdfFont.h"
#include "GlyphAtlas.h"
#include <unordered_map>
#include <list>
#include <cstdint>

namespace core::graphics {

/**
 * キャッシュされたグリフ情報
 *
 * アトラス上の UV 座標、EM 正規化メトリクス、セルインデックスを保持する。
 * cell_index が -1 の場合、輪郭のないグリフ（空白文字等）を表す。
 */
struct CachedGlyph {
	UVRect uv;				///< アトラス上のテクスチャ UV 座標
	GlyphMetrics metrics;	///< グリフの寸法情報（EM 正規化座標）
	int cell_index;			///< アトラスのセルインデックス（-1 = 輪郭なし）
};

/**
 * MSDF グリフの LRU キャッシュ
 *
 * フォントとアトラスへの参照を保持し、コードポイント → CachedGlyph のマッピングを管理する。
 * LRU リストで使用順序を追跡し、上限超過時に最古のエントリを除去する。
 */
class GlyphCache {
	MsdfFont* font_;		///< グリフ輪郭の取得元フォント（外部所有）
	GlyphAtlas* atlas_;		///< MSDF ビットマップのアップロード先アトラス（外部所有）

	std::unordered_map< uint32_t, CachedGlyph > cache_;	///< コードポイント → キャッシュエントリ
	std::list< uint32_t > lru_;								///< LRU 順序リスト（先頭が最新）

	int msdf_size_;			///< MSDF ビットマップのピクセルサイズ（正方形、アトラスの cell_size と一致させる）
	double msdf_range_;		///< MSDF の距離フィールド範囲（ピクセル単位、シェーダーの px_range と対応）

	/// キャッシュに保持する最大グリフ数（超過分は LRU で除去）
	static constexpr size_t MAX_CACHED_GLYPHS = 2048;

	/**
	 * 指定コードポイントの MSDF グリフを生成してアトラスに登録する
	 *
	 * MsdfFont からグリフ輪郭を取得し、msdfgen で MSDF ビットマップを生成、
	 * アトラスにアップロードする。輪郭のないグリフ（空白等）も advance のみ設定して成功を返す。
	 *
	 * @param codepoint Unicode コードポイント
	 * @param result [out] 生成されたグリフ情報
	 * @return 成功時 true
	 */
	bool generate_glyph( uint32_t codepoint, CachedGlyph& result );

	/**
	 * LRU リスト末尾（最古）のグリフをキャッシュから除去する
	 *
	 * キャッシュマップと LRU リストからエントリを削除する。
	 * アトラスのセル自体は解放せず、後続の add_glyph / replace_glyph で上書きされる。
	 */
	void evict_oldest();

public:
	/**
	 * @param font グリフ輪郭の取得元フォント
	 * @param atlas MSDF ビットマップのアップロード先アトラス
	 * @param msdf_size MSDF ビットマップのピクセルサイズ（デフォルト: 48）
	 * @param msdf_range MSDF の距離フィールド範囲（デフォルト: 4.0 ピクセル）
	 */
	GlyphCache( MsdfFont* font, GlyphAtlas* atlas, int msdf_size = 48, double msdf_range = 4.0 );

	/**
	 * 指定コードポイントのキャッシュ済みグリフを取得する
	 *
	 * キャッシュヒット時は LRU を更新して返す。
	 * キャッシュミス時は MSDF を生成・登録して返す。
	 *
	 * @param codepoint Unicode コードポイント
	 * @return キャッシュ済みグリフへのポインタ（取得失敗時 nullptr）
	 */
	const CachedGlyph* get( uint32_t codepoint );

	/// キャッシュを全クリアする（フォント変更やリセット時に使用）
	void clear();
};

} // namespace core::graphics

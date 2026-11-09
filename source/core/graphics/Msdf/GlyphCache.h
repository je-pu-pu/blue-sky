/**
 * MSDF グリフの LRU キャッシュ
 *
 * Unicode コードポイントをキーとして、MSDF 生成済みグリフを管理する。
 * キャッシュミス時に MsdfFont からグリフ輪郭を取得し、msdfgen で MSDF ビットマップを生成、
 * GlyphAtlas にアップロードしてキャッシュに登録する。
 *
 * アトラスのセルが尽きたら、最も長く使われていない（LRU）グリフを追い出し、そのセルを再利用する。
 * （以前は追い出してもセルが空かず、アトラスが満杯になると新しい文字が描けなくなり、
 *   表示できていた文字まで 1 つずつ消えていった）
 */
#pragma once

#include "MsdfFont.h"
#include "GlyphAtlasStorage.h"
#include <unordered_map>
#include <list>
#include <vector>
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
 *
 * - 使用順は std::list で持ち、各エントリがリスト上の位置を覚える（使うたびの更新が O(1)）。
 * - 追い出したグリフのセルは空きリストに戻し、次のグリフで再利用する。
 * - 1 回の描画（flush）で使う文字の種類がセルの総数（既定 1764）を超えない限り、
 *   同じ描画の途中で使用中のグリフが追い出されることはない（使ったグリフは使用順の先頭にあるため）。
 */
class GlyphCache {
	struct Entry
	{
		CachedGlyph glyph;
		std::list< uint32_t >::iterator lru_position;	///< lru_ 上の位置
	};

	MsdfFont* font_;		///< グリフ輪郭の取得元フォント（外部所有）
	GlyphAtlasStorage* atlas_;		///< MSDF ビットマップのアップロード先アトラス（外部所有）

	std::unordered_map< uint32_t, Entry > cache_;		///< コードポイント → キャッシュエントリ
	std::unordered_map< uint32_t, double > advance_cache_;		///< 計測用の送り幅（MSDF を作っていない文字も）
	std::unordered_map< uint64_t, double > kerning_cache_;		///< カーニング
	std::list< uint32_t > lru_;							///< 使用順（先頭が最新）
	std::vector< int > free_cells_;						///< 追い出しで空いたアトラスのセル

	int msdf_size_;			///< MSDF ビットマップのピクセルサイズ（正方形、アトラスの cell_size と一致させる）
	double msdf_range_;		///< MSDF の距離フィールド範囲（ピクセル単位、シェーダーの px_range と対応）

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
	 * アトラスのセルを 1 つ確保してビットマップを書き込む
	 *
	 * 空きセル → 未使用のセル → 最も古いグリフを追い出して得たセル、の順に使う。
	 *
	 * @return セル番号（確保できなければ -1）
	 */
	int store_bitmap( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv );

	/// 最も古いグリフを 1 つ追い出す。セルを持っていれば空きリストに戻す。追い出せなければ false
	bool evict_oldest();

public:
	/**
	 * @param font グリフ輪郭の取得元フォント
	 * @param atlas MSDF ビットマップのアップロード先アトラス
	 * @param msdf_size MSDF ビットマップのピクセルサイズ（デフォルト: 48）
	 * @param msdf_range MSDF の距離フィールド範囲（デフォルト: 4.0 ピクセル）
	 */
	GlyphCache( MsdfFont* font, GlyphAtlasStorage* atlas, int msdf_size = 48, double msdf_range = 4.0 );

	/**
	 * 指定コードポイントのキャッシュ済みグリフを取得する
	 *
	 * キャッシュヒット時は使用順を更新して返す。
	 * キャッシュミス時は MSDF を生成・登録して返す。
	 *
	 * @param codepoint Unicode コードポイント
	 * @return キャッシュ済みグリフへのポインタ（取得失敗時 nullptr）。後続の get() で追い出されると無効になる
	 */
	const CachedGlyph* get( uint32_t codepoint );

	/**
	 * 送り幅（EM 正規化）だけを得る。MSDF の生成と GPU への転送はしない（文字幅を測るため）
	 *
	 * 以前は文字幅を測るだけで、グリフの MSDF の生成とアトラスへの転送が走っていた。
	 * 文字が無ければ false
	 */
	bool get_advance( uint32_t codepoint, double& advance );

	/// 2 つの文字の間のカーニング（EM 正規化。無ければ 0）
	double get_kerning( uint32_t left, uint32_t right );

	/// キャッシュを全クリアし、アトラスのセルもすべて未使用に戻す（フォント変更やリセット時に使用）
	void clear();

	/// キャッシュしているグリフの数（テスト用）
	size_t size() const { return cache_.size(); }
};

} // namespace core::graphics

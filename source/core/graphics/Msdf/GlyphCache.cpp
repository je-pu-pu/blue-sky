#include "GlyphCache.h"

namespace core::graphics {

/**
 * @note msdf_size はアトラスの cell_size と一致させる必要がある。
 *       不一致の場合、セル内のビットマップがクリップされるか余白が生じる。
 */
GlyphCache::GlyphCache( MsdfFont* font, GlyphAtlasStorage* atlas, int msdf_size, double msdf_range )
	: font_( font )
	, atlas_( atlas )
	, msdf_size_( msdf_size )
	, msdf_range_( msdf_range )
{

}

/**
 * 指定コードポイントのグリフを取得する（キャッシュミス時は自動生成）
 *
 * 処理フロー:
 * 1. キャッシュヒット → 使用順の先頭に移動して返す（リスト上の位置を覚えているので O(1)）
 * 2. キャッシュミス → generate_glyph() で MSDF 生成・アトラス登録（セルが無ければ古いグリフを追い出して再利用）
 *
 * @note 返されるポインタはキャッシュ内部のデータを指す。
 *       次の get() 呼び出しで追い出しが発生すると無効化される可能性がある。
 */
bool GlyphCache::get_advance( uint32_t codepoint, double& advance )
{
	if ( auto it = cache_.find( codepoint ); it != cache_.end() )
	{
		advance = it->second.glyph.metrics.advance;
		return true;
	}

	if ( auto it = advance_cache_.find( codepoint ); it != advance_cache_.end() )
	{
		advance = it->second;
		return true;
	}

	GlyphMetrics metrics;

	if ( ! font_->get_glyph_metrics( codepoint, metrics ) )
	{
		return false;
	}

	advance_cache_[ codepoint ] = metrics.advance;
	advance = metrics.advance;
	return true;
}

double GlyphCache::get_kerning( uint32_t left, uint32_t right )
{
	const uint64_t key = ( static_cast< uint64_t >( left ) << 32 ) | right;

	if ( auto it = kerning_cache_.find( key ); it != kerning_cache_.end() )
	{
		return it->second;
	}

	double kerning = 0.0;

	if ( ! font_->get_kerning( left, right, kerning ) )
	{
		kerning = 0.0;
	}

	kerning_cache_[ key ] = kerning;
	return kerning;
}

const CachedGlyph* GlyphCache::get( uint32_t codepoint )
{
	auto it = cache_.find( codepoint );

	if ( it != cache_.end() )
	{
		// キャッシュヒット: 使用順の先頭へ移す
		lru_.splice( lru_.begin(), lru_, it->second.lru_position );

		return & it->second.glyph;
	}

	// キャッシュミス: MSDF ビットマップを生成してアトラスに登録
	CachedGlyph glyph;

	if ( ! generate_glyph( codepoint, glyph ) )
	{
		return nullptr;
	}

	lru_.push_front( codepoint );

	Entry& entry = cache_[ codepoint ];
	entry.glyph = glyph;
	entry.lru_position = lru_.begin();

	return & entry.glyph;
}

/**
 * 指定コードポイントの MSDF ビットマップを生成してアトラスに登録する
 *
 * 処理フロー:
 * 1. MsdfFont からグリフ輪郭 (Shape) と advance を取得
 * 2. 輪郭がない場合（空白文字等）→ advance のみ設定して成功を返す
 * 3. 輪郭を正規化・色分けし、バウンディングボックスからスケールを計算
 * 4. msdfgen::generateMSDF() で MSDF ビットマップを生成
 * 5. アトラスにアップロード（満杯時は古いグリフを追い出してセルを再利用）
 *
 * --- MSDF ビットマップのレイアウト ---
 *
 *   +-------------------------------------------+
 *   |  padding (msdf_range_ px)                 |
 *   |  +-------------------------------------+  |
 *   |  |                                     |  |
 *   |  |  グリフ輪郭（available x available） |  |
 *   |  |  スケール = available / max_dim      |  |
 *   |  |                                     |  |
 *   |  +-------------------------------------+  |
 *   |  padding (msdf_range_ px)                 |
 *   +-------------------------------------------+
 *          msdf_size_ x msdf_size_ px
 *
 * スケール計算の考え方:
 * - available = msdf_size_ - 2 * padding で、グリフ本体に使えるピクセル数
 * - scale = available / max(max_dim, 1.0) で EM 座標→ピクセルに変換
 * - max_dim >= 1.0 EM のグリフはスケールダウンされる
 * - max_dim < 1.0 EM のグリフもスケールは同一（アウトライン太さの均一性のため）
 *   → max(max_dim, 1.0) の 1.0 がこれを保証する
 *
 * @param codepoint Unicode コードポイント
 * @param result [out] 生成されたグリフ情報（UV、メトリクス、セルインデックス）
 * @return 成功時 true（アトラスが完全に満杯で登録不能な場合のみ false）
 */
bool GlyphCache::generate_glyph( uint32_t codepoint, CachedGlyph& result )
{
	msdfgen::Shape shape;
	double advance = 0;

	if ( ! font_->get_glyph_shape( codepoint, shape, advance ) )
	{
		return false;
	}

	result.metrics.advance = advance;

	if ( shape.contours.empty() )
	{
		// 空白文字など輪郭のないグリフ: ビットマップ不要、advance のみ有効
		result.metrics.bearing_x = 0;
		result.metrics.bearing_y = 0;
		result.metrics.width = 0;
		result.metrics.height = 0;
		result.uv = {};
		result.cell_index = -1;

		return true;
	}

	// コンターの向きを正規化（内側/外側の判定に必要）
	shape.normalize();

	// エッジの色分け: MSDF は R/G/B チャンネルに異なるエッジ情報を格納する
	// 角度閾値 3.0 で隣接エッジに異なる色を割り当てる
	msdfgen::edgeColoringSimple( shape, 3.0 );

	// シェイプの EM 座標系でのバウンディングボックスを取得
	double left = 1e240, bottom = 1e240, right = -1e240, top = -1e240;
	shape.bound( left, bottom, right, top );

	double glyph_width = right - left;
	double glyph_height = top - bottom;

	// パディング: 距離フィールドがグリフ外側にも広がるため、その分の余白を確保
	double padding = msdf_range_;
	double available = msdf_size_ - 2.0 * padding;

	// 統一スケール: 全グリフで同じ texel-to-screen 比率を確保する
	// max(max_dim, 1.0) により、1 EM 未満のグリフもスケールが統一され
	// アウトラインの太さが文字サイズによらず一定になる
	double max_dim = std::max( glyph_width, glyph_height );
	double scale = available / std::max( max_dim, 1.0 );

	// frame_scale: EM 座標 → ビットマップピクセルへの変換スケール
	msdfgen::Vector2 frame_scale( scale, scale );

	// frame_translate: グリフをビットマップ中央に配置するための平行移動
	// padding/scale で左下にパディング分の余白を確保し、
	// (available/scale - glyph_dim) * 0.5 で中央寄せする
	msdfgen::Vector2 frame_translate(
		padding / scale - left + ( available / scale - glyph_width ) * 0.5,
		padding / scale - bottom + ( available / scale - glyph_height ) * 0.5
	);

	// EM 正規化座標でのメトリクスを保存（描画時のクワッド配置に使用）
	result.metrics.bearing_x = left;
	result.metrics.bearing_y = top;
	result.metrics.width = glyph_width;
	result.metrics.height = glyph_height;

	// MSDF ビットマップ生成（3ch float: R, G, B に各エッジの距離値）
	msdfgen::Bitmap< float, 3 > msdf( msdf_size_, msdf_size_ );

	// Range はシェイプ空間（EM 座標）の単位で指定する
	// ピクセル範囲 msdf_range_ をスケールで割って EM 空間に変換
	// 2 倍にするのは、パディング全体（±msdf_range_ ピクセル）を有効距離として使うため
	msdfgen::generateMSDF( msdf, shape, msdfgen::Range( 2.0 * msdf_range_ / scale ), frame_scale, frame_translate );

	// 生成した MSDF ビットマップをアトラスのセルにアップロード
	result.cell_index = store_bitmap( msdf, result.uv );

	return result.cell_index >= 0;
}

int GlyphCache::store_bitmap( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv )
{
	// 1. 追い出しで空いたセル
	if ( free_cells_.empty() )
	{
		// 2. まだ一度も使っていないセル
		const int cell = atlas_->add_glyph( bitmap, uv );

		if ( cell >= 0 )
		{
			return cell;
		}

		// 3. アトラスが満杯 : セルを持つグリフが追い出されるまで古い順に追い出す
		while ( free_cells_.empty() )
		{
			if ( ! evict_oldest() )
			{
				return -1;
			}
		}
	}

	const int cell = free_cells_.back();
	free_cells_.pop_back();

	atlas_->replace_glyph( cell, bitmap, uv );

	return cell;
}

/**
 * @brief 使用順の末尾（最古）のグリフをキャッシュから除去する
 *
 * グリフがアトラスのセルを持っていれば、そのセルを空きリストに戻す（次のグリフが再利用する）。
 */
bool GlyphCache::evict_oldest()
{
	if ( lru_.empty() )
	{
		return false;
	}

	const uint32_t oldest = lru_.back();
	lru_.pop_back();

	auto it = cache_.find( oldest );

	if ( it != cache_.end() )
	{
		if ( it->second.glyph.cell_index >= 0 )
		{
			free_cells_.push_back( it->second.glyph.cell_index );
		}

		cache_.erase( it );
	}

	return true;
}

/**
 * @brief キャッシュを全クリアする
 *
 * フォント変更やシーンリセット時に呼び出す。
 * アトラスのテクスチャデータは消さない（次の書き込みで上書きされる）。
 */
void GlyphCache::clear()
{
	cache_.clear();
	lru_.clear();
	free_cells_.clear();

	// セルもすべて未使用に戻す（以前はキャッシュだけ消してセルが減り続け、フォントを数回読み直すと満杯になった）
	atlas_->reset();
}

} // namespace core::graphics

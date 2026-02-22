#include "GlyphCache.h"

namespace core::graphics {

GlyphCache::GlyphCache( MsdfFont* font, GlyphAtlas* atlas, int msdf_size, double msdf_range )
	: font_( font )
	, atlas_( atlas )
	, msdf_size_( msdf_size )
	, msdf_range_( msdf_range )
{

}

const CachedGlyph* GlyphCache::get( uint32_t codepoint )
{
	auto it = cache_.find( codepoint );

	if ( it != cache_.end() )
	{
		// LRU 更新: 先頭に移動
		lru_.remove( codepoint );
		lru_.push_front( codepoint );

		return &it->second;
	}

	// キャッシュミス: MSDF 生成
	CachedGlyph glyph;

	if ( ! generate_glyph( codepoint, glyph ) )
	{
		return nullptr;
	}

	cache_[ codepoint ] = glyph;
	lru_.push_front( codepoint );

	// キャッシュ上限を超えたら古いものを除去
	while ( lru_.size() > MAX_CACHED_GLYPHS )
	{
		evict_oldest();
	}

	return &cache_[ codepoint ];
}

bool GlyphCache::generate_glyph( uint32_t codepoint, CachedGlyph& result )
{
	msdfgen::Shape shape;
	double advance = 0;

	if ( ! font_->get_glyph_shape( codepoint, shape, advance ) )
	{
		return false;
	}

	// メトリクス取得
	result.metrics.advance = advance;

	if ( shape.contours.empty() )
	{
		// 空白文字など輪郭のないグリフ
		result.metrics.bearing_x = 0;
		result.metrics.bearing_y = 0;
		result.metrics.width = 0;
		result.metrics.height = 0;
		result.uv = {};
		result.cell_index = -1;

		return true;
	}

	// コンターの向きを正規化（内外判定に必要）
	shape.normalize();

	// エッジの色分け
	msdfgen::edgeColoringSimple( shape, 3.0 );

	// シェイプの境界を取得してプロジェクションを計算
	double left = 1e240, bottom = 1e240, right = -1e240, top = -1e240;
	shape.bound( left, bottom, right, top );

	double glyph_width = right - left;
	double glyph_height = top - bottom;

	// パディング（距離フィールドの範囲分）
	double padding = msdf_range_;
	double available = msdf_size_ - 2.0 * padding;

	// グリフをセル内に収まるようにスケーリング
	double scale = 1.0;
	if ( glyph_width > 0 && glyph_height > 0 )
	{
		scale = available / std::max( glyph_width, glyph_height );
	}

	msdfgen::Vector2 frame_scale( scale, scale );
	msdfgen::Vector2 frame_translate(
		padding / scale - left + ( available / scale - glyph_width ) * 0.5,
		padding / scale - bottom + ( available / scale - glyph_height ) * 0.5
	);

	// EM 正規化座標でのメトリクスを保存
	result.metrics.bearing_x = left;
	result.metrics.bearing_y = top;
	result.metrics.width = glyph_width;
	result.metrics.height = glyph_height;

	// MSDF 生成
	msdfgen::Bitmap< float, 3 > msdf( msdf_size_, msdf_size_ );
	// Range はシェイプ空間の単位なので、ピクセル範囲をスケールで変換
	// パディング全体（±msdf_range_ ピクセル）を有効に使うため 2 倍にする
	msdfgen::generateMSDF( msdf, shape, msdfgen::Range( 2.0 * msdf_range_ / scale ), frame_scale, frame_translate );

	// アトラスに配置
	result.cell_index = atlas_->add_glyph( msdf, result.uv );

	if ( result.cell_index < 0 )
	{
		// アトラスが満杯: LRU で古いセルを再利用
		evict_oldest();
		result.cell_index = atlas_->add_glyph( msdf, result.uv );
	}

	return result.cell_index >= 0;
}

void GlyphCache::evict_oldest()
{
	if ( lru_.empty() )
	{
		return;
	}

	uint32_t oldest = lru_.back();
	lru_.pop_back();
	cache_.erase( oldest );

	// next_cell_index はリセットしない（replace_glyph で上書き時に対応）
}

void GlyphCache::clear()
{
	cache_.clear();
	lru_.clear();
}

} // namespace core::graphics

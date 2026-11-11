#pragma once

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
 * グリフの MSDF ビットマップを入れる先（抽象）
 *
 * GlyphCache はこれだけを使う。実装（テクスチャへのアップロード）は描画の API ごとに用意する
 * （Direct3D 11 は direct_3d_11 側の GlyphAtlas）。
 * 以前は GlyphCache が Direct3D 11 のアトラスに直接依存していて、MSDF のフォルダ全体が API に縛られていた（設計上の問題 B19）。
 */
class GlyphAtlasStorage {
public:
	virtual ~GlyphAtlasStorage() = default;

	/// 次の空きセルにグリフを入れる。セルの番号を返す（満杯なら -1）
	virtual int add_glyph( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv ) = 0;

	/// 既存のセルを上書きする（使われなくなったグリフのセルを再利用する）
	virtual void replace_glyph( int cell_index, const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv ) = 0;

	/// すべてのセルを空きに戻す
	virtual void reset() = 0;
};

} // namespace core::graphics

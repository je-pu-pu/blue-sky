/**
 * FreeType + msdfgen を使って TrueType フォントを読み込み、
 * グリフのシェイプ（輪郭）やメトリクス（寸法情報）を取得するクラス。
 *
 * 座標系は EM 正規化 (FONT_SCALING_EM_NORMALIZED) を使用しており、
 * 1 EM = 1.0 の座標空間でグリフデータが返される。
 * 実際のピクセルサイズへの変換は描画側 (MsdfTextRenderer) で行う。
 */
#pragma once

#include <vector>
#include <cstdint>

#define MSDFGEN_PUBLIC
#define MSDFGEN_EXT_PUBLIC
#include <msdfgen/msdfgen.h>
#include <msdfgen/ext/import-font.h>

namespace core::graphics {

/**
 * グリフの寸法情報（EM 正規化座標）
 *
 * advance   : グリフ描画後にカーソルを進める量
 * bearing_x : グリフ原点から左端までのオフセット（X）
 * bearing_y : ベースラインからグリフ上端までのオフセット（Y）
 * width     : グリフのバウンディングボックス幅
 * height    : グリフのバウンディングボックス高さ
 */
struct GlyphMetrics {
	double advance;
	double bearing_x;
	double bearing_y;
	double width;
	double height;
};

/**
 * FreeType + msdfgen フォントラッパー
 *
 * FreeType ライブラリとフォントハンドルの寿命を管理し、
 * グリフの輪郭データ (Shape) やメトリクスを取得するインターフェースを提供する。
 */
class MsdfFont {
	msdfgen::FreetypeHandle* ft_;			///< FreeType ライブラリハンドル
	msdfgen::FontHandle* font_;				///< フォントファイルハンドル
	std::vector< msdfgen::FontHandle* > fallbacks_;	///< 代わりのフォント

	/// その文字を持つフォント（無ければ主のフォント）
	msdfgen::FontHandle* font_for( uint32_t codepoint ) const;
	msdfgen::FontMetrics font_metrics_;		///< フォント全体のメトリクス (emSize, lineHeight, ascenderY 等)

public:
	MsdfFont();
	~MsdfFont();

	bool load(const char* font_path);

	/**
	 * 代わりのフォントを足す（主のフォントに無い文字を、足した順に探す）
	 *
	 * 以前はフォントが 1 種類だけで、その言語の文字が無いと豆腐になった。
	 */
	bool add_fallback(const char* font_path);

	/// どれかのフォントにその文字があるか
	bool has_glyph(uint32_t codepoint) const;
	void unload();

	bool get_glyph_shape(uint32_t codepoint, msdfgen::Shape& shape, double& advance) const;
	bool get_glyph_metrics(uint32_t codepoint, GlyphMetrics& metrics) const;
	bool get_kerning(uint32_t left, uint32_t right, double& kerning) const;

	const msdfgen::FontMetrics& get_font_metrics() const { return font_metrics_; }
	bool is_loaded() const { return font_ != nullptr; }
};

} // namespace core::graphics

#include "MsdfFont.h"

namespace core::graphics {

MsdfFont::MsdfFont()
	: ft_( nullptr )
	, font_( nullptr )
	, font_metrics_{}
{

}

MsdfFont::~MsdfFont()
{
	unload();
}

/**
 * フォントファイルを読み込む
 *
 * FreeType を初期化し、指定パスの TrueType / OpenType フォントをロードする。
 * フォントメトリクスは EM 正規化座標 (1 EM = 1.0) で取得される。
 *
 * @param font_path フォントファイルのパス (.ttf / .otf)
 * @return 読み込みに成功した場合 true
 */
bool MsdfFont::load( const char* font_path )
{
	unload();

	ft_ = msdfgen::initializeFreetype();
	if ( ! ft_ )
	{
		return false;
	}

	font_ = msdfgen::loadFont( ft_, font_path );
	if ( ! font_ )
	{
		msdfgen::deinitializeFreetype( ft_ );
		ft_ = nullptr;
		return false;
	}

	// EM 正規化座標でフォントメトリクスを取得
	// emSize=1.0 として lineHeight, ascenderY 等が返る
	msdfgen::getFontMetrics( font_metrics_, font_, msdfgen::FONT_SCALING_EM_NORMALIZED );

	return true;
}

void MsdfFont::unload()
{
	if ( font_ )
	{
		msdfgen::destroyFont( font_ );
		font_ = nullptr;
	}

	if ( ft_ )
	{
		msdfgen::deinitializeFreetype( ft_ );
		ft_ = nullptr;
	}
}

/**
 * 指定コードポイントのグリフ輪郭を取得する
 *
 * MSDF 生成に必要な Shape (コンター群) と advance (送り幅) を返す。
 * 座標は EM 正規化。
 *
 * @param codepoint Unicode コードポイント
 * @param shape [out] グリフの輪郭 (コンター群)
 * @param advance [out] グリフの送り幅 (EM 正規化)
 * @return 取得成功時 true
 */
bool MsdfFont::get_glyph_shape( uint32_t codepoint, msdfgen::Shape& shape, double& advance ) const
{
	if ( ! font_ )
	{
		return false;
	}

	return msdfgen::loadGlyph( shape, font_, static_cast< msdfgen::unicode_t >( codepoint ), msdfgen::FONT_SCALING_EM_NORMALIZED, &advance );
}

/**
 * 指定コードポイントのグリフメトリクスを取得する
 *
 * Shape を読み込んでバウンディングボックスから bearing, width, height を計算する。
 * 空白文字など輪郭のないグリフは width=0, height=0 で返す。
 *
 * @param codepoint Unicode コードポイント
 * @param metrics [out] グリフメトリクス (EM 正規化)
 * @return 取得成功時 true
 */
bool MsdfFont::get_glyph_metrics( uint32_t codepoint, GlyphMetrics& metrics ) const
{
	if ( ! font_ )
	{
		return false;
	}

	msdfgen::Shape shape;
	double advance = 0;

	if ( ! msdfgen::loadGlyph( shape, font_, static_cast< msdfgen::unicode_t >( codepoint ), msdfgen::FONT_SCALING_EM_NORMALIZED, &advance ) )
	{
		return false;
	}

	metrics.advance = advance;

	if ( shape.contours.empty() )
	{
		// 空白文字など輪郭のないグリフ
		metrics.bearing_x = 0;
		metrics.bearing_y = 0;
		metrics.width = 0;
		metrics.height = 0;
	}
	else
	{
		double left = 0, bottom = 0, right = 0, top = 0;
		shape.bound( left, bottom, right, top );

		metrics.bearing_x = left;
		metrics.bearing_y = top;
		metrics.width = right - left;
		metrics.height = top - bottom;
	}

	return true;
}

/**
 * 2 文字間のカーニング値を取得する
 *
 * @param left 左側の文字のコードポイント
 * @param right 右側の文字のコードポイント
 * @param kerning [out] カーニング値 (EM 正規化)
 * @return 取得成功時 true
 */
bool MsdfFont::get_kerning( uint32_t left, uint32_t right, double& kerning ) const
{
	if ( ! font_ )
	{
		return false;
	}

	return msdfgen::getKerning( kerning, font_, static_cast< msdfgen::unicode_t >( left ), static_cast< msdfgen::unicode_t >( right ), msdfgen::FONT_SCALING_EM_NORMALIZED );
}

} // namespace core::graphics

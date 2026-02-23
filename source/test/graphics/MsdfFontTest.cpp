#include "pch.h"
#include <core/graphics/Msdf/MsdfFont.h>
#include <filesystem>

namespace {

// __FILE__ はソースファイルの絶対パスを返す (MSVC デフォルト)
// test/graphics/MsdfFontTest.cpp → ../../media/font/ で source/ 基準に解決
std::string get_font_path()
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media/font/rounded-mplus-1p-regular.ttf" ).lexically_normal().string();
}

#define FONT_PATH get_font_path().c_str()

} // namespace

TEST( MsdfFontTest, LoadValidFont )
{
	core::graphics::MsdfFont font;
	EXPECT_TRUE( font.load( FONT_PATH ) );
	EXPECT_TRUE( font.is_loaded() );
}

TEST( MsdfFontTest, LoadInvalidFont )
{
	core::graphics::MsdfFont font;
	EXPECT_FALSE( font.load( "nonexistent/path/font.ttf" ) );
	EXPECT_FALSE( font.is_loaded() );
}

TEST( MsdfFontTest, Unload )
{
	core::graphics::MsdfFont font;
	font.load( FONT_PATH );
	font.unload();
	EXPECT_FALSE( font.is_loaded() );
}

TEST( MsdfFontTest, FontMetrics )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	const auto& m = font.get_font_metrics();

	// EM 正規化なので emSize ≈ 1.0
	EXPECT_NEAR( m.emSize, 1.0, 0.01 );
	EXPECT_GT( m.lineHeight, 0.0 );
	EXPECT_GT( m.ascenderY, 0.0 );
}

TEST( MsdfFontTest, GlyphMetricsAscii )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	core::graphics::GlyphMetrics metrics{};
	ASSERT_TRUE( font.get_glyph_metrics( U'A', metrics ) );

	EXPECT_GT( metrics.advance, 0.0 );
	EXPECT_GT( metrics.width, 0.0 );
	EXPECT_GT( metrics.height, 0.0 );
}

TEST( MsdfFontTest, GlyphMetricsSpace )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	core::graphics::GlyphMetrics metrics{};
	ASSERT_TRUE( font.get_glyph_metrics( U' ', metrics ) );

	EXPECT_GT( metrics.advance, 0.0 );
	EXPECT_DOUBLE_EQ( metrics.width, 0.0 );
	EXPECT_DOUBLE_EQ( metrics.height, 0.0 );
}

TEST( MsdfFontTest, GlyphMetricsCjk )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	core::graphics::GlyphMetrics metrics{};
	ASSERT_TRUE( font.get_glyph_metrics( 0x65E5, metrics ) ); // '日'

	EXPECT_GT( metrics.advance, 0.0 );
	EXPECT_GT( metrics.width, 0.0 );
}

TEST( MsdfFontTest, GlyphShape )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	msdfgen::Shape shape;
	double advance = 0;
	ASSERT_TRUE( font.get_glyph_shape( U'A', shape, advance ) );

	EXPECT_GT( advance, 0.0 );
}

TEST( MsdfFontTest, Kerning )
{
	core::graphics::MsdfFont font;
	ASSERT_TRUE( font.load( FONT_PATH ) );

	double kerning = 0;
	EXPECT_TRUE( font.get_kerning( U'A', U'V', kerning ) );
}

TEST( MsdfFontTest, UnloadedFontReturnsFailure )
{
	core::graphics::MsdfFont font;

	core::graphics::GlyphMetrics metrics{};
	EXPECT_FALSE( font.get_glyph_metrics( U'A', metrics ) );

	msdfgen::Shape shape;
	double advance = 0;
	EXPECT_FALSE( font.get_glyph_shape( U'A', shape, advance ) );

	double kerning = 0;
	EXPECT_FALSE( font.get_kerning( U'A', U'V', kerning ) );
}

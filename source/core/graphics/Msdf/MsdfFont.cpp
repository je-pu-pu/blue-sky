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

bool MsdfFont::get_glyph_shape( uint32_t codepoint, msdfgen::Shape& shape, double& advance ) const
{
	if ( ! font_ )
	{
		return false;
	}

	return msdfgen::loadGlyph( shape, font_, static_cast< msdfgen::unicode_t >( codepoint ), msdfgen::FONT_SCALING_EM_NORMALIZED, &advance );
}

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

bool MsdfFont::get_kerning( uint32_t left, uint32_t right, double& kerning ) const
{
	if ( ! font_ )
	{
		return false;
	}

	return msdfgen::getKerning( kerning, font_, static_cast< msdfgen::unicode_t >( left ), static_cast< msdfgen::unicode_t >( right ), msdfgen::FONT_SCALING_EM_NORMALIZED );
}

} // namespace core::graphics

#include "TextFit.h"
#include "Renderer.h"

namespace core::ui
{

namespace
{

/// UTF-8 の末尾の 1 文字（コードポイント）を消す
void pop_codepoint( string_t& s )
{
	if ( s.empty() )
	{
		return;
	}

	size_t i = s.size() - 1;

	while ( i > 0 && ( static_cast< unsigned char >( s[ i ] ) & 0xC0 ) == 0x80 )
	{
		i--;
	}

	s.erase( i );
}

} // namespace

void fit_text_to_width( const Renderer& renderer, float_t width, string_t& text, core::graphics::TextStyle& style )
{
	if ( text.empty() || width <= 0.f )
	{
		return;
	}

	const float_t text_width = renderer.measure_text_width( text.c_str(), style.font_size );

	if ( text_width <= width )
	{
		return;
	}

	// 収まるまで小さくする（下限まで）
	const float_t scale = width / text_width;

	if ( scale >= MIN_TEXT_FIT_SCALE )
	{
		style.font_size *= scale;
		return;
	}

	// 下限まで小さくしても収まらない : 末尾を「…」にして省略する
	style.font_size *= MIN_TEXT_FIT_SCALE;

	const string_t ellipsis = "…";

	while ( ! text.empty() && renderer.measure_text_width( ( text + ellipsis ).c_str(), style.font_size ) > width )
	{
		pop_codepoint( text );
	}

	text += ellipsis;
}

float_t text_row_height( const Renderer& renderer, float_t min_height, float_t font_size, float_t padding )
{
	const float_t text_height = renderer.measure_line_height( font_size ) + padding;
	return min_height > text_height ? min_height : text_height;
}

void draw_text_fit( Renderer& renderer, const Rect& rect, string_t text, core::graphics::TextStyle style )
{
	fit_text_to_width( renderer, rect.w, text, style );
	renderer.draw_text( rect, text.c_str(), style );
}

} // namespace core::ui

#include "Label.h"
#include "TextFit.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

namespace
{

/// UTF-8 の先頭バイトから、そのコードポイントのバイト数を返す
size_t utf8_length( unsigned char c )
{
	if ( c < 0x80 ) return 1;
	if ( ( c >> 5 ) == 0x6 ) return 2;
	if ( ( c >> 4 ) == 0xE ) return 3;
	if ( ( c >> 3 ) == 0x1E ) return 4;
	return 1; // 不正なバイトは 1 バイトとして進める
}

} // namespace

float_t Label::get_font_size( const UIContext& ctx ) const
{
	switch ( style_ )
	{
		case Style::TITLE: return ctx.theme.font_size_title;
		case Style::SMALL: return ctx.theme.font_size_small;
		default: return ctx.theme.font_size_normal;
	}
}

Color Label::get_color( const UIContext& ctx ) const
{
	switch ( style_ )
	{
		case Style::HEADER: return ctx.theme.header;
		case Style::SMALL: return ctx.theme.text_dim;
		default: return ctx.theme.text;
	}
}

string_t Label::get_text( const UIContext& ctx ) const
{
	return provider_ ? provider_() : ctx.tr( text_key_ );
}

float_t Label::measure_width( const UIContext& ctx ) const
{
	return ctx.renderer.measure_text_width( get_text( ctx ).c_str(), get_font_size( ctx ) );
}

void Label::update( float_t dt, UIContext& ctx )
{
	if ( provider_ )
	{
		string_t now = provider_();

		if ( now != last_provided_ )
		{
			last_provided_ = std::move( now );
			ctx.screen.invalidate_layout();
		}
	}

	Widget::update( dt, ctx );
}

namespace
{

/// 行の始めに置かない文字（句読点・閉じ括弧・小書きの仮名・長音など）
bool is_no_line_start( const string_t& ch )
{
	static const char_t* const LIST[] = {
		"、", "。", "，", "．", "・", "：", "；", "？", "！", "ー", "）", "」", "』", "】", "〕", "〉", "》", "｝", "〟", "’", "”",
		"ぁ", "ぃ", "ぅ", "ぇ", "ぉ", "っ", "ゃ", "ゅ", "ょ", "ゎ", "ァ", "ィ", "ゥ", "ェ", "ォ", "ッ", "ャ", "ュ", "ョ", "ヮ", "ヵ", "ヶ", "々", "…",
		",", ".", "!", "?", ":", ";", ")", "]", "}",
	};

	for ( const char_t* c : LIST ) { if ( ch == c ) return true; }
	return false;
}

/// 行の終わりに置かない文字（開き括弧）
bool is_no_line_end( const string_t& ch )
{
	static const char_t* const LIST[] = { "（", "「", "『", "【", "〔", "〈", "《", "｛", "〝", "‘", "“", "(", "[", "{" };

	for ( const char_t* c : LIST ) { if ( ch == c ) return true; }
	return false;
}

/// UTF-8 の文字列の最後の 1 文字の始まりの位置
size_t last_codepoint_start( const string_t& s )
{
	size_t i = s.size();

	while ( i > 0 )
	{
		i--;

		if ( ( static_cast< unsigned char >( s[ i ] ) & 0xC0 ) != 0x80 )
		{
			break;
		}
	}

	return i;
}

} // namespace

std::vector< string_t > Label::wrap_lines( const string_t& text, float_t width, float_t font_size, const Renderer& renderer )
{
	std::vector< string_t > lines;
	string_t line;

	for ( size_t i = 0; i < text.size(); )
	{
		if ( text[ i ] == '\n' )
		{
			lines.push_back( line );
			line.clear();
			i++;
			continue;
		}

		const size_t len = std::min( utf8_length( static_cast< unsigned char >( text[ i ] ) ), text.size() - i );
		const string_t ch = text.substr( i, len );
		const string_t candidate = line + ch;

		if ( ! line.empty() && width > 0.f && renderer.measure_text_width( candidate.c_str(), font_size ) > width )
		{
			// 禁則処理 : 行の始めに置けない文字（句読点など）や、行の終わりに置けない文字（開き括弧）が来たら、
			// 前の行の最後の 1 文字を次の行へ送る（前の行が 1 文字だけなら送らない）。以前は禁則処理が無かった
			const size_t last = last_codepoint_start( line );
			const string_t last_ch = line.substr( last );

			if ( last > 0 && ( is_no_line_start( ch ) || is_no_line_end( last_ch ) ) )
			{
				lines.push_back( line.substr( 0, last ) );
				line = last_ch + ch;
			}
			else
			{
				lines.push_back( line );
				line = ch;
			}
		}
		else
		{
			line = candidate;
		}

		i += len;
	}

	lines.push_back( line );

	return lines;
}

float_t Label::measure_height( float_t width, const UIContext& ctx ) const
{
	const float_t font_size = get_font_size( ctx );
	const float_t line_h = ctx.renderer.measure_line_height( font_size );
	const string_t text = get_text( ctx );

	size_t lines = 1;

	if ( wrap_ )
	{
		lines = wrap_lines( text, width, font_size, ctx.renderer ).size();
	}
	else
	{
		for ( char_t c : text ) if ( c == '\n' ) lines++;
	}

	return line_h * static_cast< float_t >( lines );
}

void Label::render( Renderer& renderer, const UIContext& ctx ) const
{
	const float_t font_size = get_font_size( ctx );
	const string_t text = get_text( ctx );
	const auto style = make_text_style( get_color( ctx ), font_size, align_, core::graphics::VAlign::TOP );

	if ( ! wrap_ )
	{
		draw_text_fit( renderer, rect_, text, style );
		return;
	}

	const float_t line_h = renderer.measure_line_height( font_size );
	float_t y = rect_.y;

	for ( const auto& line : wrap_lines( text, rect_.w, font_size, renderer ) )
	{
		draw_text_fit( renderer, Rect( rect_.x, y, rect_.w, line_h ), line, style );
		y += line_h;
	}
}

AccessibleInfo Label::describe( const UIContext& ctx ) const
{
	return AccessibleInfo{ "label", get_text( ctx ), string_t() };
}

} // namespace core::ui

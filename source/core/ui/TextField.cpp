#include "TextField.h"
#include "TextFit.h"
#include "Brush.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <cmath>

namespace core::ui
{

size_t TextField::count_codepoints( const string_t& s )
{
	size_t n = 0;

	for ( unsigned char c : s )
	{
		if ( ( c & 0xC0 ) != 0x80 )
		{
			n++;
		}
	}

	return n;
}

string_t TextField::erase_last_codepoint( const string_t& s )
{
	if ( s.empty() )
	{
		return s;
	}

	size_t i = s.size() - 1;

	// 継続バイト（10xxxxxx）をさかのぼって、最後の文字の先頭まで消す
	while ( i > 0 && ( static_cast< unsigned char >( s[ i ] ) & 0xC0 ) == 0x80 )
	{
		i--;
	}

	return s.substr( 0, i );
}

string_t TextField::to_utf8( char32_t c )
{
	string_t out;

	if ( c < 0x80 )
	{
		out.push_back( static_cast< char_t >( c ) );
	}
	else if ( c < 0x800 )
	{
		out.push_back( static_cast< char_t >( 0xC0 | ( c >> 6 ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( c & 0x3F ) ) );
	}
	else if ( c < 0x10000 )
	{
		if ( c >= 0xD800 && c <= 0xDFFF )
		{
			return string_t();		// サロゲートは文字ではない
		}

		out.push_back( static_cast< char_t >( 0xE0 | ( c >> 12 ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( ( c >> 6 ) & 0x3F ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( c & 0x3F ) ) );
	}
	else if ( c < 0x110000 )
	{
		out.push_back( static_cast< char_t >( 0xF0 | ( c >> 18 ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( ( c >> 12 ) & 0x3F ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( ( c >> 6 ) & 0x3F ) ) );
		out.push_back( static_cast< char_t >( 0x80 | ( c & 0x3F ) ) );
	}

	return out;
}

bool TextField::on_text( const std::u32string& text, UIContext& ctx )
{
	if ( ! setter_ || ! is_enabled() )
	{
		return false;
	}

	string_t value = get_value();
	const string_t before = value;

	for ( char32_t c : text )
	{
		if ( c == U'\b' )
		{
			value = erase_last_codepoint( value );
		}
		else if ( c >= 0x20 && c != 0x7F && count_codepoints( value ) < max_length_ )
		{
			// 制御文字（Enter・Tab・Esc など）は入れない。Enter は決定（on_confirm）で扱う
			value += to_utf8( c );
		}
	}

	if ( value != before )
	{
		setter_( value );
		blink_time_ = 0.f;		// 入力した直後はカーソルを見せる
		ctx.screen.play( UISoundEvent::CHANGE );
	}

	return true;
}

void TextField::on_composition( const std::u32string& text, UIContext& )
{
	string_t utf8;

	for ( char32_t c : text )
	{
		utf8 += to_utf8( c );
	}

	if ( utf8 != composition_ )
	{
		composition_ = std::move( utf8 );
		blink_time_ = 0.f;
	}
}

bool TextField::on_confirm( UIContext& ctx )
{
	if ( on_submit_ && is_enabled() )
	{
		ctx.screen.play( UISoundEvent::CONFIRM );
		auto f = on_submit_;	// コールバックの中で自分が破棄されても安全なようにコピー
		f();
	}

	return true;
}

void TextField::update( float_t dt, UIContext& ctx )
{
	blink_time_ = is_focused() ? blink_time_ + dt : 0.f;

	if ( ! is_focused() )
	{
		composition_.clear();		// フォーカスが外れたら変換中の表示も消す
	}
	LabeledRow::update( dt, ctx );
}

void TextField::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_row( renderer, ctx );

	const auto& t = ctx.theme;
	const Rect box = value_rect_.inset( 0.f, rect_.h * t.value_box_inset );
	draw_brush( renderer, box, is_focused() ? t.row_pressed : t.track );

	// 文字と、IME で変換中の文字と、フォーカス中は末尾のカーソル（0.5 秒ごとに点滅）
	const string_t value = get_value();
	string_t shown = value + composition_;

	if ( is_focused() && std::fmod( blink_time_, 1.f ) < 0.5f )
	{
		shown += "|";
	}

	const Rect area = box.inset( t.row_padding_x, 0.f );
	string_t fitted = shown;
	auto style = make_text_style( value_color( ctx ), t.font_size_normal );
	fit_text_to_width( renderer, area.w, fitted, style );
	renderer.draw_text( area, fitted.c_str(), style );

	// 変換中の文字に下線（省略されていないときだけ。位置がずれるため）
	if ( ! composition_.empty() && fitted == shown )
	{
		const float_t x = area.x + renderer.measure_text_width( value.c_str(), style.font_size );
		const float_t w = renderer.measure_text_width( composition_.c_str(), style.font_size );
		const float_t line_h = renderer.measure_line_height( style.font_size );
		const float_t y = area.y + ( area.h + line_h ) * 0.5f - t.accent_bar_thickness * 0.5f;

		draw_brush( renderer, Rect( x, y, w, std::max( 1.f, t.accent_bar_thickness * 0.5f ) ), t.accent );
	}
}

void TextField::collect_hints( std::vector< Hint >& hints ) const
{
	if ( on_submit_ )
	{
		hints.push_back( Hint{ HintAction::CONFIRM, "hint.select" } );
	}
}

} // namespace core::ui

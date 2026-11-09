#include "Theme.h"
#include "Renderer.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>

namespace core::ui
{

namespace
{

constexpr float_t INF = std::numeric_limits< float_t >::infinity();

const char_t* COLOR_FORMAT = "色として解釈できない（#RRGGBB / #RRGGBBAA / r g b [a]。各成分は 0〜1）";

/// 先頭・末尾の空白（スペース・タブ・CR）を取り除く
string_t trim( const string_t& s )
{
	const char_t* ws = " \t\r";
	const auto begin = s.find_first_not_of( ws );

	if ( begin == string_t::npos )
	{
		return string_t();
	}

	const auto end = s.find_last_not_of( ws );
	return s.substr( begin, end - begin + 1 );
}

/// 「空白のあとの #」以降をコメントとして取り除く（値の先頭の # は色の指定なので残す）
string_t strip_comment( const string_t& s )
{
	for ( size_t i = 1; i < s.size(); i++ )
	{
		if ( s[ i ] == '#' && ( s[ i - 1 ] == ' ' || s[ i - 1 ] == '\t' ) )
		{
			return trim( s.substr( 0, i ) );
		}
	}

	return trim( s );
}

/// 空白区切りの数値をすべて読む。数値でないもの・有限でないもの（nan / inf）があれば false
bool parse_floats( const string_t& s, std::vector< float_t >& out )
{
	out.clear();
	std::istringstream ss( s );
	string_t token;

	while ( ss >> token )
	{
		char* end = nullptr;
		const float_t v = std::strtof( token.c_str(), & end );

		if ( end == token.c_str() || *end != '\0' || ! std::isfinite( v ) )
		{
			return false;
		}

		out.push_back( v );
	}

	return true;
}

/// #RRGGBB / #RRGGBBAA / "r g b" / "r g b a"（0〜1）
bool parse_color( const string_t& s, Color& out )
{
	if ( ! s.empty() && s[ 0 ] == '#' )
	{
		const string_t hex = s.substr( 1 );

		if ( hex.size() != 6 && hex.size() != 8 )
		{
			return false;
		}

		for ( char_t c : hex )
		{
			if ( ! ( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
			{
				return false;
			}
		}

		unsigned long v = std::strtoul( hex.c_str(), nullptr, 16 );

		if ( hex.size() == 6 )
		{
			v = ( v << 8 ) | 0xFF;
		}

		out = Color::from_hex( static_cast< Color::UintType >( v ) );
		return true;
	}

	std::vector< float_t > f;

	if ( ! parse_floats( s, f ) || ( f.size() != 3 && f.size() != 4 ) )
	{
		return false;
	}

	for ( float_t v : f )
	{
		if ( v < 0.f || v > 1.f )
		{
			return false;
		}
	}

	out = Color( f[ 0 ], f[ 1 ], f[ 2 ], f.size() == 4 ? f[ 3 ] : 1.f );
	return true;
}

struct BrushField
{
	const char_t* name;
	Brush Theme::* member;
};

struct ColorField
{
	const char_t* name;
	Color Theme::* member;
};

/// 数値の項目と、受け付ける範囲（min_exclusive なら min ちょうどは不可）
struct FloatField
{
	const char_t* name;
	float_t Theme::* member;
	float_t min;
	float_t max;
	bool min_exclusive;
};

const BrushField BRUSH_FIELDS[] =
{
	{ "panel", & Theme::panel },
	{ "row", & Theme::row },
	{ "row_focused", & Theme::row_focused },
	{ "row_pressed", & Theme::row_pressed },
	{ "row_disabled", & Theme::row_disabled },
	{ "track", & Theme::track },
	{ "accent", & Theme::accent },
	{ "slider_fill", & Theme::slider_fill },
	{ "fill_disabled", & Theme::fill_disabled },
	{ "slider_knob", & Theme::slider_knob },
	{ "toggle_on", & Theme::toggle_on },
	{ "tab_indicator", & Theme::tab_indicator },
	{ "tab_badge", & Theme::tab_badge },
	{ "scroll_thumb", & Theme::scroll_thumb },
	{ "dialog_header", & Theme::dialog_header },
};

const ColorField COLOR_FIELDS[] =
{
	{ "screen_dim", & Theme::screen_dim },
	{ "dialog_dim", & Theme::dialog_dim },
	{ "text", & Theme::text },
	{ "text_focused", & Theme::text_focused },
	{ "text_dim", & Theme::text_dim },
	{ "text_disabled", & Theme::text_disabled },
	{ "header", & Theme::header },
	{ "outline", & Theme::outline },
};

const FloatField FLOAT_FIELDS[] =
{
	{ "font_size_title", & Theme::font_size_title, 0.f, INF, true },
	{ "font_size_normal", & Theme::font_size_normal, 0.f, INF, true },
	{ "font_size_small", & Theme::font_size_small, 0.f, INF, true },
	{ "panel_width", & Theme::panel_width, 0.f, INF, true },
	{ "panel_margin_y", & Theme::panel_margin_y, 0.f, INF, false },
	{ "padding", & Theme::padding, 0.f, INF, false },
	{ "title_height", & Theme::title_height, 0.f, INF, false },
	{ "footer_height", & Theme::footer_height, 0.f, INF, false },
	{ "row_height", & Theme::row_height, 0.f, INF, true },
	{ "spacing", & Theme::spacing, 0.f, INF, false },
	{ "label_ratio", & Theme::label_ratio, 0.f, 1.f, false },
	{ "row_padding_x", & Theme::row_padding_x, 0.f, INF, false },
	{ "tab_height", & Theme::tab_height, 0.f, INF, true },
	{ "dialog_width", & Theme::dialog_width, 0.f, INF, true },
	{ "slider_knob_width", & Theme::slider_knob_width, 0.f, INF, false },
	{ "value_box_inset", & Theme::value_box_inset, 0.f, 0.45f, false },
	{ "slider_track_width", & Theme::slider_track_width, 0.f, 1.f, true },
	{ "slider_track_height", & Theme::slider_track_height, 0.f, 1.f, true },
	{ "choice_arrow_width", & Theme::choice_arrow_width, 0.f, 0.5f, false },
	{ "row_icon_ratio", & Theme::row_icon_ratio, 0.f, 1.f, false },
	{ "safe_area", & Theme::safe_area, 0.f, 0.2f, false },
	{ "tab_badge_size", & Theme::tab_badge_size, 0.f, 64.f, false },
	{ "accent_bar_thickness", & Theme::accent_bar_thickness, 0.f, INF, false },
	{ "toggle_mark_width", & Theme::toggle_mark_width, 0.f, INF, false },
	{ "scrollbar_width", & Theme::scrollbar_width, 0.f, INF, false },
	{ "scrollbar_min_thumb", & Theme::scrollbar_min_thumb, 0.f, INF, false },
	{ "menu_width", & Theme::menu_width, 0.f, INF, true },
	{ "section_spacing", & Theme::section_spacing, 0.f, INF, false },
	{ "row_text_padding", & Theme::row_text_padding, 0.f, INF, false },
};

Brush* find_brush( Theme& theme, const string_t& name )
{
	for ( const auto& f : BRUSH_FIELDS )
	{
		if ( name == f.name )
		{
			return & ( theme.*f.member );
		}
	}

	return nullptr;
}

/// Brush の 1 項目を解釈する。失敗したら理由を返す（成功は空文字列）
string_t apply_brush_property( Brush& brush, const string_t& prop, const string_t& value )
{
	std::vector< float_t > f;

	if ( prop.empty() || prop == "color" )
	{
		return parse_color( value, brush.color ) ? string_t() : COLOR_FORMAT;
	}
	if ( prop == "image" )
	{
		// 区切りは / にそろえる（同じ画像を別名で二重に読まないように）
		string_t path = value;

		for ( auto& c : path )
		{
			if ( c == '\\' )
			{
				c = '/';
			}
		}

		brush.image = path;		// 空なら画像なし（単色）に戻る
		return string_t();
	}
	if ( prop == "tint" )
	{
		return parse_color( value, brush.tint ) ? string_t() : COLOR_FORMAT;
	}
	if ( prop == "slice" )
	{
		if ( ! parse_floats( value, f ) || ( f.size() != 1 && f.size() != 4 ) )
		{
			return "数値 1 つ（四辺同じ）か 4 つ（左 上 右 下）で書く";
		}
		for ( float_t v : f )
		{
			if ( v < 0.f )
			{
				return "負の値は使えない";
			}
		}

		brush.slice = f.size() == 1 ? Insets( f[ 0 ], f[ 0 ], f[ 0 ], f[ 0 ] ) : Insets( f[ 0 ], f[ 1 ], f[ 2 ], f[ 3 ] );
		return string_t();
	}
	if ( prop == "slice_scale" )
	{
		if ( ! parse_floats( value, f ) || f.size() != 1 || f[ 0 ] < 0.f )
		{
			return "0 以上の数値 1 つで書く";
		}

		brush.slice_scale = f[ 0 ];
		return string_t();
	}
	if ( prop == "source" )
	{
		if ( ! parse_floats( value, f ) || f.size() != 4 || f[ 0 ] < 0.f || f[ 1 ] < 0.f || f[ 2 ] < 0.f || f[ 3 ] < 0.f )
		{
			return "0 以上の数値 4 つ（x y 幅 高さ）で書く";
		}

		brush.source = Rect( f[ 0 ], f[ 1 ], f[ 2 ], f[ 3 ] );
		return string_t();
	}

	if ( prop == "edge" || prop == "center" )
	{
		FillMode mode;

		if ( value == "stretch" )		{ mode = FillMode::STRETCH; }
		else if ( value == "repeat" )	{ mode = FillMode::REPEAT; }
		else if ( value == "round" )	{ mode = FillMode::ROUND; }
		else if ( value == "none" && prop == "center" ) { mode = FillMode::NONE; }
		else
		{
			return prop == "center" ? "stretch / repeat / round / none のどれかで書く" : "stretch / repeat / round のどれかで書く";
		}

		( prop == "edge" ? brush.edge : brush.center ) = mode;
		return string_t();
	}

	return "不明な属性（color / image / tint / slice / slice_scale / source / edge / center）";
}

/// 1 項目を反映する。失敗したら理由を返す（成功は空文字列）
string_t apply_entry( Theme& theme, const string_t& key, const string_t& value )
{
	// 面 : 属性はキーの最後の . で分ける（"panel" だけなら色の省略形）
	if ( Brush* brush = find_brush( theme, key ) )
	{
		return apply_brush_property( *brush, string_t(), value );
	}

	const auto dot = key.rfind( '.' );

	if ( dot != string_t::npos )
	{
		if ( Brush* brush = find_brush( theme, key.substr( 0, dot ) ) )
		{
			if ( dot + 1 == key.size() )
			{
				return "属性が空（例: panel.color）";
			}

			return apply_brush_property( *brush, key.substr( dot + 1 ), value );
		}

		return "不明なキー";
	}

	for ( const auto& f : COLOR_FIELDS )
	{
		if ( key == f.name )
		{
			return parse_color( value, theme.*f.member ) ? string_t() : COLOR_FORMAT;
		}
	}

	for ( const auto& f : FLOAT_FIELDS )
	{
		if ( key == f.name )
		{
			std::vector< float_t > v;

			if ( ! parse_floats( value, v ) || v.size() != 1 )
			{
				return "数値 1 つで書く";
			}

			const bool below = f.min_exclusive ? v[ 0 ] <= f.min : v[ 0 ] < f.min;

			if ( below || v[ 0 ] > f.max )
			{
				std::ostringstream ss;
				ss << "範囲外（" << ( f.min_exclusive ? "0 より大きい" : "0 以上" );
				if ( f.max != INF )
				{
					ss << "、" << f.max << " 以下";
				}
				ss << "）";
				return ss.str();
			}

			theme.*f.member = v[ 0 ];
			return string_t();
		}
	}

	return "不明なキー";
}

} // namespace

int load_theme( Theme& theme, const string_t& text, std::vector< string_t >* errors )
{
	auto report = [ errors ] ( int line, const string_t& key, const string_t& reason )
	{
		if ( errors )
		{
			errors->push_back( std::to_string( line ) + " 行目: " + key + ": " + reason );
		}
	};

	string_t body = text;

	// UTF-8 BOM を読み飛ばす
	if ( body.size() >= 3 &&
		static_cast< unsigned char >( body[ 0 ] ) == 0xEF &&
		static_cast< unsigned char >( body[ 1 ] ) == 0xBB &&
		static_cast< unsigned char >( body[ 2 ] ) == 0xBF )
	{
		body.erase( 0, 3 );
	}

	std::istringstream ss( body );
	std::map< string_t, int > seen;		// キー → 最初に書かれた行
	string_t line;
	int line_number = 0;
	int count = 0;

	while ( std::getline( ss, line ) )
	{
		line_number++;

		const string_t trimmed = trim( line );

		if ( trimmed.empty() || trimmed[ 0 ] == '#' )
		{
			continue;
		}

		const auto eq = trimmed.find( '=' );

		if ( eq == string_t::npos )
		{
			report( line_number, trimmed, "「キー = 値」の形になっていない" );
			continue;
		}

		const string_t key = trim( trimmed.substr( 0, eq ) );
		// 先に前後の空白を落とす（値の先頭の # を「空白のあとの #」= コメントと誤認しないように）
		const string_t value = strip_comment( trim( trimmed.substr( eq + 1 ) ) );

		if ( key.empty() )
		{
			report( line_number, trimmed, "キーが空" );
			continue;
		}

		// 重複は「同じ項目を書き換えるか」で判定する（"panel" と "panel.color" は同じ項目）
		const string_t item = find_brush( theme, key ) ? key + ".color" : key;

		if ( auto i = seen.find( item ); i != seen.end() )
		{
			report( line_number, key, std::to_string( i->second ) + " 行目と重複（後の行で上書きする）" );
		}
		else
		{
			seen[ item ] = line_number;
		}

		const string_t error = apply_entry( theme, key, value );

		if ( error.empty() )
		{
			count++;
		}
		else
		{
			report( line_number, key, error );
		}
	}

	return count;
}

bool load_theme_file( Theme& theme, const char_t* path, std::vector< string_t >* errors )
{
	// パスは UTF-8 として扱う（std::ifstream( const char* ) は ANSI として解釈するので、日本語を含むパスが開けない）
	std::ifstream in( std::filesystem::path( reinterpret_cast< const char8_t* >( path ) ), std::ios::binary );

	if ( ! in )
	{
		return false;
	}

	std::stringstream ss;
	ss << in.rdbuf();

	load_theme( theme, ss.str(), errors );
	return true;
}

void preload_theme_images( const Theme& theme, const Renderer& renderer )
{
	for ( const auto& f : BRUSH_FIELDS )
	{
		const Brush& b = theme.*f.member;

		if ( b.has_image() )
		{
			float_t w = 0.f;
			float_t h = 0.f;
			renderer.get_image_size( b.image, w, h );
		}
	}
}

} // namespace core::ui

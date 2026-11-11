#include "EffectParser.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <set>

namespace core::graphics::effect
{

namespace
{

struct Token
{
	enum class Type { IDENT, NUMBER, STRING, PUNCT };

	Type type;
	std::string text;
	size_t begin;		///< ソース上の位置
	size_t end;
	std::string location;	///< ファイル名(行)
};

bool is_ident_start( char c ) { return std::isalpha( static_cast< unsigned char >( c ) ) || c == '_'; }
bool is_ident_char( char c ) { return std::isalnum( static_cast< unsigned char >( c ) ) || c == '_'; }

/**
 * 字句に分ける
 *
 * コメントと、プリプロセッサの行（#line など）は読み飛ばす。
 * #line は D3DPreprocess が出すもので、元のファイル名と行を知らせるのに使う。
 */
std::vector< Token > tokenize( const std::string& s )
{
	std::vector< Token > tokens;

	std::string file;
	int line = 1;
	bool line_start = true;

	auto location = [&] () { return ( file.empty() ? std::string( "effect" ) : file ) + "(" + std::to_string( line ) + ")"; };

	size_t i = 0;

	while ( i < s.size() )
	{
		const char c = s[ i ];

		if ( c == '\n' )
		{
			line++;
			line_start = true;
			i++;
			continue;
		}

		if ( std::isspace( static_cast< unsigned char >( c ) ) )
		{
			i++;
			continue;
		}

		// プリプロセッサの行（#line N "file" を読み取る）
		if ( c == '#' && line_start )
		{
			size_t e = i;

			while ( e < s.size() && s[ e ] != '\n' )
			{
				e++;
			}

			const std::string directive = s.substr( i + 1, e - i - 1 );
			size_t p = directive.find_first_not_of( " \t" );

			if ( p != std::string::npos && directive.compare( p, 4, "line" ) == 0 )
			{
				p += 4;
			}

			const size_t digit = directive.find_first_of( "0123456789", p );

			if ( digit != std::string::npos && directive.find_first_not_of( " \t", p ) == digit )
			{
				const int n = std::atoi( directive.c_str() + digit );
				const size_t q1 = directive.find( '"', digit );
				const size_t q2 = q1 == std::string::npos ? std::string::npos : directive.find( '"', q1 + 1 );

				if ( q2 != std::string::npos )
				{
					file = directive.substr( q1 + 1, q2 - q1 - 1 );
				}

				line = n - 1;	// 次の行が n 行目
			}

			i = e;
			continue;
		}

		line_start = false;

		// コメント
		if ( c == '/' && i + 1 < s.size() && s[ i + 1 ] == '/' )
		{
			while ( i < s.size() && s[ i ] != '\n' )
			{
				i++;
			}
			continue;
		}

		if ( c == '/' && i + 1 < s.size() && s[ i + 1 ] == '*' )
		{
			i += 2;

			while ( i + 1 < s.size() && ! ( s[ i ] == '*' && s[ i + 1 ] == '/' ) )
			{
				if ( s[ i ] == '\n' )
				{
					line++;
				}
				i++;
			}

			i = std::min( i + 2, s.size() );
			continue;
		}

		Token t { Token::Type::PUNCT, std::string(), i, i, location() };

		if ( is_ident_start( c ) )
		{
			size_t e = i;

			while ( e < s.size() && is_ident_char( s[ e ] ) )
			{
				e++;
			}

			t.type = Token::Type::IDENT;
			t.end = e;
		}
		else if ( std::isdigit( static_cast< unsigned char >( c ) ) || ( c == '.' && i + 1 < s.size() && std::isdigit( static_cast< unsigned char >( s[ i + 1 ] ) ) ) )
		{
			const bool hex = c == '0' && i + 1 < s.size() && ( s[ i + 1 ] == 'x' || s[ i + 1 ] == 'X' );
			size_t e = i;

			while ( e < s.size() && ( is_ident_char( s[ e ] ) || s[ e ] == '.' ) )
			{
				// 指数の符号（1e-3）
				if ( ! hex && ( s[ e ] == 'e' || s[ e ] == 'E' ) && e + 1 < s.size() && ( s[ e + 1 ] == '+' || s[ e + 1 ] == '-' ) )
				{
					e++;
				}
				e++;
			}

			t.type = Token::Type::NUMBER;
			t.end = e;
		}
		else if ( c == '"' )
		{
			size_t e = i + 1;

			while ( e < s.size() && s[ e ] != '"' && s[ e ] != '\n' )
			{
				e += s[ e ] == '\\' ? 2 : 1;
			}

			t.type = Token::Type::STRING;
			t.end = std::min( e + 1, s.size() );
		}
		else
		{
			t.end = i + 1;
		}

		t.text = s.substr( t.begin, t.end - t.begin );
		tokens.push_back( std::move( t ) );
		i = tokens.back().end;
	}

	return tokens;
}

/// 範囲を空白にする（改行とプリプロセッサの行は残して、行の位置と #line を保つ）
void blank( std::string& s, size_t begin, size_t end )
{
	size_t p = begin;

	while ( p < end && p < s.size() )
	{
		if ( p == 0 || s[ p - 1 ] == '\n' )
		{
			size_t q = p;

			while ( q < end && ( s[ q ] == ' ' || s[ q ] == '\t' ) )
			{
				q++;
			}

			if ( q < end && s[ q ] == '#' )
			{
				while ( p < s.size() && s[ p ] != '\n' )
				{
					p++;
				}
				continue;
			}
		}

		if ( s[ p ] != '\n' && s[ p ] != '\r' )
		{
			s[ p ] = ' ';
		}

		p++;
	}
}

std::string trim( const std::string& s )
{
	const size_t b = s.find_first_not_of( " \t\r\n" );

	if ( b == std::string::npos )
	{
		return std::string();
	}

	const size_t e = s.find_last_not_of( " \t\r\n" );

	return s.substr( b, e - b + 1 );
}

class Parser
{
private:
	const std::string& source_;
	std::vector< Token > tokens_;
	size_t pos_ = 0;
	ParseResult& result_;

public:
	Parser( const std::string& source, ParseResult& result )
		: source_( source )
		, tokens_( tokenize( source ) )
		, result_( result )
	{ }

	void parse()
	{
		result_.shader_source = source_;

		int brace = 0;
		int paren = 0;

		while ( pos_ < tokens_.size() )
		{
			const Token& t = tokens_[ pos_ ];

			if ( brace == 0 && paren == 0 && t.type == Token::Type::IDENT )
			{
				if ( t.text == "technique11" || t.text == "technique10" || t.text == "technique" )
				{
					parse_technique();
					continue;
				}

				if ( t.text == "fxgroup" )
				{
					error( t, "fxgroup には対応していません" );
					skip_block_statement();
					continue;
				}

				if ( auto kind = state_kind_of( t.text ) )
				{
					if ( peek( 1 ).type == Token::Type::IDENT && peek( 2 ).text == "{" )
					{
						parse_state_block( *kind );
						continue;
					}

					if ( peek( 1 ).type == Token::Type::IDENT && peek( 2 ).text == "[" )
					{
						warning( t, "配列の状態（" + peek( 1 ).text + "）には対応していません" );
					}
				}
			}

			if ( t.text == "{" ) { brace++; }
			else if ( t.text == "}" ) { brace--; }
			else if ( t.text == "(" ) { paren++; }
			else if ( t.text == ")" ) { paren--; }

			pos_++;
		}

		generate_wrappers();
	}

private:
	const Token& peek( size_t offset = 0 ) const
	{
		static const Token END { Token::Type::PUNCT, std::string(), 0, 0, std::string() };

		return pos_ + offset < tokens_.size() ? tokens_[ pos_ + offset ] : END;
	}

	bool at_end() const { return pos_ >= tokens_.size(); }

	void warning( const Token& t, const std::string& message ) { result_.warnings.push_back( t.location + ": " + message ); }
	void error( const Token& t, const std::string& message ) { result_.errors.push_back( t.location + ": " + message ); }

	static std::optional< StateKind > state_kind_of( const std::string& s )
	{
		if ( s == "BlendState" ) { return StateKind::BLEND; }
		if ( s == "DepthStencilState" ) { return StateKind::DEPTH_STENCIL; }
		if ( s == "RasterizerState" ) { return StateKind::RASTERIZER; }
		if ( s == "SamplerState" || s == "SamplerComparisonState" ) { return StateKind::SAMPLER; }

		return std::nullopt;
	}

	/// 対応する閉じ括弧の位置（tokens_ の添字）。見つからなければ tokens_.size()
	size_t find_close( size_t open ) const
	{
		const std::string o = tokens_[ open ].text;
		const std::string c = o == "{" ? "}" : o == "(" ? ")" : o == "[" ? "]" : ">";
		int depth = 0;

		for ( size_t i = open; i < tokens_.size(); i++ )
		{
			if ( tokens_[ i ].text == o ) { depth++; }
			else if ( tokens_[ i ].text == c )
			{
				if ( --depth == 0 )
				{
					return i;
				}
			}
		}

		return tokens_.size();
	}

	/// < … > の注釈を読み飛ばす
	void skip_annotations()
	{
		if ( peek().text == "<" )
		{
			pos_ = std::min( find_close( pos_ ) + 1, tokens_.size() );
		}
	}

	/// 次の { … } とその後の ; までを読み飛ばす
	void skip_block_statement()
	{
		while ( ! at_end() && peek().text != "{" )
		{
			pos_++;
		}

		if ( ! at_end() )
		{
			pos_ = std::min( find_close( pos_ ) + 1, tokens_.size() );
		}

		if ( peek().text == ";" )
		{
			pos_++;
		}
	}

	std::string text_of( size_t first, size_t last ) const
	{
		if ( first > last || last >= tokens_.size() )
		{
			return std::string();
		}

		return source_.substr( tokens_[ first ].begin, tokens_[ last ].end - tokens_[ first ].begin );
	}

	/// first から last（含む）を、括弧の外のカンマで分ける（各要素は [ 先頭, 末尾 ]）
	std::vector< std::pair< size_t, size_t > > split_arguments( size_t first, size_t last ) const
	{
		std::vector< std::pair< size_t, size_t > > list;

		if ( first > last || last >= tokens_.size() )
		{
			return list;
		}

		int depth = 0;
		size_t begin = first;

		for ( size_t i = first; i <= last; i++ )
		{
			const std::string& t = tokens_[ i ].text;

			if ( t == "(" || t == "[" || t == "{" ) { depth++; }
			else if ( t == ")" || t == "]" || t == "}" ) { depth--; }
			else if ( t == "," && depth == 0 )
			{
				list.emplace_back( begin, i - 1 );
				begin = i + 1;
			}
		}

		list.emplace_back( begin, last );

		return list;
	}

	void parse_technique()
	{
		const size_t start = pos_;
		const Token& keyword = tokens_[ pos_++ ];

		if ( peek().type != Token::Type::IDENT )
		{
			error( keyword, "technique の名前がありません" );
			skip_block_statement();
			blank( result_.shader_source, keyword.begin, tokens_[ std::min( pos_, tokens_.size() ) - 1 ].end );
			return;
		}

		TechniqueDesc technique;
		technique.name = tokens_[ pos_++ ].text;

		skip_annotations();

		if ( peek().text != "{" )
		{
			error( keyword, "technique " + technique.name + " の { がありません" );
			return;
		}

		const size_t close = find_close( pos_ );

		if ( close >= tokens_.size() )
		{
			error( keyword, "technique " + technique.name + " の } がありません" );
			pos_ = tokens_.size();
			return;
		}

		pos_++;

		while ( pos_ < close )
		{
			const Token& t = tokens_[ pos_ ];

			if ( t.type == Token::Type::IDENT && t.text == "pass" )
			{
				parse_pass( technique, close );
			}
			else
			{
				error( t, "technique " + technique.name + " の中に pass 以外のもの（" + t.text + "）があります" );
				pos_++;
			}
		}

		pos_ = close + 1;

		if ( peek().text == ";" )
		{
			pos_++;
		}

		blank( result_.shader_source, tokens_[ start ].begin, tokens_[ pos_ - 1 ].end );

		if ( result_.find_technique( technique.name ) )
		{
			warning( keyword, "technique " + technique.name + " が 2 回定義されています（最初のものを使います）" );
			return;
		}

		result_.techniques.push_back( std::move( technique ) );
	}

	void parse_pass( TechniqueDesc& technique, size_t technique_close )
	{
		const Token& keyword = tokens_[ pos_++ ];

		PassDesc pass;

		if ( peek().type == Token::Type::IDENT )
		{
			pass.name = tokens_[ pos_++ ].text;
		}

		skip_annotations();

		if ( peek().text != "{" )
		{
			error( keyword, "pass " + pass.name + " の { がありません" );
			pos_ = technique_close;
			return;
		}

		const size_t close = std::min( find_close( pos_ ), technique_close );
		pos_++;

		while ( pos_ < close )
		{
			// ; までを 1 文として読む
			size_t end = pos_;
			int depth = 0;

			while ( end < close && ! ( depth == 0 && tokens_[ end ].text == ";" ) )
			{
				const std::string& s = tokens_[ end ].text;

				if ( s == "(" ) { depth++; }
				else if ( s == ")" ) { depth--; }

				end++;
			}

			if ( end > pos_ )
			{
				parse_pass_statement( technique, pass, pos_, end - 1 );
			}

			pos_ = end + 1;
		}

		pos_ = close + 1;

		technique.passes.push_back( std::move( pass ) );
	}

	void parse_pass_statement( const TechniqueDesc& technique, PassDesc& pass, size_t first, size_t last )
	{
		const Token& head = tokens_[ first ];
		const std::string where = "technique " + technique.name + " / pass " + pass.name + ": ";

		if ( head.type != Token::Type::IDENT )
		{
			error( head, where + "読めない文です: " + text_of( first, last ) );
			return;
		}

		// 関数の形（SetBlendState( … )）
		if ( first + 1 <= last && tokens_[ first + 1 ].text == "(" )
		{
			const size_t close = find_close( first + 1 );

			if ( close != last )
			{
				error( head, where + "読めない文です: " + text_of( first, last ) );
				return;
			}

			const auto args = close > first + 2 ? split_arguments( first + 2, close - 1 ) : std::vector< std::pair< size_t, size_t > >();
			const std::string& name = head.text;

			if ( auto stage = shader_stage_of_setter( name ) )
			{
				if ( args.size() != 1 )
				{
					error( head, where + name + " の引数の数が違います" );
					return;
				}

				set_shader( pass, *stage, head, args[ 0 ].first, args[ 0 ].second, where );
				return;
			}

			if ( equals_ignore_case( name, "SetBlendState" ) )
			{
				if ( args.size() != 3 )
				{
					error( head, where + "SetBlendState の引数の数が違います" );
					return;
				}

				set_state( pass.blend_state, args[ 0 ] );

				if ( ! parse_float4( text_of( args[ 1 ].first, args[ 1 ].second ), pass.blend_factor ) )
				{
					warning( head, where + "BlendFactor を読めません: " + text_of( args[ 1 ].first, args[ 1 ].second ) );
				}

				pass.sample_mask = read_uint( head, args[ 2 ], 0xFFFFFFFF, where + "SampleMask" );
				return;
			}

			if ( equals_ignore_case( name, "SetDepthStencilState" ) )
			{
				if ( args.size() != 2 )
				{
					error( head, where + "SetDepthStencilState の引数の数が違います" );
					return;
				}

				set_state( pass.depth_stencil_state, args[ 0 ] );
				pass.stencil_ref = read_uint( head, args[ 1 ], 0, where + "StencilRef" );
				return;
			}

			if ( equals_ignore_case( name, "SetRasterizerState" ) )
			{
				if ( args.size() != 1 )
				{
					error( head, where + "SetRasterizerState の引数の数が違います" );
					return;
				}

				set_state( pass.rasterizer_state, args[ 0 ] );
				return;
			}

			warning( head, where + "対応していない設定です（無視します）: " + text_of( first, last ) );
			return;
		}

		// 代入の形（RASTERIZERSTATE = Default）
		size_t eq = first + 1;

		if ( eq <= last && tokens_[ eq ].text == "[" )
		{
			eq = find_close( eq ) + 1;
		}

		if ( eq > last || tokens_[ eq ].text != "=" || eq + 1 > last )
		{
			error( head, where + "読めない文です: " + text_of( first, last ) );
			return;
		}

		const std::pair< size_t, size_t > value( eq + 1, last );
		const std::string& key = head.text;

		if ( auto stage = shader_stage_of_key( key ) )
		{
			set_shader( pass, *stage, head, value.first, value.second, where );
		}
		else if ( equals_ignore_case( key, "BlendState" ) )
		{
			set_state( pass.blend_state, value );
		}
		else if ( equals_ignore_case( key, "DepthStencilState" ) )
		{
			set_state( pass.depth_stencil_state, value );
		}
		else if ( equals_ignore_case( key, "RasterizerState" ) )
		{
			set_state( pass.rasterizer_state, value );
		}
		else if ( equals_ignore_case( key, "BlendFactor" ) || equals_ignore_case( key, "AB_BlendFactor" ) )
		{
			if ( ! parse_float4( text_of( value.first, value.second ), pass.blend_factor ) )
			{
				warning( head, where + "BlendFactor を読めません: " + text_of( value.first, value.second ) );
			}
		}
		else if ( equals_ignore_case( key, "SampleMask" ) || equals_ignore_case( key, "AB_SampleMask" ) )
		{
			pass.sample_mask = read_uint( head, value, 0xFFFFFFFF, where + "SampleMask" );
		}
		else if ( equals_ignore_case( key, "StencilRef" ) || equals_ignore_case( key, "DS_StencilRef" ) )
		{
			pass.stencil_ref = read_uint( head, value, 0, where + "StencilRef" );
		}
		else
		{
			warning( head, where + "対応していない設定です（無視します）: " + text_of( first, last ) );
		}
	}

	uint32_t read_uint( const Token& head, std::pair< size_t, size_t > range, uint32_t fallback, const std::string& what )
	{
		double v = 0;

		if ( ! parse_number( text_of( range.first, range.second ), v ) )
		{
			warning( head, what + " を読めません: " + text_of( range.first, range.second ) );
			return fallback;
		}

		return static_cast< uint32_t >( static_cast< int64_t >( v ) );
	}

	void set_state( StateRef& ref, std::pair< size_t, size_t > range )
	{
		ref.is_set = true;
		ref.name = text_of( range.first, range.second );

		if ( equals_ignore_case( ref.name, "NULL" ) || ref.name == "0" )
		{
			ref.name.clear();
		}
	}

	static std::optional< ShaderStage > shader_stage_of_setter( const std::string& name )
	{
		if ( equals_ignore_case( name, "SetVertexShader" ) ) { return ShaderStage::VS; }
		if ( equals_ignore_case( name, "SetHullShader" ) ) { return ShaderStage::HS; }
		if ( equals_ignore_case( name, "SetDomainShader" ) ) { return ShaderStage::DS; }
		if ( equals_ignore_case( name, "SetGeometryShader" ) ) { return ShaderStage::GS; }
		if ( equals_ignore_case( name, "SetPixelShader" ) ) { return ShaderStage::PS; }

		return std::nullopt;
	}

	static std::optional< ShaderStage > shader_stage_of_key( const std::string& name )
	{
		if ( equals_ignore_case( name, "VertexShader" ) ) { return ShaderStage::VS; }
		if ( equals_ignore_case( name, "HullShader" ) ) { return ShaderStage::HS; }
		if ( equals_ignore_case( name, "DomainShader" ) ) { return ShaderStage::DS; }
		if ( equals_ignore_case( name, "GeometryShader" ) ) { return ShaderStage::GS; }
		if ( equals_ignore_case( name, "PixelShader" ) ) { return ShaderStage::PS; }

		return std::nullopt;
	}

	/// NULL か CompileShader( プロファイル, 関数( 引数… ) ) を読む
	void set_shader( PassDesc& pass, ShaderStage stage, const Token& head, size_t first, size_t last, const std::string& where )
	{
		ShaderRef ref;

		if ( first == last && ( equals_ignore_case( tokens_[ first ].text, "NULL" ) || tokens_[ first ].text == "0" ) )
		{
			pass.shaders[ static_cast< size_t >( stage ) ] = ref;
			return;
		}

		const bool is_compile = tokens_[ first ].text == "CompileShader" && first + 1 <= last && tokens_[ first + 1 ].text == "(" && find_close( first + 1 ) == last;

		if ( ! is_compile )
		{
			error( head, where + "対応していないシェーダーの指定です（CompileShader( プロファイル, 関数() ) か NULL のみ対応）: " + text_of( first, last ) );
			return;
		}

		const auto args = split_arguments( first + 2, last - 1 );

		if ( args.size() != 2 || args[ 0 ].first != args[ 0 ].second || tokens_[ args[ 0 ].first ].type != Token::Type::IDENT )
		{
			error( head, where + "CompileShader の引数を読めません: " + text_of( first, last ) );
			return;
		}

		ref.profile = tokens_[ args[ 0 ].first ].text;

		const size_t f = args[ 1 ].first;
		const size_t f_last = args[ 1 ].second;

		if ( tokens_[ f ].type != Token::Type::IDENT || f + 1 > f_last || tokens_[ f + 1 ].text != "(" || find_close( f + 1 ) != f_last )
		{
			error( head, where + "CompileShader の関数の呼び出しを読めません: " + text_of( args[ 1 ].first, args[ 1 ].second ) );
			return;
		}

		ref.function = tokens_[ f ].text;
		ref.entry_point = ref.function;

		if ( f_last > f + 2 )
		{
			for ( const auto& a : split_arguments( f + 2, f_last - 1 ) )
			{
				ref.arguments.push_back( text_of( a.first, a.second ) );
			}
		}

		pass.shaders[ static_cast< size_t >( stage ) ] = ref;
	}

	void parse_state_block( StateKind kind )
	{
		const size_t start = pos_;
		const Token& keyword = tokens_[ pos_ ];

		StateBlockDesc block;
		block.kind = kind;
		block.name = tokens_[ pos_ + 1 ].text;
		block.location = keyword.location;

		const size_t open = pos_ + 2;
		const size_t close = find_close( open );

		if ( close >= tokens_.size() )
		{
			error( keyword, block.name + " の } がありません" );
			pos_ = tokens_.size();
			return;
		}

		size_t i = open + 1;

		while ( i < close )
		{
			size_t end = i;

			while ( end < close && tokens_[ end ].text != ";" )
			{
				end++;
			}

			if ( end > i )
			{
				parse_state_assignment( block, i, end - 1 );
			}

			i = end + 1;
		}

		pos_ = close + 1;

		if ( kind == StateKind::SAMPLER )
		{
			// サンプラーは宣言を残す（シェーダーが名前で参照する）。中身だけ空白にする
			blank( result_.shader_source, tokens_[ open ].begin, tokens_[ close ].end );
		}
		else
		{
			if ( peek().text == ";" )
			{
				pos_++;
			}

			blank( result_.shader_source, tokens_[ start ].begin, tokens_[ pos_ - 1 ].end );
		}

		if ( result_.find_state_block( kind, block.name ) )
		{
			warning( keyword, block.name + " が 2 回定義されています（最初のものを使います）" );
			return;
		}

		result_.state_blocks.push_back( std::move( block ) );
	}

	void parse_state_assignment( StateBlockDesc& block, size_t first, size_t last )
	{
		const Token& head = tokens_[ first ];

		StateAssignment a;
		a.location = head.location;

		size_t eq = first + 1;

		if ( head.type != Token::Type::IDENT )
		{
			error( head, block.name + ": 読めない行です: " + text_of( first, last ) );
			return;
		}

		a.key = head.text;

		if ( eq <= last && tokens_[ eq ].text == "[" )
		{
			const size_t close = find_close( eq );
			double index = 0;

			if ( close > last || ! parse_number( text_of( eq + 1, close - 1 ), index ) )
			{
				error( head, block.name + ": 添字を読めません: " + text_of( first, last ) );
				return;
			}

			a.index = static_cast< int >( index );
			eq = close + 1;
		}

		if ( eq > last || tokens_[ eq ].text != "=" || eq + 1 > last )
		{
			error( head, block.name + ": 読めない行です: " + text_of( first, last ) );
			return;
		}

		a.value = text_of( eq + 1, last );
		block.assignments.push_back( std::move( a ) );
	}

	/// 関数の定義を探す（[ 先頭, 関数名, 引数の ( , ) , 本体の { ] の位置を返す）
	struct FunctionDefinition
	{
		size_t prefix_begin;	///< 属性と戻り値の型の始まり（ソース上の位置）
		size_t name;			///< 関数名のトークン
		size_t open;			///< ( のトークン
		size_t close;			///< ) のトークン
		size_t body;			///< { のトークン
	};

	static std::optional< FunctionDefinition > find_function( const std::vector< Token >& tokens, const std::string& name )
	{
		int brace = 0;
		int paren = 0;
		size_t item_begin = 0;	// 直前の ; か } の次のトークン

		for ( size_t i = 0; i < tokens.size(); i++ )
		{
			const std::string& t = tokens[ i ].text;

			if ( brace == 0 && paren == 0 && t == name && tokens[ i ].type == Token::Type::IDENT && i + 1 < tokens.size() && tokens[ i + 1 ].text == "(" )
			{
				// 対応する ) を探す
				int depth = 0;
				size_t close = i + 1;

				for ( ; close < tokens.size(); close++ )
				{
					if ( tokens[ close ].text == "(" ) { depth++; }
					else if ( tokens[ close ].text == ")" && --depth == 0 ) { break; }
				}

				size_t body = close + 1;

				while ( body < tokens.size() && tokens[ body ].text != "{" && tokens[ body ].text != ";" )
				{
					body++;
				}

				if ( body < tokens.size() && tokens[ body ].text == "{" )
				{
					return FunctionDefinition { tokens[ item_begin ].begin, i, i + 1, close, body };
				}
			}

			if ( t == "{" ) { brace++; }
			else if ( t == "}" )
			{
				brace--;

				if ( brace == 0 && paren == 0 )
				{
					item_begin = i + 1;
				}
			}
			else if ( t == "(" ) { paren++; }
			else if ( t == ")" ) { paren--; }
			else if ( t == ";" && brace == 0 && paren == 0 )
			{
				item_begin = i + 1;
			}
		}

		return std::nullopt;
	}

	/**
	 * 引数付きのエントリ（CompileShader( vs_4_0, vs_foo( 1 ) )）のために、ラッパーのエントリ関数を作ってソースに足す
	 *
	 * effect では、CompileShader に書いた引数は関数の uniform の引数に前から順に渡る。
	 * ラッパーは uniform 以外の引数だけを受け取り、元の関数に引数の式と一緒に渡す。属性（[maxvertexcount] など）と戻り値の意味（: SV_Target）は写す。
	 */
	void generate_wrappers()
	{
		std::map< std::string, std::string > entry_of_key;
		const std::vector< Token > shader_tokens = tokenize( result_.shader_source );
		std::string wrappers;

		for ( auto& technique : result_.techniques )
		{
			for ( auto& pass : technique.passes )
			{
				for ( auto& shader : pass.shaders )
				{
					if ( ! shader || shader->is_null() || shader->arguments.empty() )
					{
						continue;
					}

					const std::string key = shader->key();

					if ( auto i = entry_of_key.find( key ); i != entry_of_key.end() )
					{
						shader->entry_point = i->second;
						continue;
					}

					const std::string entry = "effect_entry_" + shader->function + "_" + std::to_string( entry_of_key.size() );
					const std::string wrapper = make_wrapper( shader_tokens, *shader, entry, technique.name );

					if ( wrapper.empty() )
					{
						continue;
					}

					wrappers += wrapper;
					entry_of_key[ key ] = entry;
					shader->entry_point = entry;
				}
			}
		}

		if ( ! wrappers.empty() )
		{
			result_.shader_source += "\n// 引数付きのエントリのラッパー（EffectParser が生成）\n" + wrappers;
		}
	}

	std::string make_wrapper( const std::vector< Token >& tokens, const ShaderRef& shader, const std::string& entry, const std::string& technique_name )
	{
		const auto def = find_function( tokens, shader.function );

		if ( ! def )
		{
			result_.errors.push_back( "technique " + technique_name + ": エントリ関数 " + shader.function + " の定義が見つかりません" );
			return std::string();
		}

		const std::string& s = result_.shader_source;

		// 属性と戻り値の型（プリプロセッサの行は除く）
		std::string prefix;
		{
			const std::string raw = s.substr( def->prefix_begin, tokens[ def->name ].begin - def->prefix_begin );
			size_t p = 0;

			while ( p < raw.size() )
			{
				size_t e = raw.find( '\n', p );
				e = e == std::string::npos ? raw.size() : e + 1;

				const std::string line = raw.substr( p, e - p );

				if ( trim( line ).rfind( "#", 0 ) != 0 )
				{
					prefix += line;
				}

				p = e;
			}
		}

		const bool is_void = [&] () {
			for ( size_t i = def->name; i > 0; i-- )
			{
				if ( tokens[ i - 1 ].type == Token::Type::IDENT )
				{
					return tokens[ i - 1 ].text == "void";
				}
			}
			return false;
		}();

		// 引数を分ける（InputPatch< T, 3 > のカンマで切らないように < > も数える）
		std::vector< std::pair< size_t, size_t > > params;

		if ( def->close > def->open + 1 )
		{
			int depth = 0;
			size_t begin = def->open + 1;

			for ( size_t i = def->open + 1; i < def->close; i++ )
			{
				const std::string& t = tokens[ i ].text;

				if ( t == "(" || t == "[" || t == "<" ) { depth++; }
				else if ( t == ")" || t == "]" || t == ">" ) { depth--; }
				else if ( t == "," && depth == 0 )
				{
					params.emplace_back( begin, i - 1 );
					begin = i + 1;
				}
			}

			params.emplace_back( begin, def->close - 1 );
		}

		std::vector< std::string > wrapper_params;
		std::vector< std::string > call_args;
		size_t next_argument = 0;

		for ( const auto& [ first, last ] : params )
		{
			bool is_uniform = false;
			size_t name_last = last;

			for ( size_t i = first; i <= last; i++ )
			{
				if ( tokens[ i ].text == "uniform" )
				{
					is_uniform = true;
				}

				if ( tokens[ i ].text == ":" || tokens[ i ].text == "=" )
				{
					name_last = i - 1;
					break;
				}
			}

			// 配列の [ n ] を除いた最後の識別子が引数の名前
			while ( name_last > first && tokens[ name_last ].text == "]" )
			{
				while ( name_last > first && tokens[ name_last ].text != "[" )
				{
					name_last--;
				}
				name_last--;
			}

			if ( is_uniform )
			{
				if ( next_argument >= shader.arguments.size() )
				{
					result_.errors.push_back( "technique " + technique_name + ": " + shader.function + " の uniform 引数に渡す値が足りません" );
					return std::string();
				}

				call_args.push_back( "( " + shader.arguments[ next_argument++ ] + " )" );
			}
			else
			{
				wrapper_params.push_back( s.substr( tokens[ first ].begin, tokens[ last ].end - tokens[ first ].begin ) );
				call_args.push_back( tokens[ name_last ].text );
			}
		}

		if ( next_argument != shader.arguments.size() )
		{
			result_.errors.push_back( "technique " + technique_name + ": " + shader.function + " に渡す値が uniform 引数より多いです" );
			return std::string();
		}

		const std::string after_params = s.substr( tokens[ def->close ].end, tokens[ def->body ].begin - tokens[ def->close ].end );

		std::string w = trim( prefix ) + " " + entry + "( ";

		for ( size_t i = 0; i < wrapper_params.size(); i++ )
		{
			w += ( i ? ", " : "" ) + wrapper_params[ i ];
		}

		w += " )" + after_params + "\n{\n\t" + ( is_void ? "" : "return " ) + shader.function + "( ";

		for ( size_t i = 0; i < call_args.size(); i++ )
		{
			w += ( i ? ", " : "" ) + call_args[ i ];
		}

		w += " );\n}\n";

		return w;
	}
};

} // namespace

std::string ShaderRef::key() const
{
	std::string k = profile + "|" + function + "(";

	for ( size_t i = 0; i < arguments.size(); i++ )
	{
		k += ( i ? "," : "" ) + arguments[ i ];
	}

	return k + ")";
}

const TechniqueDesc* ParseResult::find_technique( const std::string& name ) const
{
	for ( const auto& t : techniques )
	{
		if ( t.name == name )
		{
			return & t;
		}
	}

	return nullptr;
}

const StateBlockDesc* ParseResult::find_state_block( StateKind kind, const std::string& name ) const
{
	for ( const auto& b : state_blocks )
	{
		if ( b.kind == kind && b.name == name )
		{
			return & b;
		}
	}

	return nullptr;
}

ParseResult parse( const std::string& source )
{
	ParseResult result;
	Parser( source, result ).parse();

	return result;
}

bool equals_ignore_case( const std::string& a, const std::string& b )
{
	return a.size() == b.size() && std::equal( a.begin(), a.end(), b.begin(), [] ( char x, char y ) {
		return std::tolower( static_cast< unsigned char >( x ) ) == std::tolower( static_cast< unsigned char >( y ) );
	} );
}

bool parse_number( const std::string& text, double& value )
{
	std::string s = trim( text );

	if ( s.empty() )
	{
		return false;
	}

	bool negative = false;

	if ( s[ 0 ] == '-' || s[ 0 ] == '+' )
	{
		negative = s[ 0 ] == '-';
		s = trim( s.substr( 1 ) );
	}

	if ( s.size() > 2 && s[ 0 ] == '0' && ( s[ 1 ] == 'x' || s[ 1 ] == 'X' ) )
	{
		char* end = nullptr;
		const unsigned long long v = std::strtoull( s.c_str() + 2, & end, 16 );

		if ( end == s.c_str() + 2 || *end != '\0' )
		{
			return false;
		}

		value = negative ? -static_cast< double >( v ) : static_cast< double >( v );
		return true;
	}

	// 末尾の f / h（2.f・0.5h）
	if ( ! s.empty() && ( s.back() == 'f' || s.back() == 'F' || s.back() == 'h' || s.back() == 'H' ) )
	{
		s.pop_back();
	}

	if ( s.empty() || ! ( std::isdigit( static_cast< unsigned char >( s[ 0 ] ) ) || s[ 0 ] == '.' ) )
	{
		return false;
	}

	char* end = nullptr;
	const double v = std::strtod( s.c_str(), & end );

	if ( *end != '\0' )
	{
		return false;
	}

	value = negative ? -v : v;
	return true;
}

bool parse_float4( const std::string& text, std::array< float, 4 >& value )
{
	const std::string s = trim( text );
	const size_t open = s.find( '(' );

	if ( open == std::string::npos || s.back() != ')' || trim( s.substr( 0, open ) ) != "float4" )
	{
		return false;
	}

	const std::string inner = s.substr( open + 1, s.size() - open - 2 );
	std::array< float, 4 > result { };
	size_t p = 0;

	for ( size_t n = 0; n < 4; n++ )
	{
		const size_t comma = inner.find( ',', p );
		const std::string part = inner.substr( p, comma == std::string::npos ? std::string::npos : comma - p );
		double v = 0;

		if ( ! parse_number( part, v ) )
		{
			return false;
		}

		result[ n ] = static_cast< float >( v );

		if ( n < 3 )
		{
			if ( comma == std::string::npos )
			{
				return false;
			}

			p = comma + 1;
		}
		else if ( comma != std::string::npos )
		{
			return false;
		}
	}

	value = result;
	return true;
}

} // namespace core::graphics::effect

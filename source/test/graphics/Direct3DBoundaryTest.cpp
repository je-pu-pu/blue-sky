#include "pch.h"

// Direct3D 11 を知ってよい場所の決まりのテスト（再発を防ぐ）
//
// 決まり : Direct3D 11 の型・ヘッダ（core/graphics/Direct3D11/*・blue_sky/graphics/Direct3D11/*・d3d11.h・dxgi・ID3D11*・DXGI_*・
// direct_3d_11:: の名前空間）を知ってよいのは、組み立ての根（GameMain。App・Main.cpp も根だが、今は Direct3D を知らない）と描画の実装（Direct3D11 のフォルダ）と、
// 実装を直接試すテストだけ。ゲーム（Scene・blue_sky）も core の抽象（core/graphics の Direct3D11 の外・core/ecs・core/animation など）も知らない。
// source の .h・.cpp を読み、コメントと文字列を除いた中にこれらが出てきたら失敗する。

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

/// Direct3D 11 を知ってよい場所（source からの相対パス。/ で終わるものはフォルダ）と理由
struct AllowedPlace
{
	const char* path;
	const char* reason;
};

const AllowedPlace ALLOWED_PLACES[] =
{
	{ "core/graphics/Direct3D11/",				"描画の実装（core）" },
	{ "blue_sky/graphics/Direct3D11/",			"描画の実装（ゲーム固有の描画）" },
	{ "blue_sky/GameMain.h",					"組み立ての根（Direct3D を作り、実装を作って抽象の窓口として渡す。ヘッダは前方宣言だけ）" },
	{ "blue_sky/GameMain.cpp",					"組み立ての根" },
	{ "test/graphics/EffectParserTest.cpp",		"実装（Effect の状態の作り方・シェーダーのコンパイル）を直接試すテスト" },
	{ "test/graphics/MsdfFontTest.cpp",			"実装（MSDF の文字の描画）を直接試すテスト" },
	{ "test/graphics/Direct3DBoundaryTest.cpp",	"このテスト（調べる言葉を書いている）" },
};

/// 調べないフォルダ（外のライブラリ・ビルドの出力・素材）
const char* const SKIPPED_DIRECTORIES[] = { "lib", "extern", "packages", ".build", "media", "log", "save", "blender_script" };

fs::path get_source_root()
{
	return ( fs::path( __FILE__ ).parent_path() / ".." / ".." ).lexically_normal();
}

std::string to_relative( const fs::path& path )
{
	return fs::relative( path, get_source_root() ).generic_string();
}

bool is_allowed( const std::string& relative )
{
	return std::any_of( std::begin( ALLOWED_PLACES ), std::end( ALLOWED_PLACES ), [&] ( const AllowedPlace& a )
	{
		const std::string p = a.path;
		return p.back() == '/' ? relative.rfind( p, 0 ) == 0 : relative == p;
	} );
}

/**
 * コメントと文字列・文字の中身を空白に置き換える（行の数は変えない）
 *
 * #include の行の <> と "" はそのまま残す（読むヘッダを調べるため）
 */
std::string strip_comments_and_strings( const std::string& s )
{
	std::string out = s;
	size_t i = 0;
	bool line_start = true;
	bool in_include = false;

	auto blank = [&] ( size_t from, size_t to ) { for ( size_t k = from; k < to && k < out.size(); ++k ) { if ( out[ k ] != '\n' ) out[ k ] = ' '; } };

	while ( i < s.size() )
	{
		const char c = s[ i ];

		if ( c == '\n' ) { line_start = true; in_include = false; ++i; continue; }

		if ( line_start && c == '#' )
		{
			const size_t end = s.find( '\n', i );
			const std::string line = s.substr( i, end == std::string::npos ? std::string::npos : end - i );
			in_include = std::regex_search( line, std::regex( R"(^#\s*include\b)" ) );
		}

		if ( c != ' ' && c != '\t' && c != '\r' ) line_start = false;

		if ( c == '/' && i + 1 < s.size() && s[ i + 1 ] == '/' )
		{
			const size_t end = s.find( '\n', i );
			blank( i, end == std::string::npos ? s.size() : end );
			i = end == std::string::npos ? s.size() : end;
			continue;
		}

		if ( c == '/' && i + 1 < s.size() && s[ i + 1 ] == '*' )
		{
			const size_t end = s.find( "*/", i + 2 );
			const size_t stop = end == std::string::npos ? s.size() : end + 2;
			blank( i, stop );
			i = stop;
			continue;
		}

		if ( c == 'R' && i + 1 < s.size() && s[ i + 1 ] == '"' && ( i == 0 || ! ( std::isalnum( static_cast< unsigned char >( s[ i - 1 ] ) ) || s[ i - 1 ] == '_' ) ) )
		{
			const size_t open = s.find( '(', i + 2 );
			if ( open != std::string::npos )
			{
				const std::string close = ")" + s.substr( i + 2, open - ( i + 2 ) ) + "\"";
				const size_t end = s.find( close, open + 1 );
				const size_t stop = end == std::string::npos ? s.size() : end + close.size();
				blank( i, stop );
				i = stop;
				continue;
			}
		}

		if ( ( c == '"' || c == '\'' ) && ! in_include )
		{
			size_t k = i + 1;
			while ( k < s.size() && s[ k ] != c && s[ k ] != '\n' )
			{
				k += ( s[ k ] == '\\' ) ? 2 : 1;
			}
			blank( i, std::min( k + 1, s.size() ) );
			i = k + 1;
			continue;
		}

		++i;
	}

	return out;
}

/// Direct3D 11 を知っている印（読むヘッダと、型・名前空間の名前）
const std::regex& get_include_pattern()
{
	static const std::regex r( R"(^\s*#\s*include\s*[<"]([^>"]*(Direct3D11/|d3d11|dxgi|d3dcompiler)[^>"]*)[>"])", std::regex::icase );
	return r;
}

const std::regex& get_name_pattern()
{
	static const std::regex r( R"(\b(ID3D11\w*|IDXGI\w*|D3D11_\w*|D3D_\w*|DXGI_\w*|direct_3d_11|Direct3D11)\b)" );
	return r;
}

struct Finding
{
	std::string place;	///< ファイル:行
	std::string text;	///< その行
};

/// 1 つのファイルの中の印を探す
std::vector< Finding > find_direct3d_usage( const fs::path& path )
{
	std::ifstream in( path, std::ios::binary );
	std::stringstream ss;
	ss << in.rdbuf();
	const std::string original = ss.str();
	const std::string code = strip_comments_and_strings( original );

	std::vector< Finding > findings;
	std::istringstream code_lines( code );
	std::istringstream original_lines( original );
	std::string line, original_line;
	int number = 0;

	while ( std::getline( code_lines, line ) && std::getline( original_lines, original_line ) )
	{
		++number;

		if ( std::regex_search( line, get_include_pattern() ) || std::regex_search( line, get_name_pattern() ) )
		{
			findings.push_back( { to_relative( path ) + ":" + std::to_string( number ), original_line } );
		}
	}

	return findings;
}

/// 調べる .h・.cpp の一覧
std::vector< fs::path > list_source_files()
{
	std::vector< fs::path > files;
	const fs::path root = get_source_root();

	for ( auto it = fs::recursive_directory_iterator( root ); it != fs::recursive_directory_iterator(); ++it )
	{
		const std::string name = it->path().filename().string();

		if ( it->is_directory() )
		{
			if ( it.depth() == 0 && std::find( std::begin( SKIPPED_DIRECTORIES ), std::end( SKIPPED_DIRECTORIES ), name ) != std::end( SKIPPED_DIRECTORIES ) )
			{
				it.disable_recursion_pending();
			}
			continue;
		}

		const std::string ext = it->path().extension().string();

		if ( ext == ".h" || ext == ".cpp" || ext == ".hpp" || ext == ".inl" )
		{
			files.push_back( it->path() );
		}
	}

	return files;
}

} // namespace

TEST( Direct3DBoundaryTest, 決まりの場所の外ではDirect3D11を使わない )
{
	const auto files = list_source_files();
	ASSERT_GT( files.size(), 100u ) << "source のファイルが見つからない : " << get_source_root().string();

	std::vector< Finding > violations;

	for ( const auto& file : files )
	{
		if ( is_allowed( to_relative( file ) ) )
		{
			continue;
		}

		for ( auto& f : find_direct3d_usage( file ) )
		{
			violations.push_back( std::move( f ) );
		}
	}

	std::ostringstream message;
	for ( const auto& v : violations )
	{
		message << v.place << " : " << v.text << '\n';
	}

	EXPECT_TRUE( violations.empty() )
		<< "Direct3D 11 を知ってよい場所（組み立ての根・Direct3D11 のフォルダ・実装を試すテスト）の外で使っている。"
		<< "抽象の窓口（core::graphics の GraphicsManager・Texture・ConstantBufferTyped・VertexBuffer・DebugDraw など）を使うこと\n"
		<< message.str();
}

TEST( Direct3DBoundaryTest, 例外の場所は実際にDirect3D11を使っている )
{
	// 使わなくなった例外を残さない（表が古くなると、決まりがゆるんだままになる）。調べる仕組みが働いていることも確かめる
	const auto files = list_source_files();

	for ( const auto& place : ALLOWED_PLACES )
	{
		const std::string p = place.path;
		bool used = false;

		for ( const auto& file : files )
		{
			const std::string relative = to_relative( file );
			const bool inside = p.back() == '/' ? relative.rfind( p, 0 ) == 0 : relative == p;

			if ( inside && ! find_direct3d_usage( file ).empty() )
			{
				used = true;
				break;
			}
		}

		EXPECT_TRUE( used ) << "例外の場所で Direct3D 11 を使っていない（表から外す）: " << p << "（" << place.reason << "）";
	}
}

TEST( Direct3DBoundaryTest, コメントと文字列の中は数えない )
{
	const std::string code =
		"// ID3D11Device はコメント\n"
		"/* DXGI_FORMAT も\n direct_3d_11 も */\n"
		"const char* s = \"Direct3D11\";\n"
		"auto r = R\"x(D3D11_USAGE)x\";\n"
		"#include <core/graphics/Texture.h>\n"
		"int ok = 0;\n";

	const std::string stripped = strip_comments_and_strings( code );

	EXPECT_FALSE( std::regex_search( stripped, get_name_pattern() ) ) << stripped;
	EXPECT_EQ( std::count( code.begin(), code.end(), '\n' ), std::count( stripped.begin(), stripped.end(), '\n' ) );

	EXPECT_TRUE( std::regex_search( strip_comments_and_strings( "#include <core/graphics/Direct3D11/Texture.h>\n" ), get_include_pattern() ) );
	EXPECT_TRUE( std::regex_search( strip_comments_and_strings( "#include <d3d11.h>\n" ), get_include_pattern() ) );
	EXPECT_TRUE( std::regex_search( strip_comments_and_strings( "ID3D11Texture2D* t = nullptr;\n" ), get_name_pattern() ) );
	EXPECT_TRUE( std::regex_search( strip_comments_and_strings( "core::graphics::direct_3d_11::Texture* t;\n" ), get_name_pattern() ) );
}

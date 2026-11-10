#include "pch.h"

#include <blue_sky/stage/StoryTextParser.h>

#include <algorithm>
#include <filesystem>

/**
 * ステージの前後の文と背景（.intro.toml・.outro.toml）を読む StoryTextParser のテスト
 *
 * 書き方は doc/ステージ形式の設計.md 7 章。失敗の扱い : 文法の誤り・読めない版は例外、知らないキー・型の違いは
 * 「ファイル:行:列」付きの警告に残してそのキーだけ飛ばす
 */

using blue_sky::stage::StoryTextParser;
using blue_sky::stage::StoryTextData;

namespace
{

StoryTextData parse( const char* text )
{
	return StoryTextParser::parse_text( text, "t.intro.toml" );
}

/// 警告のどれかに、すべての語が入っているか
bool has_warning( const StoryTextData& data, std::initializer_list< const char* > words )
{
	return std::any_of( data.warnings.begin(), data.warnings.end(), [ & ] ( const string_t& w )
	{
		return std::all_of( words.begin(), words.end(), [ & ] ( const char* word ) { return w.find( word ) != string_t::npos; } );
	} );
}

string_t all_warnings( const StoryTextData& data )
{
	string_t s;

	for ( const auto& w : data.warnings )
	{
		s += w + "\n";
	}

	return s;
}

std::filesystem::path media_path( const string_t& relative )
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media" / relative ).lexically_normal();
}

} // namespace

TEST( StoryTextParserTest, 書き方のとおりに読む )
{
	const StoryTextData data = parse( R"(
format = 1
sound = "1-1-intro"
bgm = { file = "opening", volume = 0.5 }
next_scene = "ending"
next_stage = "2-0"

[[layer]]
image = "noise.jpg"
scale = { from = 1.6, to = 3, speed = 0.0001 }
rotation = { from = 0, to = 180, speed = 0.1 }

[[layer]]
image = "bg.png"
src_rect = [0, 0, 640, 480]
translation = { from = [1, 2], to = [3, 4.5], speed = [0.5, 0.25] }
color = { from = [1, 1, 1, 0], to = [1, 1, 1, 1], speed = [0, 0, 0, 0.01] }

[text]
color = 0xFFFFFFCC
border_color = 0x000000CC
body = """
一行目

三行目
"""
)" );

	EXPECT_TRUE( data.warnings.empty() ) << all_warnings( data );

	EXPECT_EQ( data.sound, "1-1-intro" );
	ASSERT_TRUE( data.bgm.has_value() );
	EXPECT_EQ( data.bgm->file, "opening" );
	EXPECT_DOUBLE_EQ( data.bgm->volume, 0.5 );
	EXPECT_EQ( data.next_scene, "ending" );
	EXPECT_EQ( data.next_stage, "2-0" );

	ASSERT_EQ( data.layers.size(), 2u );

	// 1 枚目 : 大きさと回転（書いた順に描く）
	const auto& a = data.layers[ 0 ];
	EXPECT_EQ( a.image, "noise.jpg" );
	EXPECT_FALSE( a.src_rect.has_value() );
	ASSERT_TRUE( a.scale.has_value() );
	EXPECT_DOUBLE_EQ( ( * a.scale->from )[ 0 ], 1.6 );
	EXPECT_DOUBLE_EQ( ( * a.scale->to )[ 0 ], 3.0 );
	EXPECT_DOUBLE_EQ( ( * a.scale->speed )[ 0 ], 0.0001 );
	ASSERT_TRUE( a.rotation.has_value() );
	EXPECT_DOUBLE_EQ( ( * a.rotation->from )[ 0 ], 0.0 );
	EXPECT_DOUBLE_EQ( ( * a.rotation->to )[ 0 ], 180.0 );
	EXPECT_DOUBLE_EQ( ( * a.rotation->speed )[ 0 ], 0.1 );
	EXPECT_FALSE( a.translation.has_value() );
	EXPECT_FALSE( a.color.has_value() );
	EXPECT_EQ( a.where.line, 8 );

	// 2 枚目 : 切り出し・位置・色
	const auto& b = data.layers[ 1 ];
	EXPECT_EQ( b.image, "bg.png" );
	ASSERT_TRUE( b.src_rect.has_value() );
	EXPECT_EQ( * b.src_rect, ( std::array< int, 4 >{ 0, 0, 640, 480 } ) );
	ASSERT_TRUE( b.translation.has_value() );
	EXPECT_EQ( * b.translation->to, ( std::array< double, 2 >{ 3.0, 4.5 } ) );
	ASSERT_TRUE( b.color.has_value() );
	EXPECT_EQ( * b.color->speed, ( std::array< double, 4 >{ 0.0, 0.0, 0.0, 0.01 } ) );

	EXPECT_EQ( data.text_color, 0xFFFFFFCCu );
	EXPECT_EQ( data.text_border_color, 0x000000CCu );
	// 複数行の文字列は、""" の直後の改行を除いて書いたまま（末尾の改行も本文の高さに入る）
	EXPECT_EQ( data.text, "一行目\n\n三行目\n" );
}

TEST( StoryTextParserTest, 書いていない所は空のまま )
{
	const StoryTextData data = parse( "format = 1\nsound = \"switch-off\"\n" );

	EXPECT_TRUE( data.warnings.empty() ) << all_warnings( data );
	EXPECT_EQ( data.sound, "switch-off" );
	EXPECT_TRUE( data.layers.empty() );
	EXPECT_FALSE( data.bgm.has_value() );
	EXPECT_FALSE( data.text_color.has_value() );
	EXPECT_FALSE( data.text_border_color.has_value() );
	EXPECT_TRUE( data.text.empty() );
	EXPECT_FALSE( data.next_scene.has_value() );
	EXPECT_FALSE( data.next_stage.has_value() );
}

TEST( StoryTextParserTest, 動きの書いていない所は空のまま )
{
	const StoryTextData data = parse( "format = 1\n[[layer]]\nimage = \"a.png\"\nscale = { to = 2 }\n" );

	EXPECT_TRUE( data.warnings.empty() ) << all_warnings( data );
	ASSERT_EQ( data.layers.size(), 1u );
	ASSERT_TRUE( data.layers[ 0 ].scale.has_value() );
	EXPECT_FALSE( data.layers[ 0 ].scale->from.has_value() );
	EXPECT_DOUBLE_EQ( ( * data.layers[ 0 ].scale->to )[ 0 ], 2.0 );
	EXPECT_FALSE( data.layers[ 0 ].scale->speed.has_value() );
}

TEST( StoryTextParserTest, 知らないキーは行を付けて警告しそのキーだけ飛ばす )
{
	const StoryTextData data = parse( R"(format = 1
sound = "a"
voice = "b"

[[layer]]
image = "noise.jpg"
layer_rotate = { from = 0, to = 180, speed = 0.1 }
scale = { from = 1.6, until = 3 }

[text]
colour = 0xFFFFFFFF
body = "x"
)" );

	EXPECT_EQ( data.sound, "a" );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:3:", "voice" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:7:", "layer_rotate" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:8:", "until" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:11:", "colour" } ) ) << all_warnings( data );
	EXPECT_EQ( data.warnings.size(), 4u ) << all_warnings( data );

	// 重なりは残り、読めたキーは当たる
	ASSERT_EQ( data.layers.size(), 1u );
	EXPECT_FALSE( data.layers[ 0 ].rotation.has_value() );
	ASSERT_TRUE( data.layers[ 0 ].scale.has_value() );
	EXPECT_DOUBLE_EQ( ( * data.layers[ 0 ].scale->from )[ 0 ], 1.6 );
	EXPECT_FALSE( data.layers[ 0 ].scale->to.has_value() );
	EXPECT_EQ( data.text, "x" );
}

TEST( StoryTextParserTest, 型の違いは行を付けて警告しそのキーだけ飛ばす )
{
	const StoryTextData data = parse( R"(format = 1
sound = 3
bgm = "opening"

[[layer]]
image = "noise.jpg"
src_rect = [0, 0, 640]
scale = { from = "1.6", to = 3 }
translation = { from = [1, 2, 3] }
rotation = 180

[text]
color = "FFFFFFCC"
body = 1
)" );

	EXPECT_FALSE( data.sound.has_value() );
	EXPECT_FALSE( data.bgm.has_value() );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:2:", "sound", "文字列" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:3:", "bgm" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:7:", "src_rect" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:8:", "scale.from" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:9:", "translation.from" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:10:", "rotation" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:13:", "text.color" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.intro.toml:14:", "text.body" } ) ) << all_warnings( data );
	EXPECT_EQ( data.warnings.size(), 8u ) << all_warnings( data );

	ASSERT_EQ( data.layers.size(), 1u );
	const auto& layer = data.layers[ 0 ];
	EXPECT_FALSE( layer.src_rect.has_value() );
	EXPECT_FALSE( layer.rotation.has_value() );
	ASSERT_TRUE( layer.scale.has_value() );
	EXPECT_FALSE( layer.scale->from.has_value() );
	EXPECT_DOUBLE_EQ( ( * layer.scale->to )[ 0 ], 3.0 );
	EXPECT_FALSE( data.text_color.has_value() );
	EXPECT_TRUE( data.text.empty() );
}

TEST( StoryTextParserTest, 画像の無い重なりは飛ばす )
{
	const StoryTextData data = parse( "format = 1\n[[layer]]\nscale = { from = 2 }\n[[layer]]\nimage = \"a.png\"\n" );

	EXPECT_TRUE( has_warning( data, { "t.intro.toml:2:", "image" } ) ) << all_warnings( data );
	ASSERT_EQ( data.layers.size(), 1u );
	EXPECT_EQ( data.layers[ 0 ].image, "a.png" );
}

TEST( StoryTextParserTest, 文法の誤りと読めない版は例外 )
{
	EXPECT_THROW( parse( "format = 1\nsound = \"a\n" ), std::exception );
	EXPECT_THROW( parse( "format = 2\n" ), std::exception );
	EXPECT_THROW( StoryTextParser::parse_file( "media/stage/無いファイル.intro.toml" ), std::exception );

	try
	{
		parse( "format = 1\n\nsound = = \"a\"\n" );
		FAIL();
	}
	catch ( const std::exception& e )
	{
		EXPECT_NE( string_t( e.what() ).find( "t.intro.toml:3:" ), string_t::npos ) << e.what();
	}
}

TEST( StoryTextParserTest, formatが無ければ警告して読む )
{
	const StoryTextData data = parse( "sound = \"a\"\n" );

	EXPECT_TRUE( has_warning( data, { "format" } ) ) << all_warnings( data );
	EXPECT_EQ( data.sound, "a" );
}

TEST( StoryTextFilesTest, 同梱の全introとoutroが警告0で読める )
{
	size_t count = 0;

	for ( const auto& entry : std::filesystem::directory_iterator( media_path( "stage" ) ) )
	{
		const string_t name = entry.path().filename().string();
		const bool is_story = name.size() > 11 && ( name.compare( name.size() - 11, 11, ".intro.toml" ) == 0 || name.compare( name.size() - 11, 11, ".outro.toml" ) == 0 );

		if ( ! is_story )
		{
			continue;
		}

		SCOPED_TRACE( name );

		const StoryTextData data = StoryTextParser::parse_file( entry.path().string() );

		EXPECT_TRUE( data.warnings.empty() ) << all_warnings( data );
		// どれも音声か本文のどちらかで終わる（どちらも無いと場面が終わらない）
		EXPECT_TRUE( data.sound.has_value() || ! data.text.empty() );

		++count;
	}

	// 12 ステージの intro・outro（増やしたら数も直す）
	EXPECT_EQ( count, 24u );
}

TEST( StoryTextFilesTest, 同梱のファイルの中身が古い形式から変換したときと同じ )
{
	// 0-0.intro : 本文と色（以前の読み方は 1 行ごとに改行を足し、ファイルの終わりの改行の後にもう 1 つ足していた）
	{
		const StoryTextData data = StoryTextParser::parse_file( media_path( "stage/0-0.intro.toml" ).string() );

		EXPECT_FALSE( data.sound.has_value() );
		EXPECT_EQ( data.text, "ある日、少年は空を見上げた。\n\n青く広がる空の向こうに\n何かが待っている気がした。\n\nこれは、その小さな冒険の物語。\n\n" );
		EXPECT_EQ( data.text_color, 0xFFFFFFCCu );
		EXPECT_EQ( data.text_border_color, 0x000000CCu );
		ASSERT_EQ( data.layers.size(), 1u );
	}

	// 2-3.intro : 音だけ（背景は既定の画像）
	{
		const StoryTextData data = StoryTextParser::parse_file( media_path( "stage/2-3.intro.toml" ).string() );

		EXPECT_EQ( data.sound, "switch-off" );
		EXPECT_TRUE( data.layers.empty() );
		EXPECT_TRUE( data.text.empty() );
	}

	// ほかの 22 個 : 雑音の画像を 1 枚、大きさ 1.6 → 3、回転 0 → 180（以前は layer-rotate と書いて無視されていた）、音声はファイルと同じ名前
	for ( const char* stage : { "0-0", "0-1", "0-2", "0-3", "1-0", "1-1", "1-2", "1-3", "2-0", "2-1", "2-2", "2-3" } )
	{
		for ( const char* kind : { "intro", "outro" } )
		{
			const string_t name = string_t( stage ) + "." + kind;

			if ( name == "0-0.intro" || name == "2-3.intro" )
			{
				continue;
			}

			SCOPED_TRACE( name );

			const StoryTextData data = StoryTextParser::parse_file( media_path( "stage/" + name + ".toml" ).string() );

			EXPECT_EQ( data.sound, string_t( stage ) + "-" + kind );
			EXPECT_TRUE( data.text.empty() );
			ASSERT_EQ( data.layers.size(), 1u );

			const auto& layer = data.layers[ 0 ];
			EXPECT_EQ( layer.image, "noise.jpg" );
			ASSERT_TRUE( layer.scale.has_value() );
			EXPECT_DOUBLE_EQ( ( * layer.scale->from )[ 0 ], 1.6 );
			EXPECT_DOUBLE_EQ( ( * layer.scale->to )[ 0 ], 3.0 );
			EXPECT_DOUBLE_EQ( ( * layer.scale->speed )[ 0 ], 0.0001 );
			ASSERT_TRUE( layer.rotation.has_value() );
			EXPECT_DOUBLE_EQ( ( * layer.rotation->from )[ 0 ], 0.0 );
			EXPECT_DOUBLE_EQ( ( * layer.rotation->to )[ 0 ], 180.0 );
			EXPECT_DOUBLE_EQ( ( * layer.rotation->speed )[ 0 ], 0.1 );
		}
	}
}

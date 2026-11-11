#include "pch.h"

#include <blue_sky/stage/TomlStageParser.h>

#include <filesystem>
#include <map>

/**
 * 同梱のステージ（media/stage の 12 個・media/stage_ext の 4 個の .stage.toml）のテスト
 *
 * - どれも警告 0 で読める（知らない種類・動き・キー・型の違い・見つからない名前・同じ名前が無い）。
 *   古い形式から変換したときに捨てた物（どこも読まない行・余った数・name の 2 語目・無い名前を指す動き）は、
 *   変換ツール（tools/stage_convert/convert_stage.py）が TOML に書かずコメントで残したので、読んでも警告にならない
 * - 物体の数・種類ごとの数・イベントの数・名前の数・始めの動きの数が、古い形式（.stage）から変換したときと同じ
 *   （古い形式を消す前に、古い読み方と TOML の読み方の結果が同じことを確かめてある。doc/ステージ形式の設計.md 6 章）
 */

using blue_sky::stage::TomlStageParser;
using blue_sky::stage::StageData;

namespace
{

std::filesystem::path media_path( const string_t& relative )
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media" / relative ).lexically_normal();
}

struct StageExpectation
{
	const char*	file;
	size_t		objects;		///< 物体の数（プレイヤー・ゴールは含まない）
	const char*	kinds;			///< 種類ごとの数（名前の順）
	const char*	events;			///< イベントを持つ物体ごとのイベントの数（書いた順）
	size_t		names;			///< 名前の数
	size_t		start_actions;	///< 始めるたびに行う動きの数
};

const StageExpectation expectations[] =
{
	{ "stage/0-0.stage.toml", 44, "balloon=42 medal=1 static=1", "", 0, 3 },
	{ "stage/0-1.stage.toml", 29, "area-switch=2 balloon=21 girl=1 ladder=3 medal=1 static=1", "5,1", 0, 6 },
	{ "stage/0-2.stage.toml", 54, "balloon=36 medal=1 rocket=16 static=1", "", 0, 0 },
	{ "stage/0-3.stage.toml", 3, "area-switch=2 medal=1", "1,1", 0, 6 },
	{ "stage/1-0.stage.toml", 9, "area-switch=1 ladder=1 medal=1 rocket=1 static=1 stone=4", "5", 1, 6 },
	{ "stage/1-1.stage.toml", 16, "medal=1 static=10 stone=3 switch=2", "4,3", 4, 5 },
	{ "stage/1-2.stage.toml", 19, "medal=1 robot=10 static=2 stone=5 switch=1", "2", 3, 7 },
	{ "stage/1-3.stage.toml", 8, "area-switch=1 ladder=4 medal=1 static=1 switch=1", "11,1", 1, 11 },
	{ "stage/2-0.stage.toml", 32, "balloon=1 ladder=10 medal=1 rocket=4 static=5 stone=10 switch=1", "2", 1, 8 },
	{ "stage/2-1.stage.toml", 24, "area-switch=1 girl=1 medal=1 static=1 stone=8 switch=12", "3,3,5,5,4,4,2,4,3,3,9,3,32", 14, 7 },
	{ "stage/2-2.stage.toml", 6, "area-switch=2 medal=1 static=3", "2,1", 0, 3 },
	{ "stage/2-3.stage.toml", 51, "balloon=48 girl=1 medal=1 robot=1", "", 1, 13 },
	{ "stage_ext/ext-0.stage.toml", 33, "balloon=18 robot=6 rocket=8 static=1", "", 8, 22 },
	{ "stage_ext/ext-1.stage.toml", 21, "robot=16 static=2 stone=2 switch=1", "2", 1, 5 },
	{ "stage_ext/ext-2.stage.toml", 60, "balloon=45 rocket=14 static=1", "", 0, 6 },
	{ "stage_ext/staff.stage.toml", 16, "balloon=13 rocket=2 static=1", "", 0, 3 },
};

} // namespace

TEST( StageFilesTest, 同梱の全ステージが警告0で読める )
{
	size_t count = 0;

	for ( const char* dir : { "stage", "stage_ext" } )
	{
		for ( const auto& entry : std::filesystem::directory_iterator( media_path( dir ) ) )
		{
			const string_t name = entry.path().filename().string();

			if ( name.size() < 11 || name.compare( name.size() - 11, 11, ".stage.toml" ) != 0 )
			{
				continue;
			}

			SCOPED_TRACE( name );

			const StageData data = TomlStageParser::parse_file( entry.path().string() );

			string_t warnings;

			for ( const auto& w : data.warnings )
			{
				warnings += w + "\n";
			}

			EXPECT_TRUE( data.warnings.empty() ) << warnings;
			EXPECT_TRUE( data.player.has_value() );
			EXPECT_TRUE( data.goal.has_value() );

			++count;
		}
	}

	// ステージの数（増やしたら期待値の表にも足す）
	EXPECT_EQ( count, std::size( expectations ) );
}

TEST( StageFilesTest, 同梱の16個の物体とイベントの数が合う )
{
	for ( const auto& e : expectations )
	{
		SCOPED_TRACE( e.file );

		const StageData data = TomlStageParser::parse_file( media_path( e.file ).string() );

		EXPECT_EQ( data.objects.size(), e.objects );

		std::map< string_t, int > kinds;
		string_t events;
		size_t names = 0;

		for ( const auto& o : data.objects )
		{
			kinds[ o.type ]++;
			names += o.names.size();

			if ( ! o.events.empty() )
			{
				events += ( events.empty() ? "" : "," ) + std::to_string( o.events.size() );
			}
		}

		string_t kinds_text;

		for ( const auto& [ kind, count ] : kinds )
		{
			kinds_text += ( kinds_text.empty() ? "" : " " ) + kind + "=" + std::to_string( count );
		}

		EXPECT_EQ( kinds_text, e.kinds );
		EXPECT_EQ( events, e.events );
		EXPECT_EQ( names, e.names );
		EXPECT_EQ( data.start_actions.size(), e.start_actions );
	}
}

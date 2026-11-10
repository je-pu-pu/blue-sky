#include "pch.h"

#include <blue_sky/stage/LegacyStageParser.h>

#include <common/exception.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>

/**
 * ステージの今の形式（.stage）を読む LegacyStageParser のテスト
 *
 * 同梱の 16 個（media/stage の 12 個・media/stage_ext の 4 個）を読み、物体の数・種類ごとの数・イベントの数・始めの動きの数を確かめる。
 * 読み方の決まり（足りない数は 0・余った数は捨てて警告・name は 1 語目だけ など）は以前の GamePlayScene::load_stage_file と同じ
 */

using blue_sky::stage::LegacyStageParser;
using blue_sky::stage::StageData;

namespace
{

/// source/media の下のファイルの場所（テストの実行場所によらない）
std::filesystem::path media_path( const char* relative )
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media" / relative ).lexically_normal();
}

StageData read_stage( const char* relative )
{
	std::ifstream in( media_path( relative ) );
	EXPECT_TRUE( in.good() ) << relative;
	return LegacyStageParser::parse( in, relative );
}

/// 期待値（tools で数えたもの）
struct StageExpectation
{
	const char*	file;
	size_t		objects;		///< 物体の数（プレイヤー・ゴールは含まない）
	const char*	kinds;			///< 種類ごとの数（名前の順）
	const char*	events;			///< イベントを持つ物体ごとのイベントの数（書いた順）
	size_t		names;			///< name 行の数
	size_t		start_actions;	///< exec 行の数
};

const StageExpectation expectations[] =
{
	{ "stage/0-0.stage", 44, "balloon=42 medal=1 static=1", "", 0, 3 },
	{ "stage/0-1.stage", 29, "area-switch=2 balloon=21 girl=1 ladder=3 medal=1 static=1", "5,1", 0, 6 },
	{ "stage/0-2.stage", 54, "balloon=36 medal=1 rocket=16 static=1", "", 0, 0 },
	{ "stage/0-3.stage", 3, "area-switch=2 medal=1", "1,1", 0, 6 },
	{ "stage/1-0.stage", 9, "area-switch=1 ladder=1 medal=1 rocket=1 static=1 stone=4", "5", 1, 6 },
	{ "stage/1-1.stage", 16, "medal=1 static=10 stone=3 switch=2", "4,3", 4, 5 },
	{ "stage/1-2.stage", 19, "medal=1 robot=10 static=2 stone=5 switch=1", "2", 3, 7 },
	{ "stage/1-3.stage", 8, "area-switch=1 ladder=4 medal=1 static=1 switch=1", "11,1", 1, 11 },
	{ "stage/2-0.stage", 32, "balloon=1 ladder=10 medal=1 rocket=4 static=5 stone=10 switch=1", "2", 1, 8 },
	{ "stage/2-1.stage", 24, "area-switch=1 girl=1 medal=1 static=1 stone=8 switch=12", "3,3,5,5,4,4,2,4,3,3,9,3,33", 14, 7 },
	{ "stage/2-2.stage", 6, "area-switch=2 medal=1 static=3", "2,1", 0, 3 },
	{ "stage/2-3.stage", 51, "balloon=48 girl=1 medal=1 robot=1", "", 1, 13 },
	{ "stage_ext/ext-0.stage", 33, "balloon=18 robot=6 rocket=8 static=1", "", 8, 22 },
	{ "stage_ext/ext-1.stage", 21, "robot=16 static=2 stone=2 switch=1", "2", 1, 5 },
	{ "stage_ext/ext-2.stage", 60, "balloon=45 rocket=14 static=1", "", 0, 6 },
	{ "stage_ext/staff.stage", 16, "balloon=13 rocket=2 static=1", "", 0, 3 },
};

} // namespace

TEST( LegacyStageParserTest, 同梱の16個を読み_物体とイベントの数が合う )
{
	for ( const auto& e : expectations )
	{
		SCOPED_TRACE( e.file );

		const StageData data = read_stage( e.file );

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

		// どのステージにもプレイヤーとゴールの位置がある
		EXPECT_TRUE( data.player.has_value() );
		EXPECT_TRUE( data.goal.has_value() );
	}
}

TEST( LegacyStageParserTest, ステージ2_1の中身 )
{
	const StageData data = read_stage( "stage/2-1.stage" );

	EXPECT_EQ( data.collision, "2-1.obj" );
	EXPECT_EQ( data.ground, "ground" );
	ASSERT_TRUE( data.player.has_value() );
	EXPECT_DOUBLE_EQ( data.player->position[ 1 ], 101.0 );
	ASSERT_TRUE( data.player->camera_direction.has_value() );
	EXPECT_DOUBLE_EQ( * data.player->camera_direction, 60.0 );

	// 最初の物体は背景（object stage-2-1 → 名前 bg）
	ASSERT_FALSE( data.objects.empty() );
	EXPECT_EQ( data.objects[ 0 ].type, "static" );
	EXPECT_EQ( data.objects[ 0 ].model, "stage-2-1" );
	ASSERT_EQ( data.objects[ 0 ].names.size(), 1u );
	EXPECT_EQ( data.objects[ 0 ].names[ 0 ], "bg" );

	// 「メダルを取れる場所にプレイヤーを運ぶ」は、位置の決まりどおり switch-e に付く（設計 1.4 の 4）
	const auto e = std::find_if( data.objects.begin(), data.objects.end(), [] ( const auto& o ) { return ! o.names.empty() && o.names[ 0 ] == "switch-e"; } );
	ASSERT_NE( e, data.objects.end() );
	ASSERT_EQ( e->events.size(), 4u );
	EXPECT_EQ( e->events[ 3 ].event, "turn-on" );
	EXPECT_EQ( e->events[ 3 ].action.name, "game_object.set_target_location" );
	EXPECT_EQ( e->events[ 3 ].action.params, "player 8.0 115.0 -89.0 10" );
	EXPECT_EQ( e->events[ 3 ].action.where.line, 93 );

	// 読まない行は無い（警告 0）
	EXPECT_TRUE( data.warnings.empty() );
}

TEST( LegacyStageParserTest, 足りない数は0_余った数は捨てて警告 )
{
	const StageData data = LegacyStageParser::parse_text(
		"balloon 1 2\n"
		"balloon 1 2 3 90 5 6\n"
		"object box-2x2x2 1 2 3 45\n", "t.stage" );

	ASSERT_EQ( data.objects.size(), 3u );

	EXPECT_EQ( data.objects[ 0 ].position, ( blue_sky::stage::Vector3d{ 1, 2, 0 } ) );
	EXPECT_DOUBLE_EQ( data.objects[ 0 ].direction, 0.0 );

	EXPECT_EQ( data.objects[ 1 ].position, ( blue_sky::stage::Vector3d{ 1, 2, 3 } ) );
	EXPECT_DOUBLE_EQ( data.objects[ 1 ].direction, 90.0 );

	// 動かない物 : 回転の x が向きにもなる
	EXPECT_EQ( data.objects[ 2 ].type, "static" );
	EXPECT_EQ( data.objects[ 2 ].model, "box-2x2x2" );
	EXPECT_EQ( data.objects[ 2 ].rotation, ( blue_sky::stage::Vector3d{ 45, 0, 0 } ) );
	EXPECT_DOUBLE_EQ( data.objects[ 2 ].direction, 45.0 );

	ASSERT_EQ( data.warnings.size(), 1u );
	EXPECT_NE( data.warnings[ 0 ].find( "t.stage:2" ), string_t::npos );
	EXPECT_NE( data.warnings[ 0 ].find( "5 6" ), string_t::npos );
}

TEST( LegacyStageParserTest, nameは1語目だけ_eventとnameは直前の物体に付く )
{
	const StageData data = LegacyStageParser::parse_text(
		"switch 0 0 0\n"
		"name switch switch-a\n"
		"player 1 2 3\n"
		"event turn-on play_sound a\n"
		"exec change_bgm road\n", "t.stage" );

	ASSERT_EQ( data.objects.size(), 1u );
	ASSERT_EQ( data.objects[ 0 ].names.size(), 1u );
	EXPECT_EQ( data.objects[ 0 ].names[ 0 ], "switch" );

	// player 行は物体ではないので、event は前のスイッチに付く（以前と同じ）
	ASSERT_EQ( data.objects[ 0 ].events.size(), 1u );
	EXPECT_EQ( data.objects[ 0 ].events[ 0 ].action.name, "play_sound" );
	EXPECT_EQ( data.objects[ 0 ].events[ 0 ].action.params, "a" );

	ASSERT_EQ( data.start_actions.size(), 1u );
	EXPECT_EQ( data.start_actions[ 0 ].name, "change_bgm" );
	EXPECT_EQ( data.start_actions[ 0 ].params, "road" );

	// カメラの向きを書いていないプレイヤー
	ASSERT_TRUE( data.player.has_value() );
	EXPECT_FALSE( data.player->camera_direction.has_value() );

	ASSERT_EQ( data.warnings.size(), 1u );
	EXPECT_NE( data.warnings[ 0 ].find( "switch-a" ), string_t::npos );
}

TEST( LegacyStageParserTest, 物体の無い所のeventとnameは例外 )
{
	EXPECT_THROW( LegacyStageParser::parse_text( "event turn-on play_sound a\n", "t.stage" ), std::exception );
	EXPECT_THROW( LegacyStageParser::parse_text( "name a\n", "t.stage" ), std::exception );
}

TEST( LegacyStageParserTest, 設定の行_bgm_loopだけを読み_ほかは警告 )
{
	const StageData data = LegacyStageParser::parse_text(
		"bgm.loop 0\n"
		"sky-box sky-box-3\n"
		"# コメント\n"
		"\n"
		"bgm ending 1 90\n", "t.stage" );

	ASSERT_TRUE( data.bgm_loop.has_value() );
	EXPECT_FALSE( * data.bgm_loop );
	EXPECT_EQ( data.bgm, "ending" );
	ASSERT_TRUE( data.bgm_bpm.has_value() );
	EXPECT_DOUBLE_EQ( * data.bgm_bpm, 90.0 );

	ASSERT_EQ( data.warnings.size(), 1u );
	EXPECT_NE( data.warnings[ 0 ].find( "sky-box" ), string_t::npos );
}

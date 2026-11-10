#include "pch.h"

#include <blue_sky/stage/TomlStageParser.h>

#include <algorithm>

/**
 * ステージの TOML（.stage.toml）を読む TomlStageParser のテスト
 *
 * 書き方は doc/ステージ形式の設計.md 2.2、失敗の扱いは 3.4（例外・警告に残して飛ばす・名前の参照はまとめて警告）
 */

using blue_sky::stage::TomlStageParser;
using blue_sky::stage::StageData;
using blue_sky::stage::ActionType;
using blue_sky::stage::Vector3d;

namespace
{

StageData parse( const char* text )
{
	return TomlStageParser::parse_text( text, "t.stage.toml" );
}

/// 警告のどれかに、すべての語が入っているか
bool has_warning( const StageData& data, std::initializer_list< const char* > words )
{
	return std::any_of( data.warnings.begin(), data.warnings.end(), [ & ] ( const string_t& w )
	{
		return std::all_of( words.begin(), words.end(), [ & ] ( const char* word ) { return w.find( word ) != string_t::npos; } );
	} );
}

string_t all_warnings( const StageData& data )
{
	string_t s;

	for ( const auto& w : data.warnings )
	{
		s += w + "\n";
	}

	return s;
}

} // namespace

TEST( TomlStageParserTest, 書き方のとおりに読む )
{
	const StageData data = parse( R"(
format = 1

[stage]
collision = "2-1.obj"
ground = "ground"
bgm = "ending"
bgm_bpm = 90
bgm_loop = false

[player]
position = [0.0, 101.0, -88.0]
camera_direction = 60

[goal]
position = [0, 110, -40]

[[event]]
when = "start"
then = [
  { do = "set", sky_box = "3" },
  { do = "move_to", target = "girl", position = [0.0, 99.0, -45.0], speed = 10, every = 1, count = 99999 },
]

[[object]]
type = "static"
model = "stage-2-1"
name = "bg"
position = [0, 0, 0]
rotation = [45, 0, 0]

[[object]]
type = "stone"
direction = 90
positions = [ [1, 2, 3], [4, 5, 6] ]

[[object]]
type = "girl"
position = [0, 99, -45]

[[object]]
type = "switch"
name = "switch-e"
position = [-6.0, 99.0, -58.0]
direction = 180
on_turn_on = [
  { do = "play_sound", sound = "boku-switch-5" },
  # コメントも書ける
  { do = "switch", target = "switch-e", state = "broken" },
  { do = "set", sky_box = "3", delay = 5 },
]

[[object]]
type = "area-switch"
position = [0, 99, -70.5]
size = [2, 2, 2]
on_turn_off = [ { do = "end_stage" } ]
)" );

	EXPECT_TRUE( data.warnings.empty() ) << all_warnings( data );

	EXPECT_EQ( data.collision, "2-1.obj" );
	EXPECT_EQ( data.ground, "ground" );
	EXPECT_EQ( data.bgm, "ending" );
	ASSERT_TRUE( data.bgm_bpm.has_value() );
	EXPECT_DOUBLE_EQ( * data.bgm_bpm, 90.0 );
	ASSERT_TRUE( data.bgm_loop.has_value() );
	EXPECT_FALSE( * data.bgm_loop );

	ASSERT_TRUE( data.player.has_value() );
	EXPECT_EQ( data.player->position, ( Vector3d{ 0, 101, -88 } ) );
	EXPECT_DOUBLE_EQ( data.player->camera_direction.value_or( 0 ), 60.0 );
	ASSERT_TRUE( data.goal.has_value() );
	EXPECT_EQ( * data.goal, ( Vector3d{ 0, 110, -40 } ) );

	// まとめ書き（positions）は、その数だけの物体になる（順番も書いた順）
	ASSERT_EQ( data.objects.size(), 6u );
	EXPECT_EQ( data.objects[ 0 ].type, "static" );
	EXPECT_EQ( data.objects[ 0 ].model, "stage-2-1" );
	EXPECT_DOUBLE_EQ( data.objects[ 0 ].direction, 45.0 );	// 動かない物の向きは回転の x
	EXPECT_EQ( data.objects[ 1 ].type, "stone" );
	EXPECT_EQ( data.objects[ 1 ].position, ( Vector3d{ 1, 2, 3 } ) );
	EXPECT_EQ( data.objects[ 2 ].position, ( Vector3d{ 4, 5, 6 } ) );
	EXPECT_DOUBLE_EQ( data.objects[ 2 ].direction, 90.0 );

	const auto& e = data.objects[ 4 ];
	ASSERT_EQ( e.names.size(), 1u );
	EXPECT_EQ( e.names[ 0 ], "switch-e" );
	ASSERT_EQ( e.events.size(), 3u );
	EXPECT_EQ( e.events[ 0 ].event, "turn-on" );
	EXPECT_EQ( e.events[ 0 ].action.type, ActionType::PLAY_SOUND );
	EXPECT_EQ( e.events[ 0 ].action.text( "sound" ), "boku-switch-5" );
	EXPECT_EQ( e.events[ 1 ].action.type, ActionType::SWITCH );
	EXPECT_EQ( e.events[ 1 ].action.text( "state" ), "broken" );
	EXPECT_FALSE( e.events[ 1 ].action.is_timed );
	EXPECT_TRUE( e.events[ 2 ].action.is_timed );
	EXPECT_DOUBLE_EQ( e.events[ 2 ].action.interval, 5.0 );
	EXPECT_EQ( e.events[ 2 ].action.count, 1 );

	// 動きの場所（ファイル:行）
	EXPECT_EQ( e.events[ 0 ].action.where.file, "t.stage.toml" );
	EXPECT_EQ( e.events[ 0 ].action.where.line, 47 );

	ASSERT_EQ( data.objects[ 5 ].events.size(), 1u );
	EXPECT_EQ( data.objects[ 5 ].events[ 0 ].event, "turn-off" );
	EXPECT_EQ( data.objects[ 5 ].size, ( Vector3d{ 2, 2, 2 } ) );

	ASSERT_EQ( data.start_actions.size(), 2u );
	EXPECT_EQ( data.start_actions[ 0 ].type, ActionType::SET );
	EXPECT_EQ( data.start_actions[ 0 ].text( "sky_box" ), "3" );
	const auto& move = data.start_actions[ 1 ];
	EXPECT_EQ( move.type, ActionType::MOVE_TO );
	ASSERT_NE( move.numbers( "position" ), nullptr );
	EXPECT_EQ( * move.numbers( "position" ), ( std::vector< double >{ 0, 99, -45 } ) );
	EXPECT_DOUBLE_EQ( move.number( "speed", 0 ), 10.0 );
	EXPECT_TRUE( move.is_timed );
	EXPECT_DOUBLE_EQ( move.interval, 1.0 );
	EXPECT_EQ( move.count, 99999 );
}

TEST( TomlStageParserTest, 文法の誤りは場所付きの例外 )
{
	try
	{
		parse( "format = 1\n[stage\n" );
		FAIL() << "例外になりません";
	}
	catch ( const std::exception& e )
	{
		EXPECT_NE( string_t( e.what() ).find( "t.stage.toml:2" ), string_t::npos ) << e.what();
	}
}

TEST( TomlStageParserTest, ファイルが無いと例外 )
{
	EXPECT_THROW( TomlStageParser::parse_file( "media/stage/no-such-stage.stage.toml" ), std::exception );
}

TEST( TomlStageParserTest, 読めない版は例外_版が無ければ警告 )
{
	EXPECT_THROW( parse( "format = 2\n" ), std::exception );
	EXPECT_THROW( parse( "format = \"1\"\n" ), std::exception );

	const StageData data = parse( "[goal]\nposition = [1, 2, 3]\n" );
	EXPECT_TRUE( has_warning( data, { "format" } ) ) << all_warnings( data );
	EXPECT_TRUE( data.goal.has_value() );
}

TEST( TomlStageParserTest, 知らない種類の物体は警告に残して飛ばす )
{
	const StageData data = parse( R"(
format = 1
[[object]]
type = "dragon"
position = [0, 0, 0]
[[object]]
type = "balloon"
position = [1, 2, 3]
)" );

	ASSERT_EQ( data.objects.size(), 1u );
	EXPECT_EQ( data.objects[ 0 ].type, "balloon" );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:4:", "dragon" } ) ) << all_warnings( data );
}

TEST( TomlStageParserTest, 知らない動きは警告に残して飛ばす )
{
	const StageData data = parse( R"(
format = 1
[[event]]
when = "start"
then = [
  { do = "explode" },
  { do = "stop_all_sounds" },
]
)" );

	ASSERT_EQ( data.start_actions.size(), 1u );
	EXPECT_EQ( data.start_actions[ 0 ].type, ActionType::STOP_ALL_SOUNDS );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:6:", "explode" } ) ) << all_warnings( data );
}

TEST( TomlStageParserTest, 知らないキーはそのキーだけ飛ばす )
{
	const StageData data = parse( R"(
format = 1
sky = "blue"
[stage]
collision = "a.obj"
ground_type = 1
[[object]]
type = "medal"
position = [0, 0, 0]
color = "gold"
[[event]]
when = "start"
then = [ { do = "play_sound", sound = "a", volume = 1 } ]
)" );

	EXPECT_EQ( data.collision, "a.obj" );
	ASSERT_EQ( data.objects.size(), 1u );
	ASSERT_EQ( data.start_actions.size(), 1u );
	EXPECT_FALSE( data.start_actions[ 0 ].has( "volume" ) );

	EXPECT_TRUE( has_warning( data, { "t.stage.toml:3:", "sky" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "ground_type" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:10:", "color" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "volume" } ) ) << all_warnings( data );
	EXPECT_EQ( data.warnings.size(), 4u ) << all_warnings( data );
}

TEST( TomlStageParserTest, 型の違いはその物体と動きを飛ばす )
{
	const StageData data = parse( R"(
format = 1
[stage]
ground = 3
[[object]]
type = "balloon"
position = "here"
[[object]]
type = "balloon"
position = [1, 2]
[[object]]
type = "balloon"
position = [1, 2, 3]
[[event]]
when = "start"
then = [
  { do = "set", line_type = 1.5 },
  { do = "move_to", target = "player", position = [1, 2, 3], speed = "fast" },
  { do = "set", shading_enabled = 1 },
  { do = "set", ambient_color = [1, 1, 1, 1, 1] },
  { do = "play_sound", sound = "a", delay = "soon" },
  { do = "set", line_type = 2 },
]
)" );

	EXPECT_TRUE( data.ground.empty() );
	ASSERT_EQ( data.objects.size(), 1u );
	EXPECT_EQ( data.objects[ 0 ].position, ( Vector3d{ 1, 2, 3 } ) );

	ASSERT_EQ( data.start_actions.size(), 1u );
	EXPECT_DOUBLE_EQ( data.start_actions[ 0 ].number( "line_type", 0 ), 2.0 );

	EXPECT_TRUE( has_warning( data, { "ground", "文字列" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:7:", "position" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:10:", "position" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "line_type", "整数" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "speed", "数" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "shading_enabled", "真偽" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "ambient_color", "1〜4 個" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "delay" } ) ) << all_warnings( data );
}

TEST( TomlStageParserTest, 必須の欠けと組み合わせの誤りは飛ばす )
{
	const StageData data = parse( R"(
format = 1
[[object]]
type = "static"
position = [0, 0, 0]
[[object]]
type = "area-switch"
position = [0, 0, 0]
[[object]]
type = "balloon"
[[event]]
when = "start"
then = [
  { do = "play_sound" },
  { do = "turn_to", target = "player", direction = 90, face = "goal" },
  { do = "switch", target = "player", state = "half" },
  { do = "set_mode", target = "player", mode = "dance" },
  { do = "set" },
  { do = "set_body", target = "player" },
  { do = "play_sound", sound = "a", delay = 1, every = 2 },
  { do = "play_sound", sound = "a", count = 2 },
  { do = "play_sound", sound = "a", every = 2, count = 0 },
  { sound = "a" },
  "play_sound",
]
[[event]]
when = "goal"
then = [ { do = "stop_all_sounds" } ]
)" );

	EXPECT_TRUE( data.objects.empty() );
	EXPECT_TRUE( data.start_actions.empty() );

	EXPECT_TRUE( has_warning( data, { "static", "model" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "area-switch", "size" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "balloon", "position" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "sound がありません" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "direction と face" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "half" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "dance" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "delay と every" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "count は every" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "count は 1 以上" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "do" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "goal", "start" } ) ) << all_warnings( data );
}

TEST( TomlStageParserTest, 見つからない名前の参照は読み終わったときに警告し_動きは残す )
{
	const StageData data = parse( R"(
format = 1
[[object]]
type = "switch"
name = "switch-a"
position = [0, 0, 0]
on_turn_on = [
  { do = "switch", target = "switch-a", state = "broken" },
  { do = "switch", target = "switch-j", state = "broken" },
]
[[object]]
type = "girl"
position = [0, 0, 0]
[[event]]
when = "start"
then = [
  { do = "turn_to", target = "girl", face = "nobody" },
  { do = "move_to", target = "goal", position = [0, 0, 0] },
  { do = "move_to", target = "player", position = [0, 0, 0] },
]
)" );

	ASSERT_EQ( data.objects.size(), 2u );
	EXPECT_EQ( data.objects[ 0 ].events.size(), 2u );
	EXPECT_EQ( data.start_actions.size(), 3u );

	EXPECT_TRUE( has_warning( data, { "t.stage.toml:9:", "switch-j" } ) ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "face", "nobody" } ) ) << all_warnings( data );
	EXPECT_EQ( data.warnings.size(), 2u ) << all_warnings( data );	// girl・goal・player は見つかる
}

TEST( TomlStageParserTest, 同じ名前が2つなら警告 )
{
	const StageData data = parse( R"(
format = 1
[[object]]
type = "balloon"
name = "a"
position = [0, 0, 0]
[[object]]
type = "balloon"
name = ["a", "b"]
position = [1, 1, 1]
)" );

	ASSERT_EQ( data.objects.size(), 2u );
	EXPECT_EQ( data.objects[ 1 ].names, ( std::vector< string_t >{ "a", "b" } ) );
	ASSERT_EQ( data.warnings.size(), 1u ) << all_warnings( data );
	EXPECT_TRUE( has_warning( data, { "t.stage.toml:7:", "名前 a", "t.stage.toml:3:" } ) ) << all_warnings( data );
}

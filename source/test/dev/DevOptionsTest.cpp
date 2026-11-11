#include "pch.h"

#include <blue_sky/dev/DevOptions.h>

#include <map>

using blue_sky::dev::DevOptions;
using blue_sky::SceneRequest;

namespace
{

/// 偽の環境変数（名前 → 値）から読む
DevOptions read( const std::map< std::string, std::string >& env )
{
	return DevOptions::read( [ & env ] ( const char* name ) -> std::optional< std::string >
	{
		const auto i = env.find( name );
		return i != env.end() ? std::optional< std::string >( i->second ) : std::nullopt;
	} );
}

} // namespace

/// 何も設定しなければ、どの開発用の機能も動かない
TEST( DevOptionsTest, NothingSet )
{
	const DevOptions o = read( { } );

	EXPECT_FALSE( o.start_scene );
	EXPECT_FALSE( o.start_stage );
	EXPECT_FALSE( o.start_overlay );
	EXPECT_TRUE( o.capture_frames.empty() );
	EXPECT_EQ( o.capture_directory, "dump" );
	EXPECT_FALSE( o.capture_glyph_atlas );
	EXPECT_EQ( o.display_change_width, 0 );
	EXPECT_EQ( o.display_change_height, 0 );
	EXPECT_EQ( o.fixed_step_msec, 0u );
	EXPECT_EQ( o.frame_time_first, 0 );
	EXPECT_EQ( o.frame_time_last, 0 );
	EXPECT_TRUE( o.scene_sequence.empty() );
}

/// 起動のシーン・ステージ・オーバーレイ（空の値は設定しなかったのと同じ。以前の GetEnvironmentVariableA と同じ）
TEST( DevOptionsTest, StartSceneStageOverlay )
{
	const DevOptions o = read( { { "BLUE_SKY_START_SCENE", "game_play" }, { "BLUE_SKY_START_STAGE", "1-1" }, { "BLUE_SKY_START_OVERLAY", "options?tab=controls" } } );

	EXPECT_EQ( o.start_scene.value_or( "" ), "game_play" );
	EXPECT_EQ( o.start_stage.value_or( "" ), "1-1" );
	EXPECT_EQ( o.start_overlay.value_or( "" ), "options?tab=controls" );

	const DevOptions e = read( { { "BLUE_SKY_START_SCENE", "" }, { "BLUE_SKY_START_OVERLAY", "" } } );

	EXPECT_FALSE( e.start_scene );
	EXPECT_FALSE( e.start_overlay );
}

/// フレームの保存 : 番号の並び・保存先・文字のアトラス（アトラスは値が空でも設定されていれば保存する）
TEST( DevOptionsTest, CaptureFrames )
{
	const DevOptions o = read( { { "BLUE_SKY_CAPTURE_FRAMES", "60,,120" }, { "BLUE_SKY_CAPTURE_DIR", "C:/tmp/cap" }, { "BLUE_SKY_CAPTURE_GLYPH_ATLAS", "" } } );

	EXPECT_EQ( o.capture_frames, ( std::vector< int > { 60, 120 } ) );
	EXPECT_EQ( o.capture_directory, "C:/tmp/cap" );
	EXPECT_TRUE( o.capture_glyph_atlas );
}

/// 解像度の変更の試し・経過時間の固定・フレームの時間（形の違う値は設定しなかったのと同じ）
TEST( DevOptionsTest, NumbersAndBadValues )
{
	const DevOptions o = read( { { "BLUE_SKY_DEBUG_DISPLAY_CHANGE", "1280x720" }, { "BLUE_SKY_DEBUG_FIXED_STEP_MSEC", "16" }, { "BLUE_SKY_DEBUG_FRAME_TIME", "300,1500" } } );

	EXPECT_EQ( o.display_change_width, 1280 );
	EXPECT_EQ( o.display_change_height, 720 );
	EXPECT_EQ( o.fixed_step_msec, 16u );
	EXPECT_EQ( o.frame_time_first, 300 );
	EXPECT_EQ( o.frame_time_last, 1500 );

	const DevOptions bad = read( { { "BLUE_SKY_DEBUG_DISPLAY_CHANGE", "1280" }, { "BLUE_SKY_DEBUG_FIXED_STEP_MSEC", "0" }, { "BLUE_SKY_DEBUG_FRAME_TIME", "1500,300" } } );

	EXPECT_EQ( bad.display_change_width, 0 );
	EXPECT_EQ( bad.fixed_step_msec, 0u );
	EXPECT_EQ( bad.frame_time_first, 0 );
	EXPECT_EQ( bad.frame_time_last, 0 );
}

/// シーンを順に替える : "シーン[:ステージ]" を , で並べる（空の項目は飛ばす）
TEST( DevOptionsTest, SceneSequence )
{
	const DevOptions o = read( { { "BLUE_SKY_DEBUG_SCENE_SEQUENCE", "stage_select,game_play:1-1,,title" } } );

	ASSERT_EQ( o.scene_sequence.size(), 3u );
	EXPECT_EQ( o.scene_sequence[ 0 ], ( SceneRequest { SceneRequest::Type::REPLACE_BASE, "stage_select" } ) );
	EXPECT_EQ( o.scene_sequence[ 1 ], ( SceneRequest { SceneRequest::Type::REPLACE_BASE, "game_play", "1-1" } ) );
	EXPECT_EQ( o.scene_sequence[ 2 ], ( SceneRequest { SceneRequest::Type::REPLACE_BASE, "title" } ) );
}

/// 本当の環境変数から読む（設定していない・空・値あり）
TEST( DevOptionsTest, ReadsProcessEnvironment )
{
	SetEnvironmentVariableA( "BLUE_SKY_DEV_OPTIONS_TEST", "abc" );
	EXPECT_EQ( blue_sky::dev::get_environment_variable( "BLUE_SKY_DEV_OPTIONS_TEST" ).value_or( "" ), "abc" );

	SetEnvironmentVariableA( "BLUE_SKY_DEV_OPTIONS_TEST", "" );
	const auto empty = blue_sky::dev::get_environment_variable( "BLUE_SKY_DEV_OPTIONS_TEST" );
	EXPECT_TRUE( empty.has_value() );
	EXPECT_EQ( empty.value_or( "x" ), "" );

	SetEnvironmentVariableA( "BLUE_SKY_DEV_OPTIONS_TEST", nullptr );
	EXPECT_FALSE( blue_sky::dev::get_environment_variable( "BLUE_SKY_DEV_OPTIONS_TEST" ) );
}

#include "pch.h"

#include <blue_sky/dev/DevTools.h>

#include <filesystem>
#include <string>
#include <vector>

using blue_sky::dev::DevOptions;
using blue_sky::dev::DevTools;
using blue_sky::SceneRequest;

namespace
{

/// DevTools を偽の仕事で動かす土台
class DevToolsTest : public ::testing::Test
{
protected:
	DevOptions options;

	std::vector< std::wstring > saved;			///< 保存した描画結果のパス
	std::vector< std::wstring > atlases;		///< 保存した文字のアトラスのパス
	std::vector< std::pair< int, int > > display_changes;
	std::vector< SceneRequest > requests;

	std::unique_ptr< DevTools > make()
	{
		DevTools::Hooks h;
		h.save_back_buffer = [this] ( const std::wstring& path ) { saved.push_back( path ); return true; };
		h.save_glyph_atlas = [this] ( const std::wstring& path ) { atlases.push_back( path ); };
		h.change_display = [this] ( int w, int h ) { display_changes.emplace_back( w, h ); };
		h.request_scene = [this] ( const SceneRequest& r ) { requests.push_back( r ); };
		h.current_scene_name = [] { return std::string( "title" ); };

		auto tools = std::make_unique< DevTools >( options, std::move( h ) );
		tools->setup();
		return tools;
	}

	/// 1 フレーム分の流れ（update の始まり → シーンを順に替える → 描き終えて present）
	static void frame( DevTools& tools, bool is_loading_frame = false )
	{
		tools.begin_frame();
		tools.update_scene_sequence( is_loading_frame );
		tools.after_render();
		tools.present( [] { } );
	}
};

} // namespace

/// 何も設定しなければ、保存も切り替えもしない
TEST_F( DevToolsTest, NothingHappensWithoutOptions )
{
	auto tools = make();

	for ( int i = 0; i < 300; i++ )
	{
		frame( * tools );
	}

	EXPECT_EQ( tools->get_frame_count(), 300 );
	EXPECT_TRUE( saved.empty() );
	EXPECT_TRUE( display_changes.empty() );
	EXPECT_TRUE( requests.empty() );
	EXPECT_FALSE( tools->is_measuring_frame_time() );
}

/// 決まったフレームの描画結果を保存先に保存する（文字のアトラスも）
TEST_F( DevToolsTest, CapturesRequestedFrames )
{
	const auto directory = std::filesystem::temp_directory_path() / "blue-sky-dev-tools-test";
	options.capture_frames = { 2, 4 };
	options.capture_directory = directory.string();
	options.capture_glyph_atlas = true;

	auto tools = make();

	EXPECT_TRUE( std::filesystem::is_directory( directory ) );		// 保存先を作る

	for ( int i = 0; i < 5; i++ )
	{
		frame( * tools );
	}

	ASSERT_EQ( saved.size(), 2u );
	EXPECT_EQ( saved[ 0 ], directory.wstring() + L"/capture_0002.png" );
	EXPECT_EQ( saved[ 1 ], directory.wstring() + L"/capture_0004.png" );
	ASSERT_EQ( atlases.size(), 2u );
	EXPECT_EQ( atlases[ 1 ], directory.wstring() + L"/atlas_0004.png" );

	std::error_code ec;
	std::filesystem::remove( directory, ec );
}

/// 解像度の変更の試しは、決まったフレームで 1 回だけ
TEST_F( DevToolsTest, DisplayChangeAtFixedFrame )
{
	options.display_change_width = 1024;
	options.display_change_height = 576;

	auto tools = make();

	for ( int i = 1; i < DevTools::DISPLAY_CHANGE_FRAME; i++ )
	{
		frame( * tools );
	}

	EXPECT_TRUE( display_changes.empty() );

	for ( int i = 0; i < 200; i++ )
	{
		frame( * tools );
	}

	ASSERT_EQ( display_changes.size(), 1u );
	EXPECT_EQ( display_changes[ 0 ], std::make_pair( 1024, 576 ) );
}

/// シーンを順に替える : 決まった間隔で次のシーンを要求する。読み込み中のフレームは数えない
TEST_F( DevToolsTest, SceneSequence )
{
	options.scene_sequence = { { SceneRequest::Type::REPLACE_BASE, "stage_select" }, { SceneRequest::Type::REPLACE_BASE, "game_play", "1-1" } };

	auto tools = make();

	for ( int i = 1; i < DevTools::SCENE_SEQUENCE_INTERVAL; i++ )
	{
		frame( * tools );
	}

	EXPECT_TRUE( requests.empty() );

	frame( * tools, true );		// 読み込み中のフレームは数えない
	EXPECT_TRUE( requests.empty() );

	frame( * tools );
	ASSERT_EQ( requests.size(), 1u );
	EXPECT_EQ( requests[ 0 ].name, "stage_select" );

	for ( int i = 0; i < DevTools::SCENE_SEQUENCE_INTERVAL * 3; i++ )
	{
		frame( * tools );
	}

	ASSERT_EQ( requests.size(), 2u );		// 並びを使い切ったら止まる
	EXPECT_EQ( requests[ 1 ], options.scene_sequence[ 1 ] );
}

/// フレームの時間を測る範囲が設定されていれば、測る（ログの中身は時間なので確かめない）
TEST_F( DevToolsTest, FrameTimeMeasurementIsEnabledByOptions )
{
	options.frame_time_first = 2;
	options.frame_time_last = 4;

	auto tools = make();
	EXPECT_TRUE( tools->is_measuring_frame_time() );

	for ( int i = 0; i < 6; i++ )
	{
		frame( * tools );
	}

	EXPECT_EQ( tools->get_frame_count(), 6 );
}

/// ゲーム中のスクショ（F11）は、頼んだときだけ 1 枚保存し、あるファイルは上書きせず次の番号にする
TEST_F( DevToolsTest, ScreenshotIsSavedOnlyWhenRequested )
{
	const auto directory = std::filesystem::temp_directory_path() / "blue-sky-dev-tools-screenshot-test";
	std::filesystem::remove_all( directory );

	auto tools = make();
	tools->set_screenshot_directory( directory.string() );

	tools->capture_screenshot_if_requested();
	EXPECT_TRUE( saved.empty() );								// 頼まなければ撮らない

	// shot_0000.png があるときは飛ばす
	std::filesystem::create_directories( directory );
	{
		std::FILE* f = nullptr;
		_wfopen_s( & f, ( directory / L"shot_0000.png" ).c_str(), L"wb" );
		ASSERT_NE( f, nullptr );
		std::fclose( f );
	}

	tools->request_screenshot();
	tools->capture_screenshot_if_requested();
	ASSERT_EQ( saved.size(), 1u );
	EXPECT_EQ( std::filesystem::path( saved[ 0 ] ).filename(), L"shot_0001.png" );

	tools->capture_screenshot_if_requested();
	EXPECT_EQ( saved.size(), 1u );								// 1 回頼めば 1 枚だけ

	tools->request_screenshot();
	tools->capture_screenshot_if_requested();
	ASSERT_EQ( saved.size(), 2u );
	EXPECT_EQ( std::filesystem::path( saved[ 1 ] ).filename(), L"shot_0002.png" );

	std::filesystem::remove_all( directory );
}

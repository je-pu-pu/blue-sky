#include "pch.h"

#include <blue_sky/SceneStack.h>
#include <Scene/Scene.h>

#include <common/exception.h>

#include <map>
#include <set>
#include <string>
#include <vector>

using blue_sky::Scene;
using blue_sky::SceneArguments;
using blue_sky::SceneRequest;
using blue_sky::SceneStack;

namespace
{

/// 起きたことの記録（"作る:title" "壊す:title" "片付け" など）
using Events = std::vector< std::string >;

/// 偽のシーン（GameMain なしで作る。近道は使わない）
class FakeScene : public Scene
{
	Events&		events_;
	std::string	label_;
	bool		is_overlay_;
	EscapeBehavior escape_;

public:
	int resume_count = 0;

	FakeScene( const SceneArguments& args, Events& events, const std::string& label, bool is_overlay, EscapeBehavior escape )
		: Scene( args )
		, events_( events )
		, label_( label )
		, is_overlay_( is_overlay )
		, escape_( escape )
	{
		events_.push_back( "作る:" + label_ );
	}

	~FakeScene() override
	{
		events_.push_back( "壊す:" + label_ );
	}

	void update() override { }
	void render() override { }

	bool is_overlay() const override { return is_overlay_; }
	EscapeBehavior get_escape_behavior() const override { return escape_; }
	void on_resume() override { resume_count++; }
};

/// SceneStack を偽の仕事で動かす土台
class SceneStackTest : public ::testing::Test
{
protected:
	Events events;

	std::set< std::string > registered { "title", "game_play", "stage_select", "pause_menu", "options", "dialog", "broken", "throws" };
	std::set< std::string > overlays { "pause_menu", "options", "dialog" };
	std::map< std::string, Scene::EscapeBehavior > escapes { { "title", Scene::EscapeBehavior::QUIT }, { "game_play", Scene::EscapeBehavior::PAUSE_MENU } };

	bool title_fails = false;			///< title も作れない（例外になる）
	bool can_return_to_title = false;
	int quit_count = 0;
	int cursor_count = 0;
	int all_closed_count = 0;
	std::vector< std::string > sounds;
	SceneArguments arguments_seen_while_creating;	///< 作っている間に get_next_scene_arguments で見えた引数

	std::unique_ptr< SceneStack > stack;

	void SetUp() override
	{
		SceneStack::Hooks h;

		h.is_registered = [this] ( const std::string& name ) { return registered.count( name ) > 0; };
		h.create_scene = [this] ( const std::string& name, const SceneArguments& args ) -> std::unique_ptr< Scene >
		{
			arguments_seen_while_creating = stack->get_next_scene_arguments();

			if ( name == "broken" || ( name == "title" && title_fails ) )
			{
				return nullptr;
			}

			if ( name == "throws" )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( "モデルが無い" );
			}

			const auto e = escapes.find( name );
			return std::make_unique< FakeScene >( args, events, name, overlays.count( name ) > 0, e != escapes.end() ? e->second : Scene::EscapeBehavior::DEFAULT );
		};
		h.before_base_destroyed = [this] ( const std::string& name ) { events.push_back( "音:" + name ); };
		h.after_base_destroyed = [this] { events.push_back( "片付け" ); };
		h.show_loading_screen = [this] { events.push_back( "読み込み中" ); };
		h.after_base_replaced = [this] { events.push_back( "保存" ); };
		h.update_cursor = [this] { cursor_count++; };
		h.on_all_overlays_closed = [this] { all_closed_count++; };
		h.quit = [this] { quit_count++; };
		h.play_sound = [this] ( const char* name ) { sounds.push_back( name ); };
		h.can_return_to_title = [this] { return can_return_to_title; };

		stack = std::make_unique< SceneStack >( std::move( h ) );
	}

	/// フレームの区切りの流れ（要求を集めて実行し、ベースの置き換えがあれば次のフレームで行う）
	void end_frame()
	{
		stack->collect_requests();
		stack->process_requests();

		if ( stack->is_loading_frame() )
		{
			stack->replace_base_with_loading_screen();
		}
	}

	std::string base_name() const { return stack->get_base_scene() ? stack->get_base_scene()->get_name() : std::string( "なし" ); }
	std::string top_name() const { return stack->get_top_overlay() ? stack->get_top_overlay()->get_name() : std::string( "なし" ); }
};

} // namespace

/// 起動 : ベースを作り、登録した名前とステージ名を覚えさせる。前のシーンを壊す前に音、壊した後に片付け
TEST_F( SceneStackTest, StartCreatesBaseScene )
{
	stack->set_stage_name( "1-1" );
	stack->start( "game_play" );

	EXPECT_EQ( base_name(), "game_play" );
	EXPECT_EQ( stack->get_base_scene()->get_next_stage_name(), "1-1" );
	EXPECT_EQ( events, ( Events { "音:game_play", "片付け", "作る:game_play" } ) );
	EXPECT_EQ( cursor_count, 1 );
}

/// 起動のシーンが未登録なら title にする
TEST_F( SceneStackTest, StartWithUnregisteredSceneFallsBackToTitle )
{
	stack->start( "no_such_scene" );

	EXPECT_EQ( base_name(), "title" );
}

/// 積む : ベースはポーズし、外すとポーズが解ける。すべて閉じたら知らせる
TEST_F( SceneStackTest, PushAndPopOverlay )
{
	stack->start( "game_play" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
	end_frame();

	EXPECT_EQ( top_name(), "pause_menu" );
	EXPECT_TRUE( stack->get_base_scene()->is_paused() );
	EXPECT_TRUE( stack->has_overlay() );

	stack->request( { SceneRequest::Type::POP_OVERLAY } );
	end_frame();

	EXPECT_FALSE( stack->has_overlay() );
	EXPECT_FALSE( stack->get_base_scene()->is_paused() );
	EXPECT_EQ( all_closed_count, 1 );
}

/// 上のオーバーレイを外すと、下のオーバーレイに on_resume が届く。下が残っている間は「すべて閉じた」にならない
TEST_F( SceneStackTest, PopResumesOverlayBelow )
{
	stack->start( "game_play" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "options" } );
	end_frame();

	ASSERT_EQ( stack->get_overlays().size(), 2u );
	auto* pause_menu = static_cast< FakeScene* >( stack->get_overlays()[ 0 ].get() );

	stack->request( { SceneRequest::Type::POP_OVERLAY } );
	end_frame();

	EXPECT_EQ( top_name(), "pause_menu" );
	EXPECT_EQ( pause_menu->resume_count, 1 );
	EXPECT_EQ( all_closed_count, 0 );
	EXPECT_TRUE( stack->get_base_scene()->is_paused() );
}

/// 最前面の置き換え・オーバーレイ用でないシーンは積まない・未登録は無視する
TEST_F( SceneStackTest, ReplaceOverlayAndRejectedOverlays )
{
	stack->start( "game_play" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
	stack->request( { SceneRequest::Type::REPLACE_OVERLAY, "options" } );
	end_frame();

	ASSERT_EQ( stack->get_overlays().size(), 1u );
	EXPECT_EQ( top_name(), "options" );

	stack->request( { SceneRequest::Type::POP_OVERLAY } );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "stage_select" } );		// オーバーレイ用でない
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "no_such_scene" } );	// 未登録
	end_frame();

	EXPECT_FALSE( stack->has_overlay() );
	EXPECT_FALSE( stack->get_base_scene()->is_paused() );
	EXPECT_EQ( base_name(), "game_play" );
}

/// ベースの置き換え : オーバーレイをすべて閉じ、次のフレームで「読み込み中」を出してから置き換える。
/// 同じフレームに積まれた残りの要求は、置き換えの後で処理する
TEST_F( SceneStackTest, ReplaceBaseShowsLoadingScreenNextFrame )
{
	stack->start( "game_play" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
	end_frame();
	events.clear();

	stack->request( { SceneRequest::Type::REPLACE_BASE, "stage_select", "2-0" } );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "options" } );
	stack->process_requests();

	// このフレームでは、オーバーレイを閉じただけでベースはまだ古い
	EXPECT_TRUE( stack->is_loading_frame() );
	EXPECT_FALSE( stack->has_overlay() );
	EXPECT_EQ( base_name(), "game_play" );
	EXPECT_EQ( stack->get_stage_name(), "" );

	stack->replace_base_with_loading_screen();

	EXPECT_FALSE( stack->is_loading_frame() );
	EXPECT_EQ( base_name(), "stage_select" );
	EXPECT_EQ( stack->get_stage_name(), "2-0" );
	EXPECT_EQ( stack->get_base_scene()->get_next_stage_name(), "2-0" );
	EXPECT_EQ( top_name(), "options" );
	EXPECT_EQ( events, ( Events { "壊す:pause_menu", "読み込み中", "音:stage_select", "壊す:game_play", "片付け", "作る:stage_select", "保存", "作る:options" } ) );
}

/// 未登録のシーンへの置き換えは無視する（ベースはそのまま）
TEST_F( SceneStackTest, UnregisteredRequestIsIgnored )
{
	stack->start( "title" );
	stack->request( { SceneRequest::Type::REPLACE_BASE, "no_such_scene" } );
	end_frame();

	EXPECT_FALSE( stack->is_loading_frame() );
	EXPECT_EQ( base_name(), "title" );
}

/// 作れなかったシーン（nullptr・例外）は title にする。title も作れなければ例外
TEST_F( SceneStackTest, FailedSceneFallsBackToTitle )
{
	stack->start( "game_play" );
	stack->setup_base_scene( "broken" );
	EXPECT_EQ( base_name(), "title" );

	stack->setup_base_scene( "throws" );
	EXPECT_EQ( base_name(), "title" );

	title_fails = true;
	EXPECT_THROW( stack->setup_base_scene( "broken" ), common::exception< std::string > );
}

/// シーンの引数 : 作っている間だけ get_next_scene_arguments で見え、シーンは get_argument で読める
TEST_F( SceneStackTest, SceneArgumentsArePassedWhileCreating )
{
	stack->start( "title" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "options", "", { { "tab", "controls" } } } );
	end_frame();

	EXPECT_EQ( arguments_seen_while_creating, ( SceneArguments { { "tab", "controls" } } ) );
	EXPECT_EQ( stack->get_top_overlay()->get_argument( "tab" ), "controls" );
	EXPECT_EQ( stack->get_top_overlay()->get_argument( "missing", "x" ), "x" );
	EXPECT_TRUE( stack->get_next_scene_arguments().empty() );
}

/// シーンが出した切り替えの文字列を要求にする（ベースはステージ名も付ける。オーバーレイの "pop" は閉じる）
TEST_F( SceneStackTest, CollectsRequestsFromScenes )
{
	stack->start( "title" );
	stack->get_base_scene()->set_next_scene( "push:options?tab=controls" );
	end_frame();

	EXPECT_EQ( top_name(), "options" );
	EXPECT_EQ( stack->get_top_overlay()->get_argument( "tab" ), "controls" );
	EXPECT_EQ( stack->get_base_scene()->get_next_scene(), "" );		// 要求は消費する

	stack->get_top_overlay()->set_next_scene( "pop" );
	end_frame();
	EXPECT_FALSE( stack->has_overlay() );

	stack->get_base_scene()->set_next_stage_name( "1-2" );
	stack->get_base_scene()->set_next_scene( "game_play" );
	end_frame();

	EXPECT_EQ( base_name(), "game_play" );
	EXPECT_EQ( stack->get_stage_name(), "1-2" );
}

/// ESC : オーバーレイが開いていれば何もしない。開いていなければベースが申告したことをする
TEST_F( SceneStackTest, Escape )
{
	stack->start( "game_play" );

	stack->handle_escape( false );
	EXPECT_TRUE( sounds.empty() );

	stack->handle_escape( true );		// ゲームプレイ → ポーズメニュー
	end_frame();
	EXPECT_EQ( top_name(), "pause_menu" );
	EXPECT_EQ( sounds, ( std::vector< std::string > { "ok" } ) );

	stack->handle_escape( true );		// オーバーレイが開いているので UI に任せる
	end_frame();
	EXPECT_EQ( stack->get_overlays().size(), 1u );
	EXPECT_EQ( sounds.size(), 1u );

	stack->setup_base_scene( "title" );	// オーバーレイは開いたまま（すぐに作り直すのは開発用の窓だけ）
	stack->pop_overlay();
	stack->handle_escape( true );		// タイトル → 終了
	EXPECT_EQ( quit_count, 1 );

	stack->setup_base_scene( "stage_select" );	// 既定 : 最初のステージをクリアしていなければ終了
	stack->handle_escape( true );
	EXPECT_EQ( quit_count, 2 );
	EXPECT_EQ( sounds.back(), "cancel" );

	can_return_to_title = true;			// 既定 : クリア済みならタイトルへ
	stack->handle_escape( true );
	end_frame();
	EXPECT_EQ( quit_count, 2 );
	EXPECT_EQ( base_name(), "title" );
}

/// 起動時のオーバーレイは、決まったフレーム数の後に積む要求を出す（引数も付けられる）
TEST_F( SceneStackTest, StartOverlayOpensAfterWaitFrames )
{
	stack->start( "title" );
	stack->set_start_overlay( "options?tab=controls" );

	for ( int i = 1; i < SceneStack::START_OVERLAY_WAIT_FRAMES; i++ )
	{
		stack->update_start_overlay();
		end_frame();
		EXPECT_FALSE( stack->has_overlay() ) << i;
	}

	stack->update_start_overlay();
	end_frame();

	EXPECT_EQ( top_name(), "options" );
	EXPECT_EQ( stack->get_top_overlay()->get_argument( "tab" ), "controls" );

	// 一度だけ
	stack->pop_overlay();

	for ( int i = 0; i < SceneStack::START_OVERLAY_WAIT_FRAMES * 2; i++ )
	{
		stack->update_start_overlay();
		end_frame();
	}

	EXPECT_FALSE( stack->has_overlay() );
}

/// 終了の要求
TEST_F( SceneStackTest, QuitRequest )
{
	stack->start( "title" );
	stack->get_base_scene()->set_next_scene( "quit" );
	end_frame();

	EXPECT_EQ( quit_count, 1 );
}

/// 壊すときはオーバーレイ → ベースの順
TEST_F( SceneStackTest, ClearDestroysOverlaysBeforeBase )
{
	stack->start( "game_play" );
	stack->request( { SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
	end_frame();
	events.clear();

	stack.reset();

	EXPECT_EQ( events, ( Events { "壊す:pause_menu", "壊す:game_play" } ) );
}

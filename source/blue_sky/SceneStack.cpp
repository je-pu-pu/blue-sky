#include "SceneStack.h"

#include <Scene/Scene.h>
#include <blue_sky/DebugLog.h>

#include <common/exception.h>

namespace blue_sky
{

namespace
{

template< typename Function, typename ... Args >
void call( const Function& f, Args&& ... args )
{
	if ( f )
	{
		f( std::forward< Args >( args ) ... );
	}
}

} // namespace

SceneStack::SceneStack( Hooks hooks )
	: hooks_( std::move( hooks ) )
{

}

SceneStack::~SceneStack()
{
	clear();
}

void SceneStack::clear()
{
	// オーバーレイはベースの上に積まれているので、ベースより先に壊す
	overlays_.clear();
	base_.reset();
}

/**
 * シーンを作る（作っている間だけ引数を get_next_scene_arguments で見せる）
 */
std::unique_ptr< Scene > SceneStack::generate_scene( const string_t& name, const SceneArguments& args )
{
	next_scene_arguments_ = args;

	try
	{
		std::unique_ptr< Scene > scene = hooks_.create_scene( name, args );
		next_scene_arguments_.clear();
		return scene;
	}
	catch ( ... )
	{
		next_scene_arguments_.clear();
		throw;
	}
}

void SceneStack::start( const string_t& scene_name )
{
	if ( hooks_.is_registered( scene_name ) )
	{
		setup_base_scene( scene_name );
	}
	else
	{
		// 打ち間違いなどで未登録なら、ベースシーンが無いまま進まないようタイトルにする
		debug_log( "[Scene] debug.start_scene が未登録のシーンです。title で起動します: " + scene_name );
		setup_base_scene( "title" );
	}
}

void SceneStack::update_start_overlay()
{
	if ( ! pending_start_overlay_.empty() && ++start_overlay_wait_frames_ >= START_OVERLAY_WAIT_FRAMES )
	{
		// "options?tab=controls" のように引数も付けられる
		SceneRequest r { SceneRequest::Type::PUSH_OVERLAY };
		split_scene_name( pending_start_overlay_, r.name, r.args );
		request( r );
		pending_start_overlay_.clear();
	}
}

void SceneStack::collect_requests()
{
	// 最前面のオーバーレイ
	if ( Scene* overlay = get_top_overlay() )
	{
		const SceneRequest r = parse_scene_request( overlay->get_next_scene(), true );
		overlay->set_next_scene( "" );		// 要求は必ず消費する

		if ( r.type != SceneRequest::Type::NONE )
		{
			request( r );
		}
	}

	// ベースシーン（オーバーレイ中は更新されないので、要求も出ない）
	if ( base_ )
	{
		SceneRequest r = parse_scene_request( base_->get_next_scene(), false );
		base_->set_next_scene( "" );

		if ( r.type != SceneRequest::Type::NONE )
		{
			r.stage_name = base_->get_next_stage_name();
			request( r );
		}
	}
}

void SceneStack::process_requests()
{
	while ( ! requests_.empty() )
	{
		const SceneRequest r = requests_.front();
		requests_.pop_front();

		const bool registered = r.name.empty() || hooks_.is_registered( r.name );

		if ( ! registered )
		{
			debug_log( "[Scene] 未登録のシーンへの遷移を無視しました: " + r.name );
			continue;
		}

		switch ( r.type )
		{
			case SceneRequest::Type::PUSH_OVERLAY:
				push_overlay( r.name, r.args );
				break;

			case SceneRequest::Type::POP_OVERLAY:
				pop_overlay();

				if ( Scene* top = get_top_overlay() )
				{
					top->on_resume();
				}
				break;

			case SceneRequest::Type::REPLACE_OVERLAY:
				pop_overlay();
				push_overlay( r.name, r.args );
				break;

			case SceneRequest::Type::REPLACE_BASE:
				// 上のオーバーレイは古いベースのものなので、すべて閉じる
				while ( has_overlay() )
				{
					pop_overlay();
				}

				// 置き換えは次のフレームで「読み込み中」を表示してから行う。残りの要求は置き換えの後で処理する
				pending_base_request_ = r;
				is_loading_frame_ = true;
				return;

			case SceneRequest::Type::QUIT:
				debug_log( "[App] シーンの要求で終了します" );
				call( hooks_.quit );
				return;

			case SceneRequest::Type::NONE:
				break;
		}
	}
}

void SceneStack::replace_base_with_loading_screen()
{
	is_loading_frame_ = false;

	// 読み込みは同期で行うので、その前に「読み込み中」を 1 回表示しておく（止まって見えないように）
	// シーンの読み込みを別スレッドにしたいときは、テクスチャと同じく AsyncLoader を使う（今は同期で、この 1 回の表示で待たせる）
	call( hooks_.show_loading_screen );

	if ( ! pending_base_request_.stage_name.empty() )
	{
		set_stage_name( pending_base_request_.stage_name );
	}

	setup_base_scene( pending_base_request_.name, pending_base_request_.args );
	call( hooks_.after_base_replaced );

	// 置き換えの後に残った要求（ベースの置き換えと同時に積まれたオーバーレイなど）を処理する
	process_requests();
}

void SceneStack::handle_escape( bool escape_pushed )
{
	// オーバーレイ（UI 画面）が開いている間は、ESC は UI 側が「キャンセル」として処理する
	// （キー割り当て待ちの中止・ダイアログを閉じる・未適用の変更の確認などを UI が判断するため）
	if ( ! escape_pushed || has_overlay() || ! base_ )
	{
		return;
	}

	// 何をするかはシーンが申告する（以前はシーン名 "title" / "game_play" で分岐していた）
	switch ( base_->get_escape_behavior() )
	{
		case Scene::EscapeBehavior::QUIT:
			debug_log( "[App] ESC で終了します（シーン : " + base_->get_name() + "）" );
			call( hooks_.quit );
			break;

		case Scene::EscapeBehavior::PAUSE_MENU:
			request( SceneRequest{ SceneRequest::Type::PUSH_OVERLAY, "pause_menu" } );
			call( hooks_.play_sound, "ok" );
			break;

		case Scene::EscapeBehavior::DEFAULT:
			if ( hooks_.can_return_to_title && hooks_.can_return_to_title() )
			{
				request( SceneRequest{ SceneRequest::Type::REPLACE_BASE, "title" } );
			}
			else
			{
				call( hooks_.quit );
			}

			call( hooks_.play_sound, "cancel" );
			break;

		case Scene::EscapeBehavior::NONE:
			break;
	}
}

void SceneStack::setup_base_scene( const string_t& name, const SceneArguments& args )
{
	if ( ! hooks_.is_registered( name ) )
	{
		// 例外にすると、スクリプトから呼ばれたときに後続のスクリプトが実行されなくなるので、ログに出して無視する
		debug_log( "[Scene] 未登録のシーンを開こうとしました: " + name );
		return;
	}

	call( hooks_.before_base_destroyed, name );

	// 現在のシーンを解放
	base_.reset();

	// シーンによって設定されていたデータをクリア（物体・ECS・物理の世界・解放を頼まれたリソース）
	call( hooks_.after_base_destroyed );

	// 新しいシーンを設定
	// 生成に失敗したら（モデルが無いなど）ログを出してタイトルへ戻す（以前はシーンが null のまま次のフレームで落ちた）
	try
	{
		base_ = generate_scene( name, args );
	}
	catch ( const common::exception< std::string >& e )
	{
		debug_log( "[Scene] シーンを作れませんでした: " + name + " : " + e.data() );
	}

	if ( ! base_ )
	{
		if ( name == "title" )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "title シーンを作れませんでした" );
		}

		setup_base_scene( "title" );
		return;
	}

	// 登録した名前（stage_intro など、同じクラスに別の名前を付けたもの）をシーンに覚えさせる
	base_->set_name( name );
	base_->set_next_stage_name( get_stage_name() );

	call( hooks_.update_cursor );
}

/**
 * オーバーレイシーンをスタックにプッシュする
 *
 * ベースシーンを破棄せずに、上にオーバーレイシーン (ポーズメニュー等) を重ねる。
 * ベースシーンの更新は停止し、入力はオーバーレイシーンのみが受け取る。
 */
void SceneStack::push_overlay( const string_t& name, const SceneArguments& args )
{
	if ( ! hooks_.is_registered( name ) )
	{
		debug_log( "[Scene] 未登録のオーバーレイを開こうとしました: " + name );
		return;
	}

	// ベースシーンをポーズ
	if ( base_ )
	{
		base_->set_paused( true );
	}

	// オーバーレイシーンを生成してスタックに追加
	std::unique_ptr< Scene > overlay = generate_scene( name, args );

	// オーバーレイ用でないシーン（ゲーム本体など）は積まない（入力・描画の扱いがオーバーレイを前提にしていないため）
	if ( ! overlay || ! overlay->is_overlay() )
	{
		debug_log( "[Scene] オーバーレイ用でないシーンを重ねようとしました: " + name );

		if ( base_ )
		{
			base_->set_paused( false );
		}
		return;
	}

	overlay->set_name( name );
	overlay->set_next_stage_name( get_stage_name() );

	overlays_.push_back( std::move( overlay ) );

	// カーソルを表示 (メニュー操作用)
	call( hooks_.update_cursor );
}

/**
 * オーバーレイシーンをスタックからポップする
 *
 * スタックが空になった場合、ベースシーンのポーズを解除する。
 */
void SceneStack::pop_overlay()
{
	if ( overlays_.empty() )
	{
		return;
	}

	overlays_.pop_back();

	if ( overlays_.empty() )
	{
		// 全てのオーバーレイが閉じた → ベースシーンのポーズ解除
		if ( base_ )
		{
			base_->set_paused( false );
		}

		call( hooks_.on_all_overlays_closed );
	}
}

} // namespace blue_sky

#include "PauseMenuScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/Input.h>

namespace blue_sky
{

PauseMenuScene::PauseMenuScene()
	: ui_renderer_( get_graphics_manager() )
{
	setup_menu();
}

PauseMenuScene::~PauseMenuScene()
{
}

void PauseMenuScene::setup_menu()
{
	// 再開
	menu_.add_item( "Resume", [this] ()
	{
		set_next_scene( "pop" );
		play_sound( "ok" );
	} );

	// オプション — set_next_scene で安全に遷移 (pop + push を GameMain が処理)
	menu_.add_item( "Options", [this] ()
	{
		set_next_scene( "options" );
		play_sound( "ok" );
	} );

	// ステージ選択に戻る — ベースシーンの遷移先を設定してから pop
	menu_.add_item( "Stage Select", [this] ()
	{
		auto* base_scene = const_cast< Scene* >( GameMain::get_instance()->get_current_scene() );

		if ( base_scene )
		{
			base_scene->set_next_scene( "stage_select" );
		}

		set_next_scene( "pop" );
		play_sound( "cancel" );
	} );

	menu_.set_width( get_content_width() );
	menu_.set_item_height( 60.f );
	menu_.center_on_screen( ui_renderer_ );
}

void PauseMenuScene::update()
{
	Scene::update();

	menu_.update( get_input() );
}

void PauseMenuScene::render()
{
	float_t screen_w = static_cast< float_t >( ui_renderer_.get_screen_width() );
	float_t screen_h = static_cast< float_t >( ui_renderer_.get_screen_height() );

	// 半透明の暗幕（全画面）
	ui_renderer_.draw_rect( 0.f, 0.f, screen_w, screen_h, Color( 0.f, 0.f, 0.f, 0.5f ) );

	// パネル背景
	float_t panel_w = get_panel_width();
	float_t panel_x = ( screen_w - panel_w ) * 0.5f;
	float_t panel_y = screen_h * 0.25f;
	float_t panel_h = screen_h * 0.5f;

	ui_renderer_.draw_rect( panel_x, panel_y, panel_w, panel_h, Color( 0.05f, 0.05f, 0.1f, 0.85f ) );

	// タイトル
	float_t content_w = get_content_width();
	float_t title_x = ( screen_w - content_w ) * 0.5f;
	float_t title_y = screen_h * 0.28f;
	ui_renderer_.draw_text( title_x, title_y, content_w, 70.f, "PAUSE", Color( 1.f, 1.f, 1.f, 1.f ) );

	// メニュー
	menu_.render( ui_renderer_ );

	// 操作ヒント
	float_t hint_y = screen_h * 0.65f;
	float_t hint_x = ( screen_w - content_w ) * 0.5f;
	ui_renderer_.draw_text( hint_x, hint_y, content_w, 50.f, "Enter/Click Select    ESC Back", Color( 0.5f, 0.5f, 0.5f, 1.f ) );
}

} // namespace blue_sky

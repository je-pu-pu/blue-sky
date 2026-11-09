#include "PauseMenuScene.h"

#include <blue_sky/GameMain.h>

#include <core/ui/Button.h>
#include <core/ui/Panel.h>
#include <core/ui/Stack.h>

namespace blue_sky
{

PauseMenuScene::PauseMenuScene()
{
	using namespace core::ui;

	auto panel = std::make_unique< Panel >( "pause.title" );
	panel->set_width( 900.f );
	panel->set_fit_height( true );
	panel->set_show_description( false );

	auto* menu = panel->add< Stack >( Axis::VERTICAL );
	menu->set_wrap( true );

	menu->add< Button >( "pause.resume", [this] { close(); } );
	menu->add< Button >( "pause.options", [this] { set_next_scene( "push:options" ); } );
	menu->add< Button >( "pause.stage_select", [this]
	{
		// ベースシーンの遷移先を設定してから閉じる
		if ( auto* base = const_cast< Scene* >( GameMain::get_instance()->get_current_scene() ) )
		{
			base->set_next_scene( "stage_select" );
		}

		close();
	} );

	// キャンセル（ESC / B）で再開
	screen_.set_root( std::move( panel ), [this] { close(); } );
}

} // namespace blue_sky

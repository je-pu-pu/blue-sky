#include "PauseMenuScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <core/sound/SoundManager.h>

#include <imgui.h>

namespace blue_sky
{

PauseMenuScene::PauseMenuScene()
{
}

PauseMenuScene::~PauseMenuScene()
{
}

void PauseMenuScene::update()
{
	Scene::update();

	const auto& io = ImGui::GetIO();
	const float window_width = 300.f;
	const float window_height = 200.f;

	ImGui::SetNextWindowPos( ImVec2( ( io.DisplaySize.x - window_width ) * 0.5f, ( io.DisplaySize.y - window_height ) * 0.5f ), ImGuiCond_Always );
	ImGui::SetNextWindowSize( ImVec2( window_width, window_height ), ImGuiCond_Always );

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

	if ( ImGui::Begin( "PAUSE", nullptr, flags ) )
	{
		ImGui::Dummy( ImVec2( 0.f, 10.f ) );

		const float button_width = 260.f;
		const float button_height = 40.f;

		// 再開ボタン
		ImGui::SetCursorPosX( ( window_width - button_width ) * 0.5f );
		if ( ImGui::Button( "Resume", ImVec2( button_width, button_height ) ) )
		{
			set_next_scene( "pop" );
			play_sound( "ok" );
		}

		ImGui::Dummy( ImVec2( 0.f, 5.f ) );

		// ステージ選択に戻るボタン
		ImGui::SetCursorPosX( ( window_width - button_width ) * 0.5f );
		if ( ImGui::Button( "Stage Select", ImVec2( button_width, button_height ) ) )
		{
			// オーバーレイをポップしてからベースシーンを遷移させる
			GameMain::get_instance()->pop_overlay_scene();

			auto* base_scene = const_cast< Scene* >( GameMain::get_instance()->get_current_scene() );

			if ( base_scene )
			{
				base_scene->set_next_scene( "stage_select" );
			}

			play_sound( "cancel" );
		}
	}

	ImGui::End();
}

void PauseMenuScene::render()
{
	// 半透明の暗幕を描画
	get_graphics_manager()->set_fade_color( Color( 0.f, 0.f, 0.f, 0.5f ) );
	render_fader();
}

} // namespace blue_sky

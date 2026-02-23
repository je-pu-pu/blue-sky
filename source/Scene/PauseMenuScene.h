#pragma once

#include "Scene.h"

#include <blue_sky/ui/Renderer.h>
#include <core/ui/VerticalMenu.h>

namespace blue_sky
{

/**
 * ポーズメニューシーン
 *
 * オーバーレイシーンとして、ゲームプレイ中に上に重ねて表示される。
 * ベースシーン ( GamePlayScene ) は更新を停止し、入力はこのシーンのみが受け取る。
 * VerticalMenu を使用した ImGui 非依存の UI。
 */
class PauseMenuScene : public Scene
{
public:
	static constexpr auto name = "pause_menu";

private:
	ui::Renderer ui_renderer_;
	core::ui::VerticalMenu menu_;

	// レイアウト (仮想座標 1920x1080 基準)
	float_t get_panel_width() const { return 960.f; }
	float_t get_content_width() const { return 840.f; }

	void setup_menu();

public:
	PauseMenuScene();
	~PauseMenuScene() override;

	void update() override;
	void render() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }

}; // class PauseMenuScene

} // namespace blue_sky

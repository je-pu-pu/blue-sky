#pragma once

#include "Scene.h"

#include <blue_sky/ui/UIRenderer.h>
#include <blue_sky/ui/UIVerticalMenu.h>

namespace blue_sky
{

/**
 * ポーズメニューシーン
 *
 * オーバーレイシーンとして、ゲームプレイ中に上に重ねて表示される。
 * ベースシーン ( GamePlayScene ) は更新を停止し、入力はこのシーンのみが受け取る。
 * UIVerticalMenu を使用した ImGui 非依存の UI。
 */
class PauseMenuScene : public Scene
{
public:
	static constexpr auto name = "pause_menu";

private:
	ui::UIRenderer ui_renderer_;
	ui::UIVerticalMenu menu_;

	// レイアウト
	float_t get_panel_width() const { return 400.f; }
	float_t get_content_width() const { return 350.f; }

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

#pragma once

#include "Scene.h"

#include <blue_sky/ui/UIRenderer.h>
#include <blue_sky/ui/UIVerticalMenu.h>

namespace blue_sky
{

/**
 * オプション画面
 *
 * オーバーレイシーンとして表示される設定画面。
 * UIVerticalMenu を使用した ImGui 非依存の UI。
 */
class OptionsScene : public Scene
{
public:
	static constexpr auto name = "options";

private:
	ui::UIRenderer ui_renderer_;
	ui::UIVerticalMenu menu_;

	int bgm_volume_index_ = -1;
	int se_volume_index_ = -1;

	float_t bgm_volume_ = 1.f;
	float_t se_volume_ = 1.f;

	float_t get_volume_step() const { return 0.1f; }

	void setup_menu();
	void update_volume_text();

	void adjust_bgm_volume( float_t delta );
	void adjust_se_volume( float_t delta );

public:
	OptionsScene();
	~OptionsScene() override;

	void update() override;
	void render() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }

}; // class OptionsScene

} // namespace blue_sky

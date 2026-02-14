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
 * 変更した設定はシーン終了時に Config へ保存する。
 */
class OptionsScene : public Scene
{
public:
	static constexpr auto name = "options";

private:
	ui::UIRenderer ui_renderer_;
	ui::UIVerticalMenu menu_;

	// メニュー項目インデックス
	int volume_index_ = -1;
	int mute_index_ = -1;
	int mouse_sens_index_ = -1;
	int fov_index_ = -1;
	int fullscreen_index_ = -1;
	int back_index_ = -1;

	// 設定値
	float_t volume_ = 1.f;
	bool is_mute_ = false;
	float_t mouse_sensitivity_ = 1.f;
	float_t fov_ = 90.f;
	bool is_fullscreen_ = false;

	// ステップ値
	float_t get_volume_step() const { return 0.1f; }
	float_t get_sensitivity_step() const { return 0.1f; }
	float_t get_fov_step() const { return 5.f; }
	float_t get_min_fov() const { return 50.f; }
	float_t get_max_fov() const { return 120.f; }
	float_t get_min_sensitivity() const { return 0.1f; }
	float_t get_max_sensitivity() const { return 3.f; }

	void setup_menu();
	void update_all_text();
	void save_settings();

	void adjust_value( float_t& value, float_t delta, float_t min_val, float_t max_val );

	static string_t make_bar( float_t value, float_t min_val, float_t max_val, int bar_width = 10 );

public:
	OptionsScene();
	~OptionsScene() override;

	void update() override;
	void render() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }

}; // class OptionsScene

} // namespace blue_sky

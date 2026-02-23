#pragma once

#include "Scene.h"

#include <blue_sky/ui/UIRenderer.h>
#include <blue_sky/ui/UIWidgetContainer.h>
#include <blue_sky/ui/UIButton.h>
#include <blue_sky/ui/UISlider.h>
#include <blue_sky/ui/UISelectBox.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <vector>

namespace blue_sky
{

/**
 * オプション画面
 *
 * オーバーレイシーンとして表示される設定画面。
 * UIWidgetContainer + 個別ウィジェットで構成される ImGui 非依存の UI。
 * 変更した設定はシーン終了時に Config へ保存する。
 */
class OptionsScene : public Scene
{
public:
	static constexpr auto name = "options";

private:
	ui::UIRenderer ui_renderer_;
	ui::UIWidgetContainer container_;

	// ウィジェット（メンバ変数として直接保持）
	ui::UISelectBox resolution_select_;
	ui::UISlider volume_slider_;
	ui::UIButton mute_button_;
	ui::UISlider mouse_sens_slider_;
	ui::UISlider fov_slider_;
	ui::UIButton fullscreen_button_;
	ui::UIButton back_button_;

	// 解像度選択肢 (DXGI から動的に取得)
	using DisplayMode = graphics::GraphicsManager::DisplayMode;
	std::vector< DisplayMode > resolutions_;

	// 設定値
	bool is_mute_ = false;
	bool is_fullscreen_ = false;

	int find_current_resolution() const;
	void apply_resolution( int index );

	// レイアウト (仮想座標 1920x1080 基準)
	float_t get_panel_width() const { return 1680.f; }
	float_t get_content_width() const { return 1440.f; }

	void setup_widgets();
	void save_settings();

public:
	OptionsScene();
	~OptionsScene() override;

	void update() override;
	void render() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }

}; // class OptionsScene

} // namespace blue_sky

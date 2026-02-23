#pragma once

#include "Scene.h"

#include <blue_sky/ui/Renderer.h>
#include <core/ui/WidgetContainer.h>
#include <core/ui/Button.h>
#include <core/ui/Slider.h>
#include <core/ui/SelectBox.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <vector>

namespace blue_sky
{

/**
 * オプション画面
 *
 * オーバーレイシーンとして表示される設定画面。
 * WidgetContainer + 個別ウィジェットで構成される ImGui 非依存の UI。
 * 変更した設定はシーン終了時に Config へ保存する。
 */
class OptionsScene : public Scene
{
public:
	static constexpr auto name = "options";

private:
	ui::Renderer ui_renderer_;
	core::ui::WidgetContainer container_;

	// ウィジェット（メンバ変数として直接保持）
	core::ui::SelectBox resolution_select_;
	core::ui::Slider volume_slider_;
	core::ui::Button mute_button_;
	core::ui::Slider mouse_sens_slider_;
	core::ui::Slider fov_slider_;
	core::ui::Button fullscreen_button_;
	core::ui::Button back_button_;

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

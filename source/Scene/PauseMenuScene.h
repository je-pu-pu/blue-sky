#pragma once

#include "Scene.h"

namespace blue_sky
{

/**
 * ポーズメニューシーン
 *
 * オーバーレイシーンとして、ゲームプレイ中に上に重ねて表示される。
 * ベースシーン ( GamePlayScene ) は更新を停止し、入力はこのシーンのみが受け取る。
 */
class PauseMenuScene : public Scene
{
public:
	static constexpr auto name = "pause_menu";

	PauseMenuScene();
	~PauseMenuScene() override;

	void update() override;
	void render() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }

}; // class PauseMenuScene

} // namespace blue_sky

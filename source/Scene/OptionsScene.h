#pragma once

#include "UIOverlayScene.h"
#include "OptionsController.h"

#include <memory>
#include <vector>

namespace core::ui
{
	class Stack;
}

namespace blue_sky
{

/**
 * オプション画面（グラフィックス／サウンド／操作 のタブ）
 *
 * 値はすべてゲーム設定ストア（GameMain::get_settings()）に結びつく。
 * - 音量・感度・FOV・キー割り当て : 触った瞬間に反映（戻るときに確定・保存）
 * - 解像度・フルスクリーン : 「適用」で反映し、15 秒以内に「維持」しなければ元に戻す
 * - 未適用の変更があるまま戻ろうとしたら確認する
 */
class OptionsScene : public UIOverlayScene
{
public:
	static constexpr auto name = "options";

private:
	struct Resolution
	{
		int width;
		int height;
	};

	std::vector< Resolution > resolutions_;

	void build_graphics_page( core::ui::Stack* page );
	void build_audio_page( core::ui::Stack* page );
	void build_controls_page( core::ui::Stack* page );

	int find_resolution_index( int w, int h ) const;

	/// 適用・確認・ロールバック・保存・戻る・既定値に戻すの流れ
	std::unique_ptr< OptionsController > controller_;

public:
	OptionsScene();

}; // class OptionsScene

} // namespace blue_sky

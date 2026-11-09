#pragma once

#include "UIOverlayScene.h"

namespace blue_sky
{

/**
 * ポーズメニュー
 *
 * ゲームプレイ中に ESC で開くオーバーレイ。ベースシーンは更新を止める。
 * 「オプション」はこの画面の上に積むので、オプションを閉じるとポーズに戻る。
 */
class PauseMenuScene : public UIOverlayScene
{
public:
	static constexpr auto name = "pause_menu";

	PauseMenuScene( GameMain& game_main );

}; // class PauseMenuScene

} // namespace blue_sky

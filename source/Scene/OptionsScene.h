#pragma once

#include "UIOverlayScene.h"

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

	/// 設定ファイルへ保存する
	void save();

	/// 適用（DEFERRED の反映）。表示が変わったら確認ダイアログを出す。close_after なら最後に閉じる
	void apply( bool close_after );

	/// 戻る（未適用の変更があれば確認）
	void back();

	/// IMMEDIATE を確定・保存して閉じる
	void finish_and_close();

	/// 既定値に戻す（確認あり）
	void reset_to_defaults();

public:
	OptionsScene();

}; // class OptionsScene

} // namespace blue_sky

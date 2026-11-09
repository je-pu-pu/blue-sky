#pragma once

#include "Scene.h"

#include <blue_sky/ui/Renderer.h>
#include <blue_sky/ui/UIInputBuilder.h>
#include <core/ui/Screen.h>

namespace core::setting
{
	class SettingStore;
}

namespace blue_sky
{

/**
 * UI 画面（core::ui::Screen）を持つオーバーレイシーンの基底
 *
 * - 毎フレーム Input の生の状態から UIInput を作って Screen に渡す
 * - 開いた直後と、最前面に戻った直後の 1 フレームは入力を捨てる
 *   （ESC でポーズを開いた同じ ESC で閉じない／下の画面に戻ったときに古いマウス位置でホバーしない）
 * - Screen の効果音イベントを play_sound() に変換する
 *
 * 派生クラスはコンストラクタで screen_.set_root() に画面を組み立てるだけでよい。
 */
class UIOverlayScene : public Scene
{
protected:
	ui::Renderer renderer_;
	ui::UIInputBuilder input_builder_;
	core::ui::Screen screen_;

	core::setting::SettingStore& get_settings() const;

	/// この画面を閉じる（下の画面へ戻る）
	void close() { set_next_scene( "pop" ); }

public:
	UIOverlayScene();
	~UIOverlayScene() override;

	void update() override;
	void render() override;
	void on_resume() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }
	bool hides_overlays_below() const override { return true; }

}; // class UIOverlayScene

} // namespace blue_sky

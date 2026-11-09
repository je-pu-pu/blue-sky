#pragma once

#include "Scene.h"

#include <blue_sky/ui/Renderer.h>
#include <blue_sky/ui/UIInputBuilder.h>
#include <core/ui/Screen.h>

#include <functional>

namespace core::setting
{
	class SettingStore;
}

namespace core::ui
{
	struct Theme;
	class StringTable;
}

namespace blue_sky
{

/**
 * UI 画面が使うサービス（テーマ・文字列・設定・設定の保存）
 *
 * UI のシーンは GameMain のシングルトンを直接呼ばず、これを通して使う（設計上の問題 B28）。
 * GameMain から集めるのは from_game_main の 1 か所だけ（GameMain はシーンが作られるときに受け取る）。
 */
struct UIServices
{
	const core::ui::Theme& theme;
	const core::ui::StringTable& strings;
	core::setting::SettingStore& settings;
	std::function< void() > save_settings;

	/// 組み立ての根（GameMain）から集める（シーンが受け取った GameMain を渡す）
	static UIServices from_game_main( GameMain& game_main );
};

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
	UIServices services_;			///< テーマ・文字列・設定（screen_ より先に初期化する）
	ui::Renderer renderer_;
	ui::UIInputBuilder input_builder_;
	core::ui::Screen screen_;
	uint_t theme_revision_ = 0;		///< 最後に配置したときのテーマの版（変わったら配置し直す）

	core::setting::SettingStore& get_settings() const;

	/// この画面を閉じる（下の画面へ戻る）
	void close() { set_next_scene( "pop" ); }

public:
	UIOverlayScene( GameMain& game_main );
	~UIOverlayScene() override;

	void update() override;
	void render() override;
	void on_resume() override;

	bool is_overlay() const override { return true; }
	bool is_transparent() const override { return true; }
	bool hides_overlays_below() const override { return true; }

}; // class UIOverlayScene

} // namespace blue_sky

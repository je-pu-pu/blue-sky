#pragma once

#include <blue_sky/type.h>

#include <functional>
#include <vector>

namespace core::setting
{
	class SettingStore;
}

namespace core::ui
{
	class Screen;
}

namespace blue_sky
{

/**
 * オプション画面の流れ（適用・確認・ロールバック・保存・戻る・既定値に戻す）
 *
 * シーンから切り離し、依存（設定ストア・画面・保存・閉じる・時刻・文言）を外から受け取る。
 * GameMain などのシングルトンに触れないので、テストから動かせる（設計上の問題 B14）。
 *
 * - 表示系（解像度・フルスクリーン）を適用したら、時間内に「維持」しなければ元に戻す。
 *   確認待ちの間は表示系を元の値で保存する（試用中。強制終了しても次回は映っていた設定で起動する）
 * - 未適用の変更があるまま戻ろうとしたら確認する
 */
class OptionsController
{
public:
	/// 表示の変更を確認する時間（秒）。時間切れなら元に戻す
	static constexpr float_t DISPLAY_CONFIRM_TIMEOUT = 15.f;

	struct Dependencies
	{
		core::setting::SettingStore* store = nullptr;
		core::ui::Screen* screen = nullptr;
		std::vector< string_t > display_keys;					///< 表示系の設定キー（確認・ロールバックの対象）
		std::function< void() > save;							///< 設定をファイルへ保存する
		std::function< void() > close;							///< オプション画面を閉じる
		std::function< float_t() > now;							///< 現在の時刻（秒）
		std::function< string_t( int remaining_sec ) > confirm_display_message;	///< 「維持しますか（残り n 秒）」の文言
	};

private:
	Dependencies deps_;

	/// 即時反映の項目を確定・保存して閉じる
	void finish_and_close();

public:
	explicit OptionsController( Dependencies deps ) : deps_( std::move( deps ) ) { }

	/// 適用（「適用」まで反映しない項目を反映する）。表示が変わったら確認ダイアログを出す。close_after なら最後に閉じる
	void apply( bool close_after );

	/// 戻る（未適用の変更があれば確認する）
	void back();

	/// 既定値に戻す（確認あり）
	void reset_to_defaults();

}; // class OptionsController

} // namespace blue_sky

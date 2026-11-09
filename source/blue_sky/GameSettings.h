#pragma once

#include <blue_sky/type.h>
#include <core/input/InputManager.h>

#include <vector>

namespace core::setting
{
	class SettingStore;
}

namespace blue_sky
{
	class GameMain;

/**
 * ゲーム設定の項目登録（既定値の唯一の定義場所）
 *
 * 設定キーは従来の blue-sky.config と同じ名前を使う（既存の設定ファイルをそのまま読める）。
 */
namespace game_settings
{
	// 設定キー
	constexpr auto AUDIO_VOLUME			= "audio.volume";
	constexpr auto AUDIO_MUTE			= "audio.mute";
	constexpr auto MOUSE_X_SENSITIVITY	= "input.mouse.x_sensitivity";
	constexpr auto MOUSE_Y_SENSITIVITY	= "input.mouse.y_sensitivity";
	constexpr auto CAMERA_FOV			= "camera.fov";
	constexpr auto SCREEN_WIDTH			= "graphics.screen_width";
	constexpr auto SCREEN_HEIGHT		= "graphics.screen_height";
	constexpr auto FULL_SCREEN			= "graphics.full_screen";
	constexpr auto DISPLAY_FPS			= "graphics.display_fps";

	/// 表示系（解像度・フルスクリーン）のキー。適用後の確認・ロールバックの対象
	const std::vector< string_t >& get_display_keys();

	/// キー割り当てを変更できるボタンと、その表示用の文字列キー
	struct BindableButton
	{
		core::input::Button button;
		const char_t* label_key;
	};

	const std::vector< BindableButton >& get_bindable_buttons();

	/**
	 * 全項目を登録する
	 *
	 * 反映関数はシーンを捕まえない（シーンは破棄されるため）。
	 * FOV のようにシーンが持つ対象は、シーン側が get_change_count() の変化を見て読み直す。
	 */
	void register_all( core::setting::SettingStore& store, GameMain& game_main );

	/**
	 * キー割り当ての文字列（"W UP" など）の主キー（先頭）を差し替えた文字列を返す
	 *
	 * 2 番目以降のキーは削除する（主キー 1 つだけを編集する方針）。
	 */
	string_t replace_primary_key( const string_t& bindings, const string_t& key_name );

	/// キー割り当ての文字列の主キー名（無ければ空）
	string_t get_primary_key( const string_t& bindings );

} // namespace game_settings

} // namespace blue_sky

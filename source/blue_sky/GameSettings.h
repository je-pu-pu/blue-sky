#pragma once

#include <blue_sky/type.h>
#include <core/input/InputManager.h>
#include <core/setting/SettingKey.h>

#include <vector>

namespace core::setting
{
	class SettingStore;
}

namespace game
{
	class Config;
}

namespace blue_sky
{
	class GameMain;

/**
 * ゲーム設定の項目登録（既定値・範囲・ラベルの唯一の定義場所）
 *
 * 設定キーは従来の blue-sky.config と同じ名前を使う（既存の設定ファイルをそのまま読める）。
 * キーは型付き（SettingKey）なので、型の書き間違いはコンパイル時に分かる。
 */
namespace game_settings
{
	using core::setting::SettingKey;

	// 設定キー
	inline constexpr SettingKey< float_t >	AUDIO_VOLUME		{ "audio.volume" };
	inline constexpr SettingKey< bool >		AUDIO_MUTE			{ "audio.mute" };
	inline constexpr SettingKey< float_t >	MOUSE_X_SENSITIVITY	{ "input.mouse.x_sensitivity" };
	inline constexpr SettingKey< float_t >	MOUSE_Y_SENSITIVITY	{ "input.mouse.y_sensitivity" };
	inline constexpr SettingKey< float_t >	CAMERA_FOV			{ "camera.fov" };
	inline constexpr SettingKey< int >		SCREEN_WIDTH		{ "graphics.screen_width" };
	inline constexpr SettingKey< int >		SCREEN_HEIGHT		{ "graphics.screen_height" };
	inline constexpr SettingKey< bool >		FULL_SCREEN			{ "graphics.full_screen" };
	inline constexpr SettingKey< bool >		DISPLAY_FPS			{ "graphics.display_fps" };

	/// ユーザーの設定ファイル（SettingStore が読み書きする）。開発用・エンジンの設定（blue-sky.config）とは分ける
	inline constexpr const char_t* USER_SETTINGS_PATH = "settings.config";

	/// ユーザーの設定ファイルの形式の版
	inline constexpr int USER_SETTINGS_VERSION = 1;

	/**
	 * ユーザーの設定ファイルを読む
	 *
	 * まだ無ければ、従来どおり開発用の設定（blue-sky.config）に書かれていた値を引き継ぐ（移行）。
	 */
	void load_user_settings_file( game::Config& user_config, const game::Config& legacy_config );

	/**
	 * 起動時の表示設定（ウィンドウを作る前に必要）を、ストアと同じ既定値・範囲で読む
	 *
	 * App はストアより先にウィンドウを作るので、ここで同じ登録を使って読む（既定値・範囲の定義を 1 か所にする）。
	 */
	void read_initial_display( const game::Config& user_config, int& width, int& height, bool& full_screen );

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

	/// 読み込んだ後に、解釈できない値（キー割り当てなど）を既定値に直して確定する
	void fix_invalid_values( core::setting::SettingStore& store );

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

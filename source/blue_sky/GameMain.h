#pragma once


#include "App.h"
#include "TimeManager.h"

#include <game/Game.h>

#include <type/type.h>

#include <unordered_map>
#include <vector>

class DirectWrite;

class DirectInput;

class OculusRift;

namespace game
{
	class Config;
	class MainLoop;
};

namespace core
{
	namespace sound
	{
		class SoundEngine;
		class SoundManager;
	}

	namespace physics
	{
		class PhysicsManager;
	}

	namespace graphics::direct_3d_11
	{
		class Direct3D11;
		class BulletDebugDraw;
	}

	namespace setting
	{
		class SettingStore;
	}

	namespace ui
	{
		class StringTable;
		struct Theme;
	}
}

namespace blue_sky
{
	class Input;

	class ActiveObjectPhysics;

	class ActiveObject;
	class ActiveObjectManager;

	class SceneManager;
	class ScriptManager;

	class Player;
	class Camera;

	class Scene;

	namespace graphics
	{
		class GraphicsManager;
	}

	using graphics::GraphicsManager;
};

namespace blue_sky
{

class GameMain : public game::Game
{
public:
	using Config					= game::Config;
	using MainLoop					= game::MainLoop;

	using SoundEngine				= core::sound::SoundEngine;

	using GraphicsManager			= GraphicsManager;
	using SoundManager				= core::sound::SoundManager;
	using PhysicsManager			= core::physics::PhysicsManager;

	using Direct3D					= core::graphics::direct_3d_11::Direct3D11;
	using BulletDebugDraw			= core::graphics::direct_3d_11::BulletDebugDraw;

	// @todo Input クラスにあるべきでは？
	enum Key
	{
		KEY_LEFT  = VK_LEFT,
		KEY_RIGHT = VK_RIGHT,
		KEY_UP    = VK_UP,
		KEY_DOWN  = VK_DOWN,

		KEY_F1  = VK_F1,
		KEY_F2  = VK_F2,
		KEY_F3  = VK_F3,
		KEY_F4  = VK_F4,
		KEY_F5  = VK_F5,
		KEY_F6  = VK_F6,
		KEY_F7  = VK_F7,
		KEY_F8  = VK_F8,
		KEY_F9  = VK_F9,
		KEY_F10 = VK_F10,
		KEY_F11 = VK_F11,
		KEY_F12 = VK_F12,
	};

private:
	TimeManager								time_manager_;			///< 時間管理

	std::unique_ptr< Direct3D >				direct_3d_;				///< Direct3D

	std::unique_ptr< DirectInput >			direct_input_;			///< DirectInput
	std::unique_ptr< Input >				input_;					///< Game Input

	std::unique_ptr< OculusRift >			oculus_rift_;			///< Oculus Rift

	std::unique_ptr< SoundEngine >			sound_engine_;			///< SoundEngine

	std::unique_ptr< ActiveObjectPhysics >	active_object_physics_;	///< ActiveObjectPhysics (レガシー)
	std::unique_ptr< PhysicsManager >		physics_manager_;		///< PhysicsManager
	std::unique_ptr< GraphicsManager >		graphics_manager_;		///< GraphicsManager
	std::unique_ptr< SoundManager >			sound_manager_;			///< SoundManager
	std::unique_ptr< ScriptManager >		script_manager_;		///< ScriptManager

	std::unique_ptr< Config >				save_data_;				///< Save Data

	std::unique_ptr< MainLoop >				main_loop_;				///< ループ管理

	std::unique_ptr< ActiveObjectManager >	active_object_manager_;	///< ActiveObjectManager

	std::unique_ptr< BulletDebugDraw >		bullet_debug_draw_;

	std::unique_ptr< Scene >				scene_;					///< 現在のベースシーン
	std::vector< std::unique_ptr< Scene > >	overlay_scene_stack_;	///< オーバーレイシーンのスタック
	string_t								stage_name_;			///< 現在のステージ名

	bool									is_display_fps_;		///< FPS 表示フラグ

	bool									is_command_mode_;		///< 現在コマンドモード中かどうか？
	string_t								user_command_;			///< ユーザーの入力したコマンド

	bool									is_show_cursor_;		///< 現在カーソルを表示するかどうか

	std::unique_ptr< core::setting::SettingStore >	settings_;		///< ゲーム設定（オプション画面が編集する）
	std::unique_ptr< core::ui::StringTable >		ui_strings_;	///< UI の表示文字列
	std::unique_ptr< core::ui::Theme >				ui_theme_;		///< UI の見た目
	bool											is_display_change_requested_ = false;	///< 解像度・フルスクリーンの変更をフレーム末に行う
	bool											is_full_screen_toggle_requested_ = false;	///< ユーザーによるフルスクリーンの切り替え（Alt+Enter / F5）をフレーム末に行う
	bool											is_display_restore_requested_ = false;	///< 再びアクティブになったとき、表示を設定に合わせ直す
	bool											is_shut_down_ = false;					///< shutdown() 済み
	string_t										pending_start_overlay_;			///< 開発用 : 起動時に開くオーバーレイ（debug.start_overlay）
	int												start_overlay_wait_frames_ = 0;

	void setup_script_command();
	void apply_display_settings();

	void update_render_data_for_game() const;

	void check_scene_transition();
	void setup_scene( const string_t& );

	void render_console_window();
	void render_scene_list_window();

public:
	GameMain();
	virtual ~GameMain() override;

	ActiveObject* create_object( const char_t* ) const;
	ActiveObject* clone_object( const ActiveObject* ) const;

	void setup_scene();

	bool update() override;

	void render();

	void on_key_down( char_t ) override;
	void on_special_key_down( int ) override;
	void on_mouse_wheel( int ) override;
	void on_resize() override;
	void on_toggle_full_screen() override { is_full_screen_toggle_requested_ = true; }
	void shutdown() override;
	void on_activated() override { is_display_restore_requested_ = true; }

	Input* get_input() const { return input_.get(); }
	
	OculusRift* get_oculus_rift() const { return oculus_rift_.get(); }

	ActiveObjectPhysics* get_active_object_physics() const { return active_object_physics_.get(); }
	GraphicsManager* get_graphics_manager() const { return graphics_manager_.get(); }
	SoundManager* get_sound_manager() const { return sound_manager_.get(); }
	ScriptManager* get_script_manager() const { return script_manager_.get(); }

	ActiveObjectManager* get_active_object_manager() const { return active_object_manager_.get(); }
	
	static App* get_app() { return App::get_instance(); }

	inline int get_width() const override { return get_app()->get_width(); }
	inline int get_height() const override { return get_app()->get_height(); }

	Config* get_config() const { return get_app()->get_config(); }
	Config* get_save_data() const { return save_data_.get(); }

	const MainLoop* get_main_loop() const { return main_loop_.get(); }

	BulletDebugDraw* get_bullet_debug_draw() const { return bullet_debug_draw_.get(); }

	const std::string& get_stage_name() const { return stage_name_; }
	void set_stage_name( const std::string& stage_name ) { stage_name_ = stage_name; }

	bool is_display_fps() const { return is_display_fps_; }
	void set_display_fps( bool f ) { is_display_fps_ = f; }

	core::setting::SettingStore* get_settings() const { return settings_.get(); }
	const core::ui::StringTable& get_ui_strings() const { return *ui_strings_; }
	const core::ui::Theme& get_ui_theme() const { return *ui_theme_; }

	/// 解像度・フルスクリーンの変更を予約する（UI のコールバック中にウィンドウを変えないよう、update の最後に行う）
	void request_display_change() { is_display_change_requested_ = true; }

	inline static GameMain* get_instance() { static GameMain game_main; return & game_main; }
	const Scene* get_current_scene() const { return scene_.get(); }

	/// オーバーレイシーンをスタックにプッシュする ( ベースシーンはポーズ状態になる )
	void push_overlay_scene( const string_t& scene_name );
	/// オーバーレイシーンをスタックからポップする ( ベースシーンのポーズが解除される )
	void pop_overlay_scene();
	/// オーバーレイシーンが存在するか
	bool has_overlay_scene() const { return ! overlay_scene_stack_.empty(); }

	void set_show_cursor( bool show_cursor ) { is_show_cursor_ = show_cursor; get_app()->show_cursor( is_show_cursor_ ); }
	bool is_show_cursor() const override { return is_show_cursor_; }
};

} // namespace blue_sky

inline game::Game* game::Game::get_instance() { return blue_sky::GameMain::get_instance(); }

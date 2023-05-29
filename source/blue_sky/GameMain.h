#pragma once


#include "App.h"

#include <game/Game.h>

#include <type/type.h>

#include <unordered_map>

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
	class SoundManager;

	namespace graphics::direct_3d_11
	{
		class Direct3D11;
		class BulletDebugDraw;
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

	using PhysicsManager			= ActiveObjectPhysics;
	using GraphicsManager			= GraphicsManager;
	using SoundManager				= core::SoundManager;

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
	float									total_elapsed_time_;	///< ゲームが開始してからの経過時間 ( 秒 )

	std::unique_ptr< Direct3D >				direct_3d_;				///< Direct3D

	std::unique_ptr< DirectInput >			direct_input_;			///< DirectInput
	std::unique_ptr< Input >				input_;					///< Game Input

	std::unique_ptr< OculusRift >			oculus_rift_;			///< Oculus Rift

	std::unique_ptr< PhysicsManager >		physics_manager_;		///< PhysicsManager
	std::unique_ptr< GraphicsManager >		graphics_manager_;		///< GraphicsManager
	std::unique_ptr< SoundManager >			sound_manager_;			///< SoundManager
	std::unique_ptr< ScriptManager >		script_manager_;		///< ScriptManager

	std::unique_ptr< Config >				save_data_;				///< Save Data

	std::unique_ptr< MainLoop >				main_loop_;				///< ループ管理

	std::unique_ptr< ActiveObjectManager >	active_object_manager_;	///< ActiveObjectManager

	std::unique_ptr< BulletDebugDraw >		bullet_debug_draw_;

	std::unique_ptr< Scene >				scene_;					///< 現在のシーン
	string_t								stage_name_;			///< 現在のステージ名

	bool									is_display_fps_;		///< FPS 表示フラグ

	bool									is_command_mode_;		///< 現在コマンドモード中かどうか？
	string_t								user_command_;			///< ユーザーの入力したコマンド

	bool									is_show_cursor_;		///< 現在カーソルを表示するかどうか

	void setup_script_command();

	void update_render_data_for_game() const;

	void check_scene_transition();
	void setup_scene( const string_t& );

	void edit_command( char_t );

public:
	GameMain();
	virtual ~GameMain() override;

	ActiveObject* create_object( const char_t* );
	ActiveObject* clone_object( const ActiveObject* );

	void setup_scene();

	bool update() override;

	void render();

	void on_key_down( char_t ) override;
	void on_special_key_down( int ) override;
	void on_mouse_wheel( int ) override;
	void on_resize() override;

	float_t get_total_elapsed_time() const { return total_elapsed_time_; }
	float_t get_elapsed_time() const;

	/// @todo GraphicsManager に移行して削除する
	Direct3D* get_direct_3d() const { return direct_3d_.get(); } 

	Input* get_input() const { return input_.get(); }
	
	OculusRift* get_oculus_rift() const { return oculus_rift_.get(); }

	PhysicsManager* get_physics_manager() const { return physics_manager_.get(); }
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

	inline static GameMain* get_instance() { static GameMain game_main; return & game_main; }
	const Scene* get_current_scene() const { return scene_.get(); }

	void set_show_cursor( bool show_cursor ) { is_show_cursor_ = show_cursor; get_app()->show_cursor( is_show_cursor_ ); }
	bool is_show_cursor() const override { return is_show_cursor_; }
};

} // namespace blue_sky

inline game::Game* game::Game::get_instance() { return blue_sky::GameMain::get_instance(); }

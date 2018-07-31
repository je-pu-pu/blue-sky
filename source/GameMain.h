#pragma once

#include "Game.h"
#include "App.h"

#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

#include <type/type.h>

class Direct3D11;
class Direct3D11BulletDebugDraw;

class DirectWrite;

class DirectInput;

class OculusRift;

namespace game
{
	class Config;
	class MainLoop;
};

namespace blue_sky
{
	class Input;

	class ActiveObjectPhysics;

	class ActiveObject;
	class ActiveObjectManager;

	class SoundManager;
	class ScriptManager;

	class DrawingModel;
	class DrawingModelManager;

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

class GameMain : public Game
{
public:
	typedef game::Config					Config;
	typedef game::MainLoop					MainLoop;

	typedef Direct3D11						Direct3D;
	
	typedef ActiveObjectPhysics				PhysicsManager;
	typedef blue_sky::GraphicsManager		GraphicsManager;
	typedef blue_sky::SoundManager			SoundManager;

	typedef blue_sky::Player				Player;

	typedef std::function< Scene* () > CreateSceneFunction;
	typedef std::unordered_map< string_t, CreateSceneFunction > CreateSceneFunctionMap;

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

protected:
	CreateSceneFunctionMap					scene_creator_map_;		///< シーン生成マップ

	float									total_elapsed_time_;	///< ゲームが開始してからの経過時間 ( 秒 )

	common::auto_ptr< Direct3D >			direct_3d_;				///< Direct3D

	common::auto_ptr< DirectInput >			direct_input_;			///< DirectInput
	common::auto_ptr< Input >				input_;					///< Game Input

	common::auto_ptr< OculusRift >			oculus_rift_;			///< Oculus Rift

	common::auto_ptr< PhysicsManager >		physics_manager_;		///< PhysicsManager
	common::auto_ptr< GraphicsManager >		graphics_manager_;		///< GraphicsManager
	common::auto_ptr< SoundManager >		sound_manager_;			///< SoundManager
	common::auto_ptr< ScriptManager >		script_manager_;		///< ScriptManager

	common::auto_ptr< Config >				save_data_;				///< Save Data

	common::auto_ptr< MainLoop >			main_loop_;				///< ループ管理

	common::auto_ptr< ActiveObjectManager >	active_object_manager_;	///< ActiveObjectManager

	common::auto_ptr< Direct3D11BulletDebugDraw >	bullet_debug_draw_;

	common::safe_ptr< Scene >				scene_;					///< 現在のシーン
	string_t								stage_name_;			///< 現在のステージ名

	bool									is_display_fps_;		///< FPS 表示フラグ

	bool									is_command_mode_;		///< 現在コマンドモード中かどうか？
	string_t								user_command_;			///< ユーザーの入力したコマンド

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
	
	static App* get_app() { return App::GetInstance();  }

	inline int get_width() const override { return get_app()->get_width(); }
	inline int get_height() const override { return get_app()->get_height(); }

	Config* get_config() const { return get_app()->get_config(); }
	Config* get_save_data() const { return save_data_.get(); }

	const MainLoop* get_main_loop() const { return main_loop_.get(); }

	Direct3D11BulletDebugDraw* get_bullet_debug_draw() const { return bullet_debug_draw_.get(); }

	const std::string& get_stage_name() const { return stage_name_; }
	void set_stage_name( const std::string& stage_name ) { stage_name_ = stage_name; }

	bool is_display_fps() const { return is_display_fps_; }

	inline static GameMain* get_instance() { static GameMain game_main; return & game_main; }
	const Scene* get_current_scene() const { return scene_.get(); }

};

} // namespace blue_sky

inline Game* Game::get_instance() { return blue_sky::GameMain::get_instance(); }

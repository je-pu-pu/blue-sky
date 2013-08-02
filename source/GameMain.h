#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "Game.h"

#include "type.h"

#include "ConstantBuffer.h"

#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

class Direct3D11;
class Direct3D11ShadowMap;
class Direct3D11SkyBox;
class Direct3D11BulletDebugDraw;

class DirectWrite;

class DirectInput;

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

	class GraphicsManager;
	class SoundManager;

	class DrawingModel;
	class DrawingModelManager;

	class Player;
	class Camera;

	class Scene;
};

namespace blue_sky
{

class GameMain : public Game
{
public:
	typedef game::Config					Config;
	typedef game::MainLoop					MainLoop;

	typedef Direct3D11						Direct3D;
	typedef ActiveObjectPhysics				Physics;

	typedef blue_sky::GraphicsManager		GraphicsManager;
	typedef blue_sky::SoundManager			SoundManager;
	typedef blue_sky::Player				Player;

protected:
	float									total_elapsed_time_;	///< ゲームが開始してからの経過時間 ( 秒 )

	common::auto_ptr< Direct3D >			direct_3d_;				///< Direct3D
	common::auto_ptr< Physics >				physics_;				///< Physics

	common::auto_ptr< DirectInput >			direct_input_;			///< DirectInput
	common::auto_ptr< Input >				input_;					///< Game Input

	common::auto_ptr< GraphicsManager >		graphics_manager_;		///< GraphicsManager
	common::auto_ptr< SoundManager >		sound_manager_;			///< SoundManager

	common::auto_ptr< Config >				config_;				///< Config
	common::auto_ptr< Config >				save_data_;				///< Save Data

	common::auto_ptr< MainLoop >			main_loop_;				///< ループ管理

	common::auto_ptr< ActiveObjectManager >	active_object_manager_;	///< ActiveObjectManager

	common::auto_ptr< DrawingModelManager >	drawing_model_manager_;	///< DrawingModelManager

	/// @todo まとめる
	common::auto_ptr< GameConstantBuffer >			game_constant_buffer_;
	common::auto_ptr< FrameConstantBuffer >			frame_constant_buffer_;
	common::auto_ptr< FrameDrawingConstantBuffer >	frame_drawing_constant_buffer_;
	common::auto_ptr< ObjectConstantBuffer >		object_constant_buffer_;

	common::auto_ptr< Direct3D11BulletDebugDraw >	bullet_debug_draw_;

	common::safe_ptr< Scene >				scene_;					///< 現在のシーン
	string_t								stage_name_;			///< 現在のステージ名

	bool									is_display_fps_;		///< FPS 表示フラグ

	void check_scene_transition();
	void setup_scene( const string_t& );

public:
	GameMain();
	virtual ~GameMain();

	bool update();

	void render();


	void on_function_key_down( int );
	void on_mouse_wheel( int );
	void on_resize();

	float_t get_total_elapsed_time() const { return total_elapsed_time_; }
	float_t get_elapsed_time() const;

	Direct3D* get_direct_3d() const { return direct_3d_.get(); }
	Physics* get_physics() const { return physics_.get(); }

	Input* get_input() const { return input_.get(); }

	GraphicsManager* get_graphics_manager() const { return graphics_manager_.get(); }
	SoundManager* get_sound_manager() const { return sound_manager_.get(); }

	ActiveObjectManager* get_active_object_manager() const { return active_object_manager_.get(); }

	DrawingModelManager* get_drawing_model_manager() const { return drawing_model_manager_.get(); }
	
	Config* get_config() const { return config_.get(); }
	Config* get_save_data() const { return save_data_.get(); }

	const MainLoop* get_main_loop() const { return main_loop_.get(); }

	/// @todo 整理する
	GameConstantBuffer* get_game_constant_buffer() const { return game_constant_buffer_.get(); }
	FrameConstantBuffer* get_frame_constant_buffer() const { return frame_constant_buffer_.get(); }
	FrameDrawingConstantBuffer* get_frame_drawing_constant_buffer() const { return frame_drawing_constant_buffer_.get(); }
	ObjectConstantBuffer* get_object_constant_buffer() const { return object_constant_buffer_.get(); }
	
	Direct3D11BulletDebugDraw* get_bullet_debug_draw() const { return bullet_debug_draw_.get(); }

	const std::string& get_stage_name() const { return stage_name_; }
	void set_stage_name( const std::string& stage_name ) { stage_name_ = stage_name; }

	bool is_display_fps() const { return is_display_fps_; }

	inline static GameMain* get_instance() { static GameMain game_main; return & game_main; }
	const Scene* get_current_scene() const { return scene_.get(); }

};

} // namespace blue_sky

inline Game* Game::get_instance() { return blue_sky::GameMain::get_instance(); }


#endif // GAME_MAIN_H
#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "Game.h"

#include "type.h"

#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

class Direct3D11;
class Direct3D11ConstantBuffer;
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

	class SoundManager;

	class DrawingModel;
	class DrawingModelManager;

	class Player;
	class Camera;

	class Scene;
};

using namespace game;
using namespace blue_sky;

using common::auto_ptr;
using common::safe_ptr;

namespace blue_sky
{

class GameMain : public Game
{
public:
	typedef Direct3D11						Direct3D;
	typedef ActiveObjectPhysics				Physics;

	typedef blue_sky::SoundManager			SoundManager;
	typedef blue_sky::Player				Player;

protected:
	float									elapsed_;				///< ゲームが開始してからの経過時間 ( 秒 )

	auto_ptr< Direct3D >					direct_3d_;				///< Direct3D
	auto_ptr< Physics >						physics_;				///< Physics

	auto_ptr< DirectInput >					direct_input_;			///< DirectInput
	auto_ptr< Input >						input_;					///< Game Input
	auto_ptr< SoundManager >				sound_manager_;			///< SoundManager

	auto_ptr< Config >						config_;				///< Config
	auto_ptr< Config >						save_data_;				///< Save Data

	auto_ptr< MainLoop >					main_loop_;				///< ループ管理

	auto_ptr< ActiveObjectManager >			active_object_manager_;	///< ActiveObjectManager

	auto_ptr< DrawingModelManager >			drawing_model_manager_;	///< DrawingModelManager

	auto_ptr< Direct3D11ConstantBuffer >	game_constant_buffer_;
	auto_ptr< Direct3D11ConstantBuffer >	frame_constant_buffer_;
	auto_ptr< Direct3D11ConstantBuffer >	object_constant_buffer_;

	auto_ptr< Direct3D11ShadowMap >			shadow_map_;
	auto_ptr< Direct3D11SkyBox >			sky_box_;

	auto_ptr< DirectWrite >					direct_write_;
	auto_ptr< Direct3D11BulletDebugDraw >	bullet_debug_draw_;

	auto_ptr< Player >						player_;
	auto_ptr< Camera >						camera_;

	safe_ptr< Scene >						scene_;					///< 現在のシーン
	std::string								stage_name_;			///< 現在のステージ名

	void render( const ActiveObject* );
	void render_line( const ActiveObject* );

public:
	GameMain();
	virtual ~GameMain();

	bool update();
	void render();

	float_t get_elapsed() const { return elapsed_; }
	// void clear_elapsed() { elapsed_ = 0.f; }

	Direct3D* get_direct_3d() const { return direct_3d_.get(); }
	Physics* get_physics() const { return physics_.get(); }

	Input* get_input() const { return input_.get(); }
	SoundManager* get_sound_manager() const { return sound_manager_.get(); }

	ActiveObjectManager* get_active_object_manager() const { return active_object_manager_.get(); }

	DrawingModelManager* get_drawing_model_manager() const { return drawing_model_manager_.get(); }
	
	Config* get_config() const { return config_.get(); }
	Config* get_save_data() const { return save_data_.get(); }

	// MainLoop* get_main_loop() const { return main_loop_.get(); }

	Direct3D11ConstantBuffer* get_game_constant_buffer() const { return game_constant_buffer_.get(); }
	Direct3D11ConstantBuffer* get_frame_constant_buffer() const { return frame_constant_buffer_.get(); }
	Direct3D11ConstantBuffer* get_object_constant_buffer() const { return object_constant_buffer_.get(); }

	inline static GameMain* get_instance() { static GameMain game_main; return & game_main; }
};

} // namespace blue_sky

inline Game* Game::getInstance() { return GameMain::get_instance(); }


#endif // GAME_MAIN_H
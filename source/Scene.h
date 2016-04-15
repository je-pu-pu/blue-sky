#ifndef BLUE_SKY_SCENE_H
#define BLUE_SKY_SCENE_H

#include "type.h"

#include <d3d11.h>
#include <functional>

class App;
class Direct3D11;
class Direct3D11Color;
class Direct3D11Texture;

class OculusRift;

namespace game
{

class Sound;
class Config;
class MainLoop;
class Texture;

} // namespace game

namespace blue_sky
{

class GameMain;
class ActiveObjectPhysics;
class ActiveObjectManager;
class DrawingModelManager;
class GraphicsManager;
class SoundManager;
class Input;

class DrawingModel;

/**
 * 場面ごとの処理、描画を管理する
 *
 */
class Scene
{
public:
	typedef game::Sound					Sound;
	typedef game::Config				Config;
	typedef game::MainLoop				MainLoop;
	
	typedef game::Texture				Texture;

	typedef Direct3D11					Direct3D;
	// typedef Direct3D11Texture			Texture;

	typedef ActiveObjectPhysics			Physics;

	typedef Direct3D11Color				Color;

private:
	const GameMain* game_main_;

	float_t total_elapsed_time_;		///< シーンが開始してからの総経過時間 ( 秒 )

	std::string name_;
	std::string next_scene_;
	std::string next_stage_name_;

protected:
	void update_total_elapsed_time();
	float_t get_elapsed_time() const;

	Direct3D* get_direct_3d() const;	///< @todo 削除する
	Physics* get_physics() const;

	ActiveObjectManager* get_active_object_manager() const;
	DrawingModelManager* get_drawing_model_manager() const;

	GraphicsManager* get_graphics_manager() const;
	SoundManager* get_sound_manager() const;

	Input* get_input() const;
	Config* get_config() const;
	Config* get_save_data() const;

	OculusRift* get_oculus_rift() const;

	const MainLoop* get_main_loop() const;

	const GameMain* get_game_main() const { return game_main_; }

	const App* get_app() const;

	int get_width() const;
	int get_height() const;


	void play_sound( const char*, bool = false, bool = true ) const;
	void stop_sound( const char* ) const;

	void update_constant_buffer_for_sprite_frame( int line_type, float_t drawing_accent_scale = 1.f );
	
	void render_technique( const char_t*, std::function< void() > ) const;
	void render_fader() const;

	virtual Sound* get_bgm() { return 0; };

public:
	Scene( const GameMain* );
	virtual ~Scene();

	void set_name( const std::string& name ) { name_ = name; }
	const std::string& get_name() const { return name_; }

	virtual void update();					///< メインループ
	virtual void render() = 0;				///< 描画

	virtual void on_function_key_down( int ) { };

	bool is_first_game_play() const;
	unsigned int get_current_time() const;

	const std::string& get_next_scene() const { return next_scene_; }
	void set_next_scene( const std::string& next_scene ) { next_scene_ = next_scene; }

	const std::string& get_stage_name() const;
	
	const std::string& get_next_stage_name() const { return next_stage_name_; }
	void set_next_stage_name( const std::string& next_stage_name ) { next_stage_name_ = next_stage_name; }

	virtual bool is_clip_cursor_required() { return false; }

	float_t get_total_elapsed_time() const { return total_elapsed_time_; }
	void reset_total_elapsed_time() { total_elapsed_time_ = 0.f; }

}; // class Scene

} // namespace blue_sky

#endif // BLUE_SKY_SCENE_H
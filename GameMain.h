//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>
#include <string>

namespace game
{

class Sound;
class Config;

} // namespace game

class App;
class Direct3D9;
class DirectInput;

namespace blue_sky
{

class Input;
class SoundManager;
class GridDataManager;
class GridObjectManager;
class ActiveObjectManager;
class Scene;

/**
 * ゲーム全体を管理する
 *
 * ゲーム全体で使用するリソースを保持する
 */
class GameMain
{
public:
	typedef game::Sound Sound;
	typedef game::Config Config;

protected:
	App*					app_;					///< Application

	Direct3D9*				direct_3d_;				///< Direct3D
	DirectInput*			direct_input_;			///< DirectInput

	Input*					input_;					///< Input
	SoundManager*			sound_manager_;			///< SoundManager

	GridDataManager*		grid_data_manager_;		///< GridDataManager
	GridObjectManager*		grid_object_manager_;	///< GridObjectManager

	ActiveObjectManager*	active_object_manager_;	///< ActiveObjectManager

	Config*					config_;				///< Config

	CMainLoop				MainLoop;				///< ループ管理

	Scene*					scene_;					///< 現在のシーン
	std::string				stage_name_;			///< 現在のステージ名

	bool					display_fps_flag_;		///< FPS 表示フラグ

	GameMain();

public:
	static GameMain* getInstance() { static GameMain gm; return & gm; }
	~GameMain();				//デストラクタ

	bool update();				///< メインループ
	void render();				///< 描画

	bool is_first_game_play() const;

	/// 現在の時間をミリ秒で返す
	unsigned int get_current_time() const { return MainLoop.get_current_time(); }

	int get_width() const;
	int get_height() const;

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	Direct3D9* get_direct_3d() const { return direct_3d_; }
	Input* get_input() const { return input_; }
	SoundManager* get_sound_manager() const { return sound_manager_; }
	
	GridDataManager* get_grid_data_manager() const { return grid_data_manager_; }
	GridObjectManager* get_grid_object_manager() const { return grid_object_manager_; }
	ActiveObjectManager* get_active_object_manager() const { return active_object_manager_; }
	
	Config* get_config() const { return config_; }

	void on_reset_key_down();
	void on_function_key_down( int );
	void on_mouse_wheel( int );

	bool is_clip_cursor_required() const;

	const std::string& get_stage_name() const { return stage_name_; }
	void set_stage_name( const std::string& stage_name ) { stage_name_ = stage_name; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
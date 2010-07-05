//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

namespace game
{

class Config;

} // namespace game

class App;
class Direct3D9;

namespace blue_sky
{

class Input;
class SoundManager;
class GridObjectManager;
class Scene;

/**
 * ゲーム全体を管理する
 *
 * ゲーム全体で使用するリソースを保持する
 */
class GameMain
{
public:
	typedef game::Config Config;

protected:
	App*					app_;					///< Application

	Direct3D9*				direct_3d_;				///< Direct3D

	Input*					input_;					///< Input
	SoundManager*			sound_manager_;			///< SoundManager

	GridObjectManager*		grid_object_manager_;	///< GridObjectManager

	Config*					config_;				///< Config

	CMainLoop				MainLoop;				///< ループ管理

	Scene*					scene_;					///< 現在のシーン

	GameMain();

public:
	static GameMain* getInstance() { static GameMain gm; return & gm; }
	~GameMain();				//デストラクタ

	void update();				///< メインループ
	void render();				///< 描画

	int get_width() const;
	int get_height() const;

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	Direct3D9* get_direct_3d() const { return direct_3d_; }
	Input* get_input() const { return input_; }
	SoundManager* get_sound_manager() const { return sound_manager_; }
	GridObjectManager* get_grid_object_manager() const { return grid_object_manager_; }
	Config* get_config() const { return config_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
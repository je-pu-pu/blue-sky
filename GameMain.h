//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡
//GameMain.h
//ÅIXVú	2001/11/24
//ÅIXVª	EBhEYÇªðApp.cppÉª£
//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

namespace game
{

class Sound;
class Config;

} // namespace game

class App;
class Direct3D9;

namespace blue_sky
{

class Input;
class SoundManager;
class GridDataManager;
class GridObjectManager;
class Scene;

/**
 * Q[SÌðÇ·é
 *
 * Q[SÌÅgp·é\[XðÛ·é
 */
class GameMain
{
public:
	typedef game::Sound Sound;
	typedef game::Config Config;

protected:
	App*					app_;					///< Application

	Direct3D9*				direct_3d_;				///< Direct3D

	Input*					input_;					///< Input
	SoundManager*			sound_manager_;			///< SoundManager

	GridDataManager*		grid_data_manager_;		///< GridDataManager
	GridObjectManager*		grid_object_manager_;	///< GridObjectManager

	Config*					config_;				///< Config

	CMainLoop				MainLoop;				///< [vÇ

	Scene*					scene_;					///< »ÝÌV[

	GameMain();

public:
	static GameMain* getInstance() { static GameMain gm; return & gm; }
	~GameMain();				//fXgN^

	void update();				///< C[v
	void render();				///< `æ

	int get_width() const;
	int get_height() const;

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	Direct3D9* get_direct_3d() const { return direct_3d_; }
	Input* get_input() const { return input_; }
	SoundManager* get_sound_manager() const { return sound_manager_; }
	
	GridDataManager* get_grid_data_manager() const { return grid_data_manager_; }
	GridObjectManager* get_grid_object_manager() const { return grid_object_manager_; }
	
	Config* get_config() const { return config_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
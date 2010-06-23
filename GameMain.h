//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡
//GameMain.h
//ÅIXVú	2001/11/24
//ÅIXVª	EBhEYÇªðApp.cppÉª£
//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

class Direct3D9;

namespace blue_sky
{

class Input;
class SoundManager;

class GameMain
{
protected:
	Direct3D9*		direct_3d_;		///< Direct3D

	Input*			input_;			///< Input
	SoundManager*	sound_manager_;	///< SoundManager

	int				Width;			///< ¡
	int				Height;			///< ³

	CMainLoop		MainLoop;		///< [vÇ

	GameMain();					///< RXgN^

public:
	static GameMain* getInstance(){ static GameMain gm; return & gm; }
	~GameMain();				//fXgN^

	void update();				///< C[v
	void render();				///< `æ

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	SoundManager* getSoundManager() { return sound_manager_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
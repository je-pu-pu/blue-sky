//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

class Direct3D9;

namespace blue_sky
{

class SoundManager;

class GameMain
{
protected:
	Direct3D9*		direct_3d_;		///< Direct3D

	SoundManager*	sound_manager_;	///< SoundManager

	int				Width;			///< 横幅
	int				Height;			///< 高さ

	CMainLoop		MainLoop;		///< ループ管理

	GameMain();					///< コンストラクタ

public:
	static GameMain* getInstance(){ static GameMain gm; return & gm; }
	~GameMain();				//デストラクタ

	void update();				///< メインループ
	void render();				///< 描画

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	SoundManager* getSoundManager() { return sound_manager_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
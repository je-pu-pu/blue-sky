//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

class Direct3D9;
class DirectSound;

class CGameMain
{
protected:
	Direct3D9*		direct_3d_;		///< Direct3D
	DirectSound*	direct_sound_;	///< DirectSound

	int				Width;			///< 横幅
	int				Height;			///< 高さ

	CMainLoop		MainLoop;		///< ループ管理

	CGameMain();					///< コンストラクタ

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//デストラクタ

	void update();				///< メインループ
	void render();				///< 描画

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }
};

#endif
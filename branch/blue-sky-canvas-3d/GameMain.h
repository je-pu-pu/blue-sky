//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"

#include <windows.h>
#include <string>

// uses
class CDirectInput;
class CDirectInputDevice;

class vector3;

class CGameMain
{
public:

protected:
	HWND		hwnd_;			///< ウィンドウハンドル
	int			Width;			///< 横幅
	int			Height;			///< 高さ


	CMainLoop	MainLoop;		//ループ管理

	CGameMain();				//コンストラクタ

	void convert_3d_to_2d( vector3& );

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//デストラクタ

	void	Loop();				//メインループ

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
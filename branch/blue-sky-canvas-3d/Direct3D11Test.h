#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"

#include "Type.h"

#include <windows.h>
#include <string>

class Direct3D11;

class CGameMain
{
protected:
	Direct3D11*	direct_3d_;		///< Direct3D
		
	HWND		hwnd_;			///< ウィンドウハンドル
	int			Width;			///< 横幅
	int			Height;			///< 高さ

	CMainLoop	MainLoop;		//ループ管理

	CGameMain();				//コンストラクタ

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return & gm; }
	~CGameMain();				//デストラクタ

	void	Loop();				//メインループ
	void	render();

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
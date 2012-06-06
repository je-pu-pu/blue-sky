//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//MainLoop.h
//最終更新日	2001/9/14
//最終更新部分	GetFPS()
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef		_MAIN_LOOP_H_
#define		_MAIN_LOOP_H_

#include	<Windows.h>

//メインループクラス
class	CMainLoop
{
private:
	DWORD	LastTime;	//前回の時間
	DWORD	NowTime;	//現在の時間

	int		FPSCount;	//FPS
	int		FPS;		//FPS	デバッグ用
	DWORD	LastSec;	//

public:
	bool	Active;		//アクティブフラグ
	DWORD	WaitTime;	//待機時間

	CMainLoop();
	bool Loop();
	
	//Get
	DWORD GetNowTime() const { return NowTime; }
	int GetFPS() const { return FPS; }
};

#endif
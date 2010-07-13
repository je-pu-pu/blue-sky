//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//MainLoop.h
//最終更新日	2001/9/14
//最終更新部分	GetFPS()
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include <windows.h>

/**
 * メインループクラス
 * 
 */
class CMainLoop
{
private:
	DWORD	LastTime;	//前回の時間
	DWORD	current_time_;	//現在の時間

	int		FPSCount;	//FPS
	int		FPS;		//FPS	デバッグ用
	DWORD	LastSec;	//

public:
	bool	Active;		//アクティブフラグ
	DWORD	WaitTime;	//待機時間

	CMainLoop();
	bool Loop();
	
	//Get
	DWORD get_current_time() const { return current_time_; }
	int GetFPS() const { return FPS; }
};

#endif
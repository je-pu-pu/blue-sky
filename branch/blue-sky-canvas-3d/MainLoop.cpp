//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//MainLoop.cpp
//最終更新日	2001/9/7
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#include	<Windows.h>
#include	"MainLoop.h"

#pragma comment (lib, "winmm.lib")

//■コンストラクタ
CMainLoop::CMainLoop()
{
	Active = true;
	WaitTime = 20;	//待機時間初期値 20 ( 50 FPS )
}

//■ループ
//指定された時間が経過していればtureを返す
bool CMainLoop::Loop()
{
	//アクティブでなければ終了
	if(! Active){
		return false;
	}
	//現在の時間を取得
	NowTime = timeGetTime();
	//指定された時間が経過したかチェック
	if(NowTime - LastTime >= WaitTime){
		//
		if(NowTime - LastSec >= 1000){
			FPS = FPSCount;
			FPSCount = 0;
			LastSec = NowTime;
		}
		FPSCount++;
		//
		LastTime = NowTime;
		return true;
	}
	return false;
}

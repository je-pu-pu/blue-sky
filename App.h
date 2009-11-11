//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//	App.h
//	シングルトンアプリケーションクラス
//	最終更新日	2001/11/25
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef		_APP_H_
#define		_APP_H_

#include	<Windows.h>
#include	<string>
using namespace std;

class CApp
{	
private:
	HINSTANCE	hInst;					//インスタンスハンドル
	HWND		hWnd;					//ウィンドウハンドル
	HANDLE		hMutex;					//ミューテックスハンドル

	int			Width;					//ウィンドウ横幅
	int			Height;					//ウィンドウ高さ

	CApp();								//コンストラクタ

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
public:
	string		ClassName;				//クラス名
	string		WinTitle;				//タイトル
	DWORD		WinStyle;				//スタイル

	bool		Init(HINSTANCE, int);	//初期化
	int			MessageLoop();			//メッセージループ
	virtual		~CApp();				//デストラクタ
	
	//get
	static CApp* GetInstance(){ static CApp app; return &app; }
	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	int			GetWidth(){ return Width; }
	int			GetHeight(){ return Height; }
};

#endif
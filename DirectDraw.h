//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//DirectDraw.h
//最終更新者	
//最終更新日	2001/9/3
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef		_DIRECT_DRAW_H_
#define		_DIRECT_DRAW_H_

#include	<ddraw.h>

#pragma comment (lib, "ddraw.lib")
#pragma comment (lib, "dxguid.lib")

#ifndef		RELEASE
#define		RELEASE(x)		if(x){ x->Release(); x = NULL; }
#endif

class CDirectDrawPrimary;
class CDirectDrawBackBuffer;

//DirectDrawクラス
class CDirectDraw
{
protected:
	HWND					hWnd;			//ウィンドウハンドル
	int						Width;			//横幅
	int						Height;			//高さ
	int						Bpp;			//色数

	LPDIRECTDRAW7			lpDirectDraw;
	CDirectDrawPrimary		*lpPrimary;		//プライマリ
	CDirectDrawBackBuffer	*lpBack;		//バックバッファ
	LPDIRECTDRAWPALETTE		lpPalette;

	bool CreatePrimary();					//プライマリ生成
	bool CreatePalette();					//パレット生成
public:
	CDirectDraw();							//コンストラクタ
	~CDirectDraw();							//デストラクタ
	
	bool Init(HWND, DWORD, DWORD, DWORD);	//初期化
	int UnInit();							//終了処理

	void Flip();							//フリップ
	bool Capture(char*);					//キャプチャ

	bool LostCheck();						//ロストチェック・復元

	//Get
	HWND GetWindowHandle(){ return hWnd; }
	int GetWidth(){ return Width; }
	int GetHeight(){ return Height; }
	LPDIRECTDRAW7 GetObject(){ return lpDirectDraw; }
	CDirectDrawPrimary* GetPrimary(){ return lpPrimary; }
	CDirectDrawBackBuffer* GetBackBuffer(){ return lpBack; }
	LPDIRECTDRAWPALETTE GetPalette(){ return lpPalette; }
};

#endif
//กกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกก
//DirectDraw.h
//ลIXVา	
//ลIXV๚	2001/9/3
//กกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกกก

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

//DirectDrawNX
class CDirectDraw
{
protected:
	HWND					hWnd;			//EBhEnh
	int						Width;			//ก
	int						Height;			//ณ
	int						Bpp;			//F

	LPDIRECTDRAW7			lpDirectDraw;
	CDirectDrawPrimary		*lpPrimary;		//vC}
	CDirectDrawBackBuffer	*lpBack;		//obNobt@
	LPDIRECTDRAWPALETTE		lpPalette;

	bool CreatePrimary();					//vC}ถฌ
	bool CreatePalette();					//pbgถฌ
public:
	CDirectDraw();							//RXgN^
	~CDirectDraw();							//fXgN^
	
	bool Init(HWND, DWORD, DWORD, DWORD);	//๚ป
	int UnInit();							//Iน

	void Flip();							//tbv
	bool Capture(char*);					//Lv`

	bool LostCheck();						//Xg`FbNEณ

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
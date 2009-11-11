//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//DirectDrawSurface.h
//最終更新日	2001/11/27
//最終更新部分	DrawLine()
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef		_DIRECT_DRAW_SURFACE_H_
#define		_DIRECT_DRAW_SURFACE_H_

#include	<ddraw.h>
#include	<string>
#include	<vector>
using namespace std;

class CDirectDraw;

//DirectDrawSurfaceクラス
class CDirectDrawSurface
{
protected:
	CDirectDraw				*lpDirectDraw;	//DirectDraw
	LPDIRECTDRAWSURFACE7	lpSurface;		//オフスクリーンサーフェイス
	DDSURFACEDESC2			SurfaceDesc;	//直接描画用
	string					FileName;		//読み込んだファイルの名前

	int						Width;			//横幅
	int						Height;			//高さ

	bool					Locked;			//ロックフラグ

	void	ScanLine(RGBQUAD*, int, int, RGBQUAD);
	void	ScanLine(RGBQUAD*, int, int, RGBQUAD, RGBQUAD);
	void	RGB_Swap(DWORD&);
public:
	CDirectDrawSurface();					//コンストラクタ
	~CDirectDrawSurface();					//デストラクタ

	//サーフェイス生成
	bool	Create(CDirectDraw*, DWORD, DWORD, DWORD=0);
	bool	CreateOnVideoMemory(CDirectDraw*, DWORD, DWORD);
	bool	CreateOnSystemMemory(CDirectDraw*, DWORD, DWORD);
	//Bitmap読み込み
	bool	Load(string);
	//ロストチェック，復元
	bool	LostCheck();

	//画像転送
	void	Clipping(int&, int&, CDirectDrawSurface*, RECT&);
	void	Blt(int, int, int, int,	CDirectDrawSurface*, int, int, bool);
	void	Blt(int, int, CDirectDrawSurface*, RECT, bool);
	void	Blt(RECT, CDirectDrawSurface*, RECT, bool);
	void	DirectBlt(int, int, int, int, CDirectDrawSurface*, int, int, bool);
	void	DirectBlt(int, int, CDirectDrawSurface*, RECT, bool);
	void	DirectBlt(RECT, CDirectDrawSurface*, RECT, bool);
	void	FillRect(RECT, DWORD);
	void	Clear(DWORD);
	void	TextOut(int, int, const char*);
	void	Line(POINT, POINT);
	//直接描画
	void	Lock();
	void	UnLock();
	void	DrawLine(int, int, int, int, RGBQUAD);
	void	DrawLine(double, double, double, double, RGBQUAD);

	void	DrawLineHumanTouch( double, double, double, double, RGBQUAD );

	void	DrawPolygon(POINT*, RGBQUAD); 
	void	DrawPolygon(POINT*, RGBQUAD*);
	void	DrawPolygonHumanTouch( POINT*, RGBQUAD );

	void	DrawCircle(int, int, int, RGBQUAD, bool);
	void	DrawCircle( double, double, double, RGBQUAD, bool );

	void	DrawRing(int, int, int, int, RGBQUAD, bool);

	void	Blur(RECT, vector<POINT>&);
	void	Blur(CDirectDrawSurface*, RECT, vector<POINT>&);

	bool	LineClipping(int, int, int, int);

	//Get
	LPDIRECTDRAWSURFACE7& GetSurface(){ return lpSurface; }
	DDSURFACEDESC2* GetSurfaceDesc(){ return &SurfaceDesc; }
	int GetWidth(){ return Width; }
	int GetHeight(){ return Height; }
	bool GetLocked(){ return Locked; }
};

//DirectDrawプライマリサーフェイスクラス
class CDirectDrawPrimary : public CDirectDrawSurface
{
public:
	void SetWidth(int w){ Width = w; }
	void SetHeight(int h){ Height = h; }
};
//DirectDrawバックバッファクラス
//デストラクタでサーフェスを解放しない
class CDirectDrawBackBuffer : public CDirectDrawPrimary
{
public:
	~CDirectDrawBackBuffer(){ lpSurface = NULL; }
};

extern bool g_solid;
extern bool g_line;

extern double g_power;
extern double g_power_min;
extern double g_power_max;
extern double g_power_plus;
extern double g_power_rest;
extern double g_power_plus_reset;

extern double g_direction_fix_default;
extern double g_direction_fix_acceleration;
extern double g_direction_random;

#endif

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//GameMain.h
//最終更新日	2001/11/24
//最終更新部分	ウィンドウズ管理部分をApp.cppに分離
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef		_GAME_MAIN_H_
#define		_GAME_MAIN_H_

#include	"MainLoop.h"
#include	<Windows.h>
#include	<string>
using namespace std;

//uses
class CDirectDraw;
class CDirectDrawSurface;
class CDirectInput;
class CDirectInputDevice;

class CGameMain
{
protected:
	HWND		hWnd;			//ウィンドウハンドル
	int			Width;			//横幅
	int			Height;			//高さ
	int			MaxAngle;		//角度幅

	CMainLoop	MainLoop;		//ループ管理
	CDirectDraw	*lpDirectDraw;	//DirectDraw
	CDirectDrawSurface			//DirectDrawSurface
				*lpPrimary,
				*lpBack,
				*lpSrc,
				*lpDst;

	double		*SinTable;		//サインテーブル

	CGameMain();				//コンストラクタ

	void	CreateSinTable();	//サインテーブル作成
public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//デストラクタ

	bool	Init();				//ゲーム初期化
	void	UnInit();			//ゲーム終了処理

	void	Loop();				//メインループ
	void	Quit(string);		//終了命令

	double	Sin(int);			//サイン
	double	Cos(int);			//コサイン
	int		GetMaxAngle(){ return MaxAngle; };

	void	OnKeyDonw(UINT);
	void	OnKeyPress(char);

	void draw_house( const POINT& );
};

#endif
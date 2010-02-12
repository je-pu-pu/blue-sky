//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡
//GameMain.h
//ÅIXVú	2001/11/24
//ÅIXVª	EBhEYÇªðApp.cppÉª£
//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡

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

class vector3;

class CGameMain
{
protected:
	HWND		hWnd;			//EBhEnh
	int			Width;			//¡
	int			Height;			//³
	int			MaxAngle;		//px

	CMainLoop	MainLoop;		//[vÇ
	CDirectDraw	*lpDirectDraw;	//DirectDraw
	CDirectDrawSurface			//DirectDrawSurface
				*lpPrimary,
				*lpBack,
				*lpSrc,
				*lpDst;

	double		*SinTable;		//TCe[u

	CGameMain();				//RXgN^

	void	CreateSinTable();	//TCe[uì¬


	void convert_3d_to_2d( vector3& );

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//fXgN^

	bool	Init();				//Q[ú»
	void	UnInit();			//Q[I¹

	void	Loop();				//C[v
	void	Quit(string);		//I¹½ß

	double	Sin(int);			//TC
	double	Cos(int);			//RTC
	int		GetMaxAngle(){ return MaxAngle; };

	void	OnKeyDonw(UINT);
	void	OnKeyPress(char);

	void draw_house( const POINT& );

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
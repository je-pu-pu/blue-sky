//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡
//GameMain.h
//ÅIXVú	2001/11/24
//ÅIXVª	EBhEYÇªðApp.cppÉª£
//¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡

#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

class Direct3D9;
class DirectSound;

class CGameMain
{
protected:
	Direct3D9*		direct_3d_;		///< Direct3D
	DirectSound*	direct_sound_;	///< DirectSound

	int				Width;			///< ¡
	int				Height;			///< ³

	CMainLoop		MainLoop;		///< [vÇ

	CGameMain();					///< RXgN^

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//fXgN^

	void update();				///< C[v
	void render();				///< `æ

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }
};

#endif
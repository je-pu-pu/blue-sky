#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"

#include "Type.h"

#include <windows.h>
#include <string>

class Direct3D11;

class CGameMain
{
protected:
	Direct3D11*	direct_3d_;		///< Direct3D
		
	HWND		hwnd_;			///< �E�B���h�E�n���h��
	int			Width;			///< ����
	int			Height;			///< ����

	CMainLoop	MainLoop;		//���[�v�Ǘ�

	CGameMain();				//�R���X�g���N�^

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return & gm; }
	~CGameMain();				//�f�X�g���N�^

	void	Loop();				//���C�����[�v
	void	render();

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
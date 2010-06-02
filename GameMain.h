//��������������������������������������������������������������������������������
//GameMain.h
//�ŏI�X�V��	2001/11/24
//�ŏI�X�V����	�E�B���h�E�Y�Ǘ�������App.cpp�ɕ���
//��������������������������������������������������������������������������������

#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "MainLoop.h"

#include "Type.h"

#include <windows.h>
#include <string>

class Direct3D9;

namespace art
{

class Canvas;

}

using namespace std;

// uses
class CDirectInput;
class CDirectInputDevice;

class vector3;

class CGameMain
{
public:
	typedef art::Canvas Canvas;

protected:
	Direct3D9*	direct_3d_;		///< Direct3D
	Canvas*		canvas_;		///< Canvas
		
	HWND		hwnd_;			///< �E�B���h�E�n���h��
	int			Width;			///< ����
	int			Height;			///< ����


	CMainLoop	MainLoop;		//���[�v�Ǘ�

	CGameMain();				//�R���X�g���N�^

	void convert_3d_to_2d( vector3& );

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//�f�X�g���N�^

	void update();				///< ���C�����[�v
	void render();				///< �`��

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
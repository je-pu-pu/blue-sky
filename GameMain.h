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
	Canvas*		canvas_;		///< Canvas
		
	HWND		hwnd_;			///< �E�B���h�E�n���h��
	int			Width;			///< ����
	int			Height;			///< ����

	CMainLoop	MainLoop;		//���[�v�Ǘ�

	CGameMain();				//�R���X�g���N�^

	void convert_3d_to_2d( vector3& );
	
	void draw_2d_buildings() const;
	void draw_house( const art::Vertex& ) const;

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//�f�X�g���N�^

	void	Loop();				//���C�����[�v

	const CMainLoop& getMainLoop() const { return MainLoop; }
};

#endif
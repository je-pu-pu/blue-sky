//��������������������������������������������������������������������������������
//GameMain.h
//�ŏI�X�V��	2001/11/24
//�ŏI�X�V����	�E�B���h�E�Y�Ǘ�������App.cpp�ɕ���
//��������������������������������������������������������������������������������

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

	int				Width;			///< ����
	int				Height;			///< ����

	CMainLoop		MainLoop;		///< ���[�v�Ǘ�

	CGameMain();					///< �R���X�g���N�^

public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//�f�X�g���N�^

	void update();				///< ���C�����[�v
	void render();				///< �`��

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }
};

#endif
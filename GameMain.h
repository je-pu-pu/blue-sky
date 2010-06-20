//��������������������������������������������������������������������������������
//GameMain.h
//�ŏI�X�V��	2001/11/24
//�ŏI�X�V����	�E�B���h�E�Y�Ǘ�������App.cpp�ɕ���
//��������������������������������������������������������������������������������

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

class Direct3D9;

namespace blue_sky
{

class SoundManager;

class GameMain
{
protected:
	Direct3D9*		direct_3d_;		///< Direct3D

	SoundManager*	sound_manager_;	///< SoundManager

	int				Width;			///< ����
	int				Height;			///< ����

	CMainLoop		MainLoop;		///< ���[�v�Ǘ�

	GameMain();					///< �R���X�g���N�^

public:
	static GameMain* getInstance(){ static GameMain gm; return & gm; }
	~GameMain();				//�f�X�g���N�^

	void update();				///< ���C�����[�v
	void render();				///< �`��

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	SoundManager* getSoundManager() { return sound_manager_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
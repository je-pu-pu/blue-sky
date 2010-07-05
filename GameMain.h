//��������������������������������������������������������������������������������
//GameMain.h
//�ŏI�X�V��	2001/11/24
//�ŏI�X�V����	�E�B���h�E�Y�Ǘ�������App.cpp�ɕ���
//��������������������������������������������������������������������������������

#ifndef BLUE_SKY_GAME_MAIN_H
#define BLUE_SKY_GAME_MAIN_H

#include "MainLoop.h"
#include <windows.h>

namespace game
{

class Config;

} // namespace game

class App;
class Direct3D9;

namespace blue_sky
{

class Input;
class SoundManager;
class GridObjectManager;

class GameMain
{
public:
	typedef game::Config Config;

protected:
	App*					app_;					///< Application

	Direct3D9*				direct_3d_;				///< Direct3D

	Input*					input_;					///< Input
	SoundManager*			sound_manager_;			///< SoundManager

	GridObjectManager*		grid_object_manager_;	///< GridObjectManager

	Config*					config_;				///< Config

	CMainLoop				MainLoop;				///< ���[�v�Ǘ�


	GameMain();

public:
	static GameMain* getInstance(){ static GameMain gm; return & gm; }
	~GameMain();				//�f�X�g���N�^

	void update();				///< ���C�����[�v
	void render();				///< �`��

	int get_width() const;
	int get_height() const;

	const CMainLoop& getMainLoop() const { return MainLoop; }
	// float getSPF() const { return }

	SoundManager* getSoundManager() { return sound_manager_; }

}; // class GameMain

} // namespace blue_sky

#endif // BLUE_SKY_GAME_MAIN_H
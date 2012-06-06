//��������������������������������������������������������������������������������
//MainLoop.h
//�ŏI�X�V��	2001/9/14
//�ŏI�X�V����	GetFPS()
//��������������������������������������������������������������������������������

#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include <windows.h>

/**
 * ���C�����[�v�N���X
 * 
 */
class CMainLoop
{
private:
	DWORD	LastTime;	//�O��̎���
	DWORD	current_time_;	//���݂̎���

	int		FPSCount;	//FPS
	int		FPS;		//FPS	�f�o�b�O�p
	DWORD	LastSec;	//

public:
	bool	Active;		//�A�N�e�B�u�t���O
	DWORD	WaitTime;	//�ҋ@����

	CMainLoop();
	bool Loop();
	
	//Get
	DWORD get_current_time() const { return current_time_; }
	int GetFPS() const { return FPS; }
};

#endif
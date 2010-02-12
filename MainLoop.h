//��������������������������������������������������������������������������������
//MainLoop.h
//�ŏI�X�V��	2001/9/14
//�ŏI�X�V����	GetFPS()
//��������������������������������������������������������������������������������

#ifndef		_MAIN_LOOP_H_
#define		_MAIN_LOOP_H_

#include	<Windows.h>

//���C�����[�v�N���X
class	CMainLoop
{
private:
	DWORD	LastTime;	//�O��̎���
	DWORD	NowTime;	//���݂̎���

	int		FPSCount;	//FPS
	int		FPS;		//FPS	�f�o�b�O�p
	DWORD	LastSec;	//

public:
	bool	Active;		//�A�N�e�B�u�t���O
	DWORD	WaitTime;	//�ҋ@����

	CMainLoop();
	bool Loop();
	
	//Get
	DWORD GetNowTime() const { return NowTime; }
	int GetFPS() const { return FPS; }
};

#endif
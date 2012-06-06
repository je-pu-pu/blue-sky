//��������������������������������������������������������������������������������
//MainLoop.cpp
//�ŏI�X�V��	2001/9/7
//��������������������������������������������������������������������������������

#include	<Windows.h>
#include	"MainLoop.h"

#pragma comment (lib, "winmm.lib")

//���R���X�g���N�^
CMainLoop::CMainLoop()
{
	Active = true;
	WaitTime = 20;	//�ҋ@���ԏ����l 20 ( 50 FPS )
}

//�����[�v
//�w�肳�ꂽ���Ԃ��o�߂��Ă����ture��Ԃ�
bool CMainLoop::Loop()
{
	//�A�N�e�B�u�łȂ���ΏI��
	if(! Active){
		return false;
	}
	//���݂̎��Ԃ��擾
	NowTime = timeGetTime();
	//�w�肳�ꂽ���Ԃ��o�߂������`�F�b�N
	if(NowTime - LastTime >= WaitTime){
		//
		if(NowTime - LastSec >= 1000){
			FPS = FPSCount;
			FPSCount = 0;
			LastSec = NowTime;
		}
		FPSCount++;
		//
		LastTime = NowTime;
		return true;
	}
	return false;
}

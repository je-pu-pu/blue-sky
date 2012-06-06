//��������������������������������������������������������������������������������
//MainLoop.cpp
//�ŏI�X�V��	2001/9/7
//��������������������������������������������������������������������������������

#include	<Windows.h>
#include	"MainLoop.h"

#pragma comment (lib, "winmm.lib")

//���R���X�g���N�^
CMainLoop::CMainLoop()
	: current_time_( timeGetTime() )
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
	current_time_ = timeGetTime();
	//�w�肳�ꂽ���Ԃ��o�߂������`�F�b�N
	if(current_time_ - LastTime >= WaitTime){
		//
		if(current_time_ - LastSec >= 1000){
			FPS = FPSCount;
			FPSCount = 0;
			LastSec = current_time_;
		}
		FPSCount++;
		//
		LastTime = current_time_;
		return true;
	}
	return false;
}

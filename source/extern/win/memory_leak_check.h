/**
 * ���̃t�@�C�����C���N���[�h����ƁA�v���O�����I�����ɉ������Ă��Ȃ����������ǂ̃\�[�X�̂ǂ̏ꏊ�Ŋm�ۂ��ꂽ���̂����o�͂����悤�ɂȂ�B
 *
 * ���� : ���̃t�@�C�����C���N���[�h����� ���̏ꏊ�Œ�`���� operator new ���g�p����Ȃ��Ȃ邽�߁A�s����N����B�ꎞ�I�ȃf�o�b�O�̂��߂����Ɏg�p���鎖�B
 */

#ifndef WIN_MEMORY_LEAK_CHECK_H
#define WIN_MEMORY_LEAK_CHECK_H

#include "memory_leak_detection.h"

// new ��u������
#ifdef _DEBUG
#define new ::new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#endif // WIN_MEMORY_LEAK_DETECTION_H

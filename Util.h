//��������������������������������������������������������������������������������
//Util.h
//�ŏI�X�V��	2001/9/16
//��������������������������������������������������������������������������������

#ifndef		_UTIL_H_
#define		_UTIL_H_

#include	<Windows.h>

#define		Free(x)		if(x){ delete[] x; x = NULL; } 

extern char DebugStr[];
extern char DebugStr2[];
extern char DebugStr3[];

RECT Rect(int, int, int, int);
void DebugPrint(const char* = DebugStr);

#endif
//��������������������������������������������������������������������������������
//Util.cpp
//�ŏI�X�V��	2001/9/16
//��������������������������������������������������������������������������������

#include	"Util.h"
#include	<stdio.h>

char DebugStr[200];
char DebugStr2[200];
char DebugStr3[200];

//��RECT�\���̂��쐬
RECT Rect(int left, int top, int right, int bottom)
{
	RECT rc = {left, top, right, bottom};
	return rc;
}

//���f�o�b�O�����o��
void DebugPrint(const char *c)
{
	static FILE *fp;
	static BOOL	First = TRUE;

	//����
	if(First){
		First = FALSE;
		//���Z�b�g
		fp = fopen("Debug.txt", "w");
		if(fp)
			fclose(fp);
		//�t�@�C�����J��
		fp = fopen("Debug.txt", "a");
	}
	if(fp == NULL)
		return;
	fprintf(fp, c);

	fflush(fp);
	//fclose(fp);
}
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//Util.cpp
//最終更新日	2001/9/16
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#include	"Util.h"
#include	<stdio.h>

char DebugStr[200];
char DebugStr2[200];
char DebugStr3[200];

//■RECT構造体を作成
RECT Rect(int left, int top, int right, int bottom)
{
	RECT rc = {left, top, right, bottom};
	return rc;
}

//■デバッグ文字出力
void DebugPrint(const char *c)
{
	static FILE *fp;
	static BOOL	First = TRUE;

	//初回
	if(First){
		First = FALSE;
		//リセット
		fp = fopen("Debug.txt", "w");
		if(fp)
			fclose(fp);
		//ファイルを開く
		fp = fopen("Debug.txt", "a");
	}
	if(fp == NULL)
		return;
	fprintf(fp, c);

	fflush(fp);
	//fclose(fp);
}
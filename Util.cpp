//‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘
//Util.cpp
//ΕIXVϊ	2001/9/16
//‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘

#include	"Util.h"
#include	<stdio.h>

char DebugStr[200];
char DebugStr2[200];
char DebugStr3[200];

//‘RECT\’Μπμ¬
RECT Rect(int left, int top, int right, int bottom)
{
	RECT rc = {left, top, right, bottom};
	return rc;
}

//‘fobOΆoΝ
void DebugPrint(const char *c)
{
	static FILE *fp;
	static BOOL	First = TRUE;

	//ρ
	if(First){
		First = FALSE;
		//Zbg
		fp = fopen("Debug.txt", "w");
		if(fp)
			fclose(fp);
		//t@CπJ­
		fp = fopen("Debug.txt", "a");
	}
	if(fp == NULL)
		return;
	fprintf(fp, c);

	fflush(fp);
	//fclose(fp);
}
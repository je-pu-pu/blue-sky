//��������������������������������������������������������������������������������
//	App.h
//	�V���O���g���A�v���P�[�V�����N���X
//	�ŏI�X�V��	2001/11/25
//��������������������������������������������������������������������������������

#ifndef		_APP_H_
#define		_APP_H_

#include	<Windows.h>
#include	<string>
using namespace std;

class CApp
{	
private:
	HINSTANCE	hInst;					//�C���X�^���X�n���h��
	HWND		hWnd;					//�E�B���h�E�n���h��
	HANDLE		hMutex;					//�~���[�e�b�N�X�n���h��

	int			Width;					//�E�B���h�E����
	int			Height;					//�E�B���h�E����

	CApp();								//�R���X�g���N�^

	static LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
public:
	string		ClassName;				//�N���X��
	string		WinTitle;				//�^�C�g��
	DWORD		WinStyle;				//�X�^�C��

	bool		Init(HINSTANCE, int);	//������
	int			MessageLoop();			//���b�Z�[�W���[�v
	virtual		~CApp();				//�f�X�g���N�^
	
	//get
	static CApp* GetInstance(){ static CApp app; return &app; }
	HINSTANCE	GetInstanceHandle(){ return hInst; }
	HWND		GetWindowHandle(){ return hWnd; }
	int			GetWidth(){ return Width; }
	int			GetHeight(){ return Height; }
};

#endif
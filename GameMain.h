//��������������������������������������������������������������������������������
//GameMain.h
//�ŏI�X�V��	2001/11/24
//�ŏI�X�V����	�E�B���h�E�Y�Ǘ�������App.cpp�ɕ���
//��������������������������������������������������������������������������������

#ifndef		_GAME_MAIN_H_
#define		_GAME_MAIN_H_

#include	"MainLoop.h"
#include	<Windows.h>
#include	<string>
using namespace std;

//uses
class CDirectDraw;
class CDirectDrawSurface;
class CDirectInput;
class CDirectInputDevice;

class CGameMain
{
protected:
	HWND		hWnd;			//�E�B���h�E�n���h��
	int			Width;			//����
	int			Height;			//����
	int			MaxAngle;		//�p�x��

	CMainLoop	MainLoop;		//���[�v�Ǘ�
	CDirectDraw	*lpDirectDraw;	//DirectDraw
	CDirectDrawSurface			//DirectDrawSurface
				*lpPrimary,
				*lpBack,
				*lpSrc,
				*lpDst;

	double		*SinTable;		//�T�C���e�[�u��

	CGameMain();				//�R���X�g���N�^

	void	CreateSinTable();	//�T�C���e�[�u���쐬
public:
	static CGameMain* GetInstange(){ static CGameMain gm; return &gm; }
	~CGameMain();				//�f�X�g���N�^

	bool	Init();				//�Q�[��������
	void	UnInit();			//�Q�[���I������

	void	Loop();				//���C�����[�v
	void	Quit(string);		//�I������

	double	Sin(int);			//�T�C��
	double	Cos(int);			//�R�T�C��
	int		GetMaxAngle(){ return MaxAngle; };

	void	OnKeyDonw(UINT);
	void	OnKeyPress(char);

	void draw_house( const POINT& );
};

#endif
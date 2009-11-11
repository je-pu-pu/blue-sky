//��������������������������������������������������������������������������������
//DirectDrawSurface.h
//�ŏI�X�V��	2001/11/27
//�ŏI�X�V����	DrawLine()
//��������������������������������������������������������������������������������

#ifndef		_DIRECT_DRAW_SURFACE_H_
#define		_DIRECT_DRAW_SURFACE_H_

#include	<ddraw.h>
#include	<string>
#include	<vector>
using namespace std;

class CDirectDraw;

//DirectDrawSurface�N���X
class CDirectDrawSurface
{
protected:
	CDirectDraw				*lpDirectDraw;	//DirectDraw
	LPDIRECTDRAWSURFACE7	lpSurface;		//�I�t�X�N���[���T�[�t�F�C�X
	DDSURFACEDESC2			SurfaceDesc;	//���ڕ`��p
	string					FileName;		//�ǂݍ��񂾃t�@�C���̖��O

	int						Width;			//����
	int						Height;			//����

	bool					Locked;			//���b�N�t���O

	void	ScanLine(RGBQUAD*, int, int, RGBQUAD);
	void	ScanLine(RGBQUAD*, int, int, RGBQUAD, RGBQUAD);
	void	RGB_Swap(DWORD&);
public:
	CDirectDrawSurface();					//�R���X�g���N�^
	~CDirectDrawSurface();					//�f�X�g���N�^

	//�T�[�t�F�C�X����
	bool	Create(CDirectDraw*, DWORD, DWORD, DWORD=0);
	bool	CreateOnVideoMemory(CDirectDraw*, DWORD, DWORD);
	bool	CreateOnSystemMemory(CDirectDraw*, DWORD, DWORD);
	//Bitmap�ǂݍ���
	bool	Load(string);
	//���X�g�`�F�b�N�C����
	bool	LostCheck();

	//�摜�]��
	void	Clipping(int&, int&, CDirectDrawSurface*, RECT&);
	void	Blt(int, int, int, int,	CDirectDrawSurface*, int, int, bool);
	void	Blt(int, int, CDirectDrawSurface*, RECT, bool);
	void	Blt(RECT, CDirectDrawSurface*, RECT, bool);
	void	DirectBlt(int, int, int, int, CDirectDrawSurface*, int, int, bool);
	void	DirectBlt(int, int, CDirectDrawSurface*, RECT, bool);
	void	DirectBlt(RECT, CDirectDrawSurface*, RECT, bool);
	void	FillRect(RECT, DWORD);
	void	Clear(DWORD);
	void	TextOut(int, int, const char*);
	void	Line(POINT, POINT);
	//���ڕ`��
	void	Lock();
	void	UnLock();
	void	DrawLine(int, int, int, int, RGBQUAD);
	void	DrawLine(double, double, double, double, RGBQUAD);

	void	DrawLineHumanTouch( double, double, double, double, RGBQUAD );

	void	DrawPolygon(POINT*, RGBQUAD); 
	void	DrawPolygon(POINT*, RGBQUAD*);
	void	DrawPolygonHumanTouch( POINT*, RGBQUAD );

	void	DrawCircle(int, int, int, RGBQUAD, bool);
	void	DrawCircle( double, double, double, RGBQUAD, bool );

	void	DrawRing(int, int, int, int, RGBQUAD, bool);

	void	Blur(RECT, vector<POINT>&);
	void	Blur(CDirectDrawSurface*, RECT, vector<POINT>&);

	bool	LineClipping(int, int, int, int);

	//Get
	LPDIRECTDRAWSURFACE7& GetSurface(){ return lpSurface; }
	DDSURFACEDESC2* GetSurfaceDesc(){ return &SurfaceDesc; }
	int GetWidth(){ return Width; }
	int GetHeight(){ return Height; }
	bool GetLocked(){ return Locked; }
};

//DirectDraw�v���C�}���T�[�t�F�C�X�N���X
class CDirectDrawPrimary : public CDirectDrawSurface
{
public:
	void SetWidth(int w){ Width = w; }
	void SetHeight(int h){ Height = h; }
};
//DirectDraw�o�b�N�o�b�t�@�N���X
//�f�X�g���N�^�ŃT�[�t�F�X��������Ȃ�
class CDirectDrawBackBuffer : public CDirectDrawPrimary
{
public:
	~CDirectDrawBackBuffer(){ lpSurface = NULL; }
};

extern bool g_solid;
extern bool g_line;

extern double g_power;
extern double g_power_min;
extern double g_power_max;
extern double g_power_plus;
extern double g_power_rest;
extern double g_power_plus_reset;

extern double g_direction_fix_default;
extern double g_direction_fix_acceleration;
extern double g_direction_random;

#endif

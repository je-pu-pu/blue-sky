//��������������������������������������������������������������������������������
//DirectDraw.cpp
//�ŏI�X�V��	2001/9/4
//��������������������������������������������������������������������������������

#include	"DirectDraw.h"
#include	"DirectDrawSurface.h"
#include	"Util.h"

//���R���X�g���N�^
CDirectDraw::CDirectDraw()
{
	lpDirectDraw = NULL;
	lpPrimary = NULL;
	lpBack = NULL;
	lpPalette = NULL;
}

//���f�X�g���N�^
CDirectDraw::~CDirectDraw()
{
	UnInit();
}

//��DirectDraw�����ݒ�
bool CDirectDraw::Init(	HWND hwnd,	//�E�B���h�E�n���h��
						DWORD w,	//����
						DWORD h,	//����
						DWORD bpp)	//�F��
{
	//�v���p�e�B�[���R�s�[
	hWnd = hwnd;
	Width = w;
	Height = h;
	Bpp = bpp;

	HRESULT	hr;
	//DirectDraw�I�u�W�F�N�g����
	hr = DirectDrawCreateEx(
						NULL,
						(void **) &lpDirectDraw,
						IID_IDirectDraw7,
						NULL);
	if(hr != DD_OK){
		return false;
	}
	//������ݒ�
	hr = lpDirectDraw->SetCooperativeLevel(
						hWnd,
						DDSCL_FULLSCREEN |
						DDSCL_EXCLUSIVE |
						DDSCL_ALLOWREBOOT);
	if(hr != DD_OK){
		return false;
	}
	//��ʃ��[�h�̐ݒ�
	hr = lpDirectDraw->SetDisplayMode(
						Width,			//����
						Height,			//����
						Bpp,			//�F��
						0, 0);
	if(hr != DD_OK){
		return false;
	}
	//�v���C�}���T�[�t�F�X����
	if(! CreatePrimary()){
		return false;
	}

	if(Bpp == 8){
		//�p���b�g����
		if(! CreatePalette()){
			return false;
		}
	}
	else if(Bpp == 16){
		//�s�N�Z���t�H�[�}�b�g�𒲂ׂ�
		//GetPixelFormat();
		//DDPIXELFORMAT	ddpf;
		//ddpf.dwSize = sizeof(ddpf);
		//lpPrimary->GetPixelFormat(&ddpf);
	}

	//����
	return	true;
}

//���v���C�}���T�[�t�F�C�X���쐬
bool CDirectDraw::CreatePrimary()
{
	DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};

	ddsd.dwFlags =			DDSD_CAPS |
							DDSD_BACKBUFFERCOUNT;

	ddsd.ddsCaps.dwCaps =	DDSCAPS_PRIMARYSURFACE |
							DDSCAPS_FLIP |
							DDSCAPS_COMPLEX;
	//�o�b�N�o�b�t�@�̐�
	ddsd.dwBackBufferCount = 1;

	lpPrimary = new CDirectDrawPrimary[1];
	lpBack = new CDirectDrawBackBuffer[1];

	lpPrimary->SetWidth(Width);
	lpPrimary->SetHeight(Height);
	lpBack->SetWidth(Width);
	lpBack->SetHeight(Height);

	LPDIRECTDRAWSURFACE7 &primary = lpPrimary->GetSurface();
	LPDIRECTDRAWSURFACE7 &backbuffer = lpBack->GetSurface();

	HRESULT hr;
	//�v���C�}���T�[�t�F�X����
	hr = lpDirectDraw->CreateSurface(&ddsd, &primary, NULL);
	if(hr != DD_OK){
		return false;
	}
	//�o�b�N�o�b�t�@�̎擾
	DDSCAPS2 ddsc = {0};
	ddsc.dwCaps = DDSCAPS_BACKBUFFER;
	hr = primary->GetAttachedSurface(&ddsc, &backbuffer);
	if(hr != DD_OK){
		return false;
	}

	//����
	return true;
}

//���p���b�g����
bool CDirectDraw::CreatePalette()
{
	//�p���b�g�p��256�F�̒�`
	PALETTEENTRY	pe[256];
	for(int i = 0; i < 256; i++){
		pe[i].peRed		= i * 20 % 256;
		pe[i].peGreen	= i * 20 % 256;
		pe[i].peBlue	= i * 20 % 256;
		pe[i].peFlags	= PC_RESERVED;
	}

	HRESULT hr;
	//DirectDraw�p���b�g�̐���
	hr = lpDirectDraw->CreatePalette(	DDPCAPS_8BIT |
								DDPCAPS_ALLOW256,
								pe, &lpPalette, NULL);
	if(hr != DD_OK){
		return false;
	}
	//�p���b�g���v���C�}���T�[�t�F�X�ɐݒ�
	hr = lpPrimary->GetSurface()->SetPalette(lpPalette);
	if(hr != DD_OK){
		return false;
	}

	//����
	return true;
}

//���I������
//�J������Ȃ������I�u�W�F�N�g�̐����Ԃ�
int CDirectDraw::UnInit()
{
	int ret = 0;

	if(lpDirectDraw != NULL){
		RELEASE(lpPalette);
		Free(lpPrimary);		
		ret = lpDirectDraw->Release();	
		lpDirectDraw = NULL;
	}
	Free(lpBack);

	return ret;
}

//���t���b�v
void CDirectDraw::Flip(){
	lpPrimary->GetSurface()->Flip(NULL, DDFLIP_WAIT);
}

//���v���C�}���T�[�t�F�X�̉摜���r�b�g�}�b�v�ɏo�͂���
bool CDirectDraw::Capture(char *FileName)
{
	HANDLE		hFile;
	DWORD		dwReturn;

	BITMAPFILEHEADER		bfh;
	BITMAPINFOHEADER		bih;
	RGBQUAD					rgb[256];
	PALETTEENTRY			pe[256];

	int		nWidth;
	int		nHeight;
	int		nPitch;
	BYTE	*lpImage;

	//	�p���b�g�擾
	lpPalette->GetEntries( 0, 0, 256, pe );


	//	�T�[�t�F�X���擾�����b�N
	DDSURFACEDESC2		ddsd;
	ddsd.dwSize	=	sizeof( DDSURFACEDESC2 );
	lpPrimary->GetSurface()->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL );

	//	�T�[�t�F�X���
	nWidth	=	ddsd.dwWidth;
	nHeight	=	ddsd.dwHeight;
	nPitch	=	ddsd.lPitch;
	lpImage	=	( BYTE * )ddsd.lpSurface;



	//	�r�b�g�}�b�v�f�[�^�܂ł̃I�t�Z�b�g�ƃf�[�^�T�C�Y
	DWORD	dwOffset	=	sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER )	+ ( sizeof( RGBQUAD ) * 256 );
	DWORD	dwSize		=	dwOffset + ( nWidth*nHeight*sizeof( BYTE ) );
			

	//	BITMAPFILEHEADER�ݒ�
	bfh.bfType		=	0x4d42;			//	BITMAP�̎��ʎq�i�@0x4D42�@�j
	bfh.bfSize		=	dwSize;			//	�ۑ�����r�b�g�}�b�v�̑��T�C�Y
	bfh.bfReserved1	=	0;				//	�K���O
	bfh.bfReserved2	=	0;				//	�K���O
	bfh.bfOffBits	=	dwOffset;		//	�t�@�C���̐擪����C���[�W�f�[�^�܂ł̃o�C�g��


	//	BITMAPINFOHEADER�ݒ�
	bih.biSize				=	sizeof( BITMAPINFOHEADER );
	bih.biWidth				=	nWidth;
	bih.biHeight			=	nHeight;	//	�{�g���A�b�v�`��
	bih.biPlanes			=	1;
	bih.biBitCount			=	8;			//	�Q�T�U�F
	bih.biCompression		=	BI_RGB;
	bih.biSizeImage			=	0;
	bih.biXPelsPerMeter		=	0;
	bih.biYPelsPerMeter		=	0;
	bih.biClrUsed			=	256;		//	�p���b�g�f�[�^�̌��i�@�O�łQ�T�U�G���g���S�Ă�\�����Ƃ��ł���@�j
	bih.biClrImportant		=	0;


	//	�Q�T�U�F�p���b�g�ϊ�
	for( int i = 0 ; i < 256 ; i++ )
	{
		rgb[i].rgbRed		=	pe[i].peRed;
		rgb[i].rgbGreen		=	pe[i].peGreen;
		rgb[i].rgbBlue		=	pe[i].peBlue;
		rgb[i].rgbReserved	=	0;
	}



	//	�r�b�g�}�b�v��������
	hFile	=	CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		lpPrimary->GetSurface()->Unlock( NULL );
		return	FALSE;
	}

	WriteFile( hFile, &bfh, sizeof( BITMAPFILEHEADER ), &dwReturn, NULL );
	WriteFile( hFile, &bih, sizeof( BITMAPINFOHEADER ), &dwReturn, NULL );
	WriteFile( hFile, rgb, sizeof( RGBQUAD ) * 256, &dwReturn, NULL );

	//	�|�C���^���T�[�t�F�X�̏I�[��
	//	  �P�񂸂����珇�ɃR�s�[���āA�{�g���A�b�v�`���ŕۑ�
	lpImage	+=	nPitch*nHeight;
	for( int y = 0 ; y < nHeight ; y++ )
	{
		lpImage	-=	nPitch;
		WriteFile( hFile, lpImage, nWidth, &dwReturn, NULL );
	}

	CloseHandle( hFile );

	lpPrimary->GetSurface()->Unlock( NULL );

	return	TRUE;
}

//�����X�g�`�F�b�N�E����
bool CDirectDraw::LostCheck()
{
	if(lpPrimary->GetSurface()->IsLost() == DDERR_SURFACELOST){
		HRESULT hr = lpDirectDraw->RestoreAllSurfaces();
		if(FAILED(hr))	return false;
	}
	return true;
}
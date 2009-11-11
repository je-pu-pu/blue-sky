//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//DirectDraw.cpp
//最終更新日	2001/9/4
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#include	"DirectDraw.h"
#include	"DirectDrawSurface.h"
#include	"Util.h"

//■コンストラクタ
CDirectDraw::CDirectDraw()
{
	lpDirectDraw = NULL;
	lpPrimary = NULL;
	lpBack = NULL;
	lpPalette = NULL;
}

//■デストラクタ
CDirectDraw::~CDirectDraw()
{
	UnInit();
}

//■DirectDraw初期設定
bool CDirectDraw::Init(	HWND hwnd,	//ウィンドウハンドル
						DWORD w,	//横幅
						DWORD h,	//高さ
						DWORD bpp)	//色数
{
	//プロパティーをコピー
	hWnd = hwnd;
	Width = w;
	Height = h;
	Bpp = bpp;

	HRESULT	hr;
	//DirectDrawオブジェクト生成
	hr = DirectDrawCreateEx(
						NULL,
						(void **) &lpDirectDraw,
						IID_IDirectDraw7,
						NULL);
	if(hr != DD_OK){
		return false;
	}
	//動作環境設定
	hr = lpDirectDraw->SetCooperativeLevel(
						hWnd,
						DDSCL_FULLSCREEN |
						DDSCL_EXCLUSIVE |
						DDSCL_ALLOWREBOOT);
	if(hr != DD_OK){
		return false;
	}
	//画面モードの設定
	hr = lpDirectDraw->SetDisplayMode(
						Width,			//横幅
						Height,			//高さ
						Bpp,			//色数
						0, 0);
	if(hr != DD_OK){
		return false;
	}
	//プライマリサーフェス生成
	if(! CreatePrimary()){
		return false;
	}

	if(Bpp == 8){
		//パレット生成
		if(! CreatePalette()){
			return false;
		}
	}
	else if(Bpp == 16){
		//ピクセルフォーマットを調べる
		//GetPixelFormat();
		//DDPIXELFORMAT	ddpf;
		//ddpf.dwSize = sizeof(ddpf);
		//lpPrimary->GetPixelFormat(&ddpf);
	}

	//成功
	return	true;
}

//□プライマリサーフェイスを作成
bool CDirectDraw::CreatePrimary()
{
	DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};

	ddsd.dwFlags =			DDSD_CAPS |
							DDSD_BACKBUFFERCOUNT;

	ddsd.ddsCaps.dwCaps =	DDSCAPS_PRIMARYSURFACE |
							DDSCAPS_FLIP |
							DDSCAPS_COMPLEX;
	//バックバッファの数
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
	//プライマリサーフェス生成
	hr = lpDirectDraw->CreateSurface(&ddsd, &primary, NULL);
	if(hr != DD_OK){
		return false;
	}
	//バックバッファの取得
	DDSCAPS2 ddsc = {0};
	ddsc.dwCaps = DDSCAPS_BACKBUFFER;
	hr = primary->GetAttachedSurface(&ddsc, &backbuffer);
	if(hr != DD_OK){
		return false;
	}

	//成功
	return true;
}

//□パレット生成
bool CDirectDraw::CreatePalette()
{
	//パレット用の256色の定義
	PALETTEENTRY	pe[256];
	for(int i = 0; i < 256; i++){
		pe[i].peRed		= i * 20 % 256;
		pe[i].peGreen	= i * 20 % 256;
		pe[i].peBlue	= i * 20 % 256;
		pe[i].peFlags	= PC_RESERVED;
	}

	HRESULT hr;
	//DirectDrawパレットの生成
	hr = lpDirectDraw->CreatePalette(	DDPCAPS_8BIT |
								DDPCAPS_ALLOW256,
								pe, &lpPalette, NULL);
	if(hr != DD_OK){
		return false;
	}
	//パレットをプライマリサーフェスに設定
	hr = lpPrimary->GetSurface()->SetPalette(lpPalette);
	if(hr != DD_OK){
		return false;
	}

	//成功
	return true;
}

//■終了処理
//開放されなかったオブジェクトの数が返る
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

//■フリップ
void CDirectDraw::Flip(){
	lpPrimary->GetSurface()->Flip(NULL, DDFLIP_WAIT);
}

//■プライマリサーフェスの画像をビットマップに出力する
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

	//	パレット取得
	lpPalette->GetEntries( 0, 0, 256, pe );


	//	サーフェス情報取得＆ロック
	DDSURFACEDESC2		ddsd;
	ddsd.dwSize	=	sizeof( DDSURFACEDESC2 );
	lpPrimary->GetSurface()->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL );

	//	サーフェス情報
	nWidth	=	ddsd.dwWidth;
	nHeight	=	ddsd.dwHeight;
	nPitch	=	ddsd.lPitch;
	lpImage	=	( BYTE * )ddsd.lpSurface;



	//	ビットマップデータまでのオフセットとデータサイズ
	DWORD	dwOffset	=	sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER )	+ ( sizeof( RGBQUAD ) * 256 );
	DWORD	dwSize		=	dwOffset + ( nWidth*nHeight*sizeof( BYTE ) );
			

	//	BITMAPFILEHEADER設定
	bfh.bfType		=	0x4d42;			//	BITMAPの識別子（　0x4D42　）
	bfh.bfSize		=	dwSize;			//	保存するビットマップの総サイズ
	bfh.bfReserved1	=	0;				//	必ず０
	bfh.bfReserved2	=	0;				//	必ず０
	bfh.bfOffBits	=	dwOffset;		//	ファイルの先頭からイメージデータまでのバイト数


	//	BITMAPINFOHEADER設定
	bih.biSize				=	sizeof( BITMAPINFOHEADER );
	bih.biWidth				=	nWidth;
	bih.biHeight			=	nHeight;	//	ボトムアップ形式
	bih.biPlanes			=	1;
	bih.biBitCount			=	8;			//	２５６色
	bih.biCompression		=	BI_RGB;
	bih.biSizeImage			=	0;
	bih.biXPelsPerMeter		=	0;
	bih.biYPelsPerMeter		=	0;
	bih.biClrUsed			=	256;		//	パレットデータの個数（　０で２５６エントリ全てを表すこともできる　）
	bih.biClrImportant		=	0;


	//	２５６色パレット変換
	for( int i = 0 ; i < 256 ; i++ )
	{
		rgb[i].rgbRed		=	pe[i].peRed;
		rgb[i].rgbGreen		=	pe[i].peGreen;
		rgb[i].rgbBlue		=	pe[i].peBlue;
		rgb[i].rgbReserved	=	0;
	}



	//	ビットマップ書き込み
	hFile	=	CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		lpPrimary->GetSurface()->Unlock( NULL );
		return	FALSE;
	}

	WriteFile( hFile, &bfh, sizeof( BITMAPFILEHEADER ), &dwReturn, NULL );
	WriteFile( hFile, &bih, sizeof( BITMAPINFOHEADER ), &dwReturn, NULL );
	WriteFile( hFile, rgb, sizeof( RGBQUAD ) * 256, &dwReturn, NULL );

	//	ポインタをサーフェスの終端に
	//	  １列ずつ下から順にコピーして、ボトムアップ形式で保存
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

//■ロストチェック・復元
bool CDirectDraw::LostCheck()
{
	if(lpPrimary->GetSurface()->IsLost() == DDERR_SURFACELOST){
		HRESULT hr = lpDirectDraw->RestoreAllSurfaces();
		if(FAILED(hr))	return false;
	}
	return true;
}
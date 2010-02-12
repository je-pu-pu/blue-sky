//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//DirectDrawSurface.cpp
//最終更新日	2001/9/19
//最終更新部分	LostCheck(), RGB_Swap()
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#include	"DirectDrawSurface.h"
#include	"DirectDraw.h"

#include	"GameMain.h"

#define _USE_MATH_DEFINES
#include <math.h>

//Debug用
#include	"Util.h"
#include	<stdio.h>

//■コンストラクタ
CDirectDrawSurface::CDirectDrawSurface()
{
	lpSurface = NULL;
	Width = 0;
	Height = 0;
	Locked = false;
}

//■デストラクタ
CDirectDrawSurface::~CDirectDrawSurface()
{
	RELEASE(lpSurface);
}

//■オフスクリーンサーフェス生成
bool CDirectDrawSurface::Create(
							CDirectDraw *lpdd,	//DirectDraw
							DWORD w,			//横幅
							DWORD h,			//高さ
							DWORD flag)			//フラグ
{
	//1度開放
	RELEASE(lpSurface);
	//パラメータのコピー
	lpDirectDraw = lpdd;
	Width = w;
	Height = h;
	//生成するサーフェスの設定
	DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = Width;
	ddsd.dwHeight = Height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | flag;
	//サーフェス生成
	HRESULT hr;
	hr = lpDirectDraw->GetObject()->CreateSurface(&ddsd, &lpSurface, NULL);
	if(hr != DD_OK){
		return false;
	}
	//透明色の設定
	DDCOLORKEY ddck = {0, 0};
	lpSurface->SetColorKey(DDCKEY_SRCBLT, &ddck);
		
	//成功
	return true;
}
bool CDirectDrawSurface::CreateOnVideoMemory(CDirectDraw *lpdd, DWORD w, DWORD h)
{
	return Create(lpdd, w, h, DDSCAPS_VIDEOMEMORY);
}
bool CDirectDrawSurface::CreateOnSystemMemory(CDirectDraw *lpdd, DWORD w, DWORD h)
{
	return Create(lpdd, w, h, DDSCAPS_SYSTEMMEMORY);
}

//■サーフェイスにBitmapを読み込む
bool CDirectDrawSurface::Load(string file_name)	//ファイル名
{
	//サーフェスが生成されているか調べる
	if(lpSurface == NULL){
		return false;
	}
	//ファイル名をコピー
	FileName = file_name;

	HWND	hwnd = lpDirectDraw->GetWindowHandle();
	HDC		hdc;		//hwndから取得するHDC
	HDC		hdc2;		//新規作成用HDC
	HDC		hdc3;		//サーフェイスから取得するHDC
	HBITMAP	hBitmap;

	//ビットマップのロード
	hBitmap = (HBITMAP)LoadImage(NULL, FileName.c_str(), IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(hBitmap == NULL){
		return false;
	}
	//デバイスコンテキスト作成，設定
	hdc = GetDC(hwnd);
	hdc2 = CreateCompatibleDC(hdc);
	ReleaseDC(hwnd, hdc);
	SelectObject(hdc2, hBitmap);
	//サーフェイスに画像転送
	lpSurface->GetDC(&hdc3);
	BitBlt(hdc3, 0, 0, Width, Height, hdc2, 0, 0, SRCCOPY);
	lpSurface->ReleaseDC(hdc3);
	//解放
	DeleteObject(hBitmap);
	DeleteDC(hdc2);
	
	//成功
	return true;
}

//■ロストのチェック，復元
//サーフェイスがロストして復元に失敗した場合falseが返る
bool CDirectDrawSurface::LostCheck()
{
	//サーフェスが生成されているか調べる
	if(lpSurface == NULL){
		return false;
	}
	//ロストチェック
	if(lpSurface->IsLost() == DDERR_SURFACELOST){
		HRESULT hr;
		//復元
		hr = lpSurface->Restore();
		if(FAILED(hr)){
			return false;
		}
		//Bitmap再読み込み
		if(FileName != ""){
			return Load(FileName);
		}
	}
	return true;
}

//■クリッピング
void CDirectDrawSurface::Clipping(int &dx, int &dy,
								  CDirectDrawSurface *lps, RECT &rcSrc)
{
	//Src
	if(rcSrc.left < 0){
		dx -= rcSrc.left;
		rcSrc.left = 0;
	}
	if(rcSrc.top < 0){
		dy -= rcSrc.top;
		rcSrc.top = 0;
	}
	if(rcSrc.right > lps->GetWidth()){
		rcSrc.right = lps->GetWidth();
	}
	if(rcSrc.bottom > lps->GetHeight()){
		rcSrc.bottom = lps->GetHeight();
	}

	//Dst
	//left
	if(dx < 0){
		rcSrc.left -= dx;
		dx = 0;
	}
	//right
	if(dx + rcSrc.right - rcSrc.left > Width){
		rcSrc.right -= dx + rcSrc.right - rcSrc.left - Width;
	}
	//top
	if(dy < 0){
		rcSrc.top -= dy;
		dy = 0;
	}
	//bottom
	if(dy + rcSrc.bottom - rcSrc.top > Height){
		rcSrc.bottom -= dy + rcSrc.bottom - rcSrc.top - Height;
	}
}

//■クリッピング付き画像転送
void CDirectDrawSurface::Blt(
	int					dx,			//転送先座標
	int					dy,
	CDirectDrawSurface	*lpSrc,		//転送元サーフェイス
	RECT				rcSrc,		//転送元座標
	bool				ColorKey)	//透過
{
	//クリッピング
	Clipping(dx, dy, lpSrc, rcSrc);
	//透過
	DWORD flag = DDBLTFAST_WAIT;
	if(ColorKey){
		flag |= DDBLTFAST_SRCCOLORKEY;
	}
	//BltFast
	lpSurface->BltFast(dx, dy, lpSrc->GetSurface(), &rcSrc, flag);
}
void CDirectDrawSurface::Blt(int dx, int dy, int w, int h,
							 CDirectDrawSurface	*lpSrc, int sx, int sy, bool ColorKey)
{
	RECT rcSrc = {sx, sy, sx + w, sy + h};
	Blt(dx, dy, lpSrc, rcSrc, ColorKey);
}

//■拡大縮小可能画像転送
void CDirectDrawSurface::Blt(RECT drc, CDirectDrawSurface *lps, RECT src, bool color_key)
{
	DWORD flag = DDBLT_WAIT;
	if(color_key)	flag |= DDBLT_KEYSRC;

	//Blt
	lpSurface->Blt(&drc, lps->GetSurface(), &src, flag, NULL);
}

//■Lock時画像転送
void CDirectDrawSurface::DirectBlt(
	int					dx,			//転送先座標
	int					dy,
	CDirectDrawSurface	*lpSrc,		//転送元サーフェイス
	RECT				rcSrc,		//転送元座標
	bool				color_key)	//透過
{
	if(lpSrc == NULL)				return;
	if(! Locked)					return;
	if(! lpSrc->GetLocked())		return;

	Clipping(dx, dy, lpSrc, rcSrc);	//クリッピング

	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;
	RGBQUAD *sp = (RGBQUAD*)lpSrc->GetSurfaceDesc()->lpSurface;
	int dw = SurfaceDesc.lPitch / 4;
	int sw = lpSrc->GetSurfaceDesc()->lPitch / 4;
	int w = rcSrc.right - rcSrc.left;

	dp += dy * dw + dx;
	sp += rcSrc.top * dw + rcSrc.left;

	if(color_key){
	//透過あり
		for(int sy = rcSrc.top; sy < rcSrc.bottom; sy++){
			for(int sx = rcSrc.left; sx < rcSrc.right; sx++){
				if( (sp->rgbRed > 0) && (sp->rgbGreen > 0) && (sp->rgbBlue) ){
					*dp = *sp;
				}
				dp++;
				sp++;
			}
			dp += dw - w;
			sp += sw - w;
		}
	} else {
	//透過なし
		for(int sy = rcSrc.top; sy < rcSrc.bottom; sy++){
			for(int sx = rcSrc.left; sx < rcSrc.right; sx++){
				*dp = *sp;
				dp++;
				sp++;
			}
			dp += dw - w;
			sp += sw - w;
		}
	}
}
void CDirectDrawSurface::DirectBlt(	int dx, int dy, int w, int h,
									CDirectDrawSurface	*lpSrc, int sx, int sy, bool ColorKey)
{
	RECT rcSrc = {sx, sy, sx + w, sy + h};
	DirectBlt(dx, dy, lpSrc, rcSrc, ColorKey);
}
//■拡大縮小可能画像転送
void CDirectDrawSurface::DirectBlt(RECT drc, CDirectDrawSurface *lps, RECT src, bool color_key)
{
	//ロックされているか調べる
	if( ! Locked )				return;
	if( ! lps->GetLocked() )	return;

	DDSURFACEDESC2 *sddsd = lps->GetSurfaceDesc();

	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;
	RGBQUAD *sp = (RGBQUAD*)sddsd->lpSurface;

	int dsw = SurfaceDesc.lPitch / 4;
	int ssw = sddsd->lPitch / 4;

	int sw = src.right - src.left;	//横幅
	int dw = drc.right - drc.left;
	int sh = src.bottom - src.top;	//高さ
	int dh = drc.bottom - drc.top;
	
	if(dw <= 0)		return;
	if(dh <= 0)		return;

	double sx = src.left;			//転送元座標
	double sy = src.top;
	double asx = double(sw) / dw;	//増加量
	double asy = double(sh) / dh;

	if(drc.left < 0){				//転送先が左にはみ出していた場合
		sx += asx * -drc.left;
		dw += drc.left;
		drc.left = 0;
	}
	if(drc.top < 0){				//転送先が上にはみ出していた場合
		sy += asy * -drc.top;
		dh += drc.top;
		drc.top = 0;
	}
									 //転送先が右にはみ出していた場合
	int w = SurfaceDesc.dwWidth;
	if(drc.right > w){
		dw -= drc.right - SurfaceDesc.dwWidth;
		drc.right = SurfaceDesc.dwWidth;
	}
									//転送先が下にはみ出していた場合
	int h = SurfaceDesc.dwHeight;
	if(drc.bottom > h){
		dh -= drc.bottom - SurfaceDesc.dwHeight;
		drc.bottom = SurfaceDesc.dwHeight;
	}

	dp += drc.top * dsw + drc.left;
	double ssx = sx;				//描画開始x座標

	if(color_key){

	} else {
	//透過なし
		for(int y = drc.top; y < drc.bottom; y++){
			for(int x = drc.left; x < drc.right; x++){
				int isx = int(sx);
				int isy = int(sy);
				*dp = sp[ isy * ssw + isx ];
				dp++;
				sx += asx;
			}
			dp += dsw - dw;
			sy += asy;
			sx = ssx;
		}
	}
}

//■指定した色で塗りつぶし
void CDirectDrawSurface::FillRect(RECT rc, DWORD cl)
{
	DDBLTFX ddfx = {sizeof(DDBLTFX)};
	RGB_Swap(cl);
	ddfx.dwFillColor = cl;
	lpSurface->Blt(&rc, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddfx);
}

//■指定した色でクリア
void CDirectDrawSurface::Clear(DWORD cl)
{
	DDBLTFX ddfx = {sizeof(DDBLTFX)};
	RGB_Swap(cl);
	ddfx.dwFillColor = cl;
	lpSurface->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddfx);
}

//■文字表示
void CDirectDrawSurface::TextOut(int x, int y, const char *str)
{
	HDC hdc;
	lpSurface->GetDC(&hdc);

	SetTextColor(hdc, RGB(255,255,255));
	SetBkMode(hdc, TRANSPARENT);
	::TextOut(hdc, x, y, str, lstrlen(str));

	lpSurface->ReleaseDC(hdc);
}

//■ロック
void CDirectDrawSurface::Lock()
{
	if(! Locked){
		SurfaceDesc.dwSize = sizeof(DDSURFACEDESC2);
		lpSurface->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
		Locked = true;
	}
}

//■アンロック
void CDirectDrawSurface::UnLock()
{
	if(Locked){
		lpSurface->Unlock(NULL);
		Locked = false;
	}
}

//■線のクリッピング
//描画する必要がなければfalseを返す
bool CDirectDrawSurface::LineClipping(
					int x1, int y1,			//始点
					int x2, int y2)			//終点
{
	if( (x1 < 0) && (x2 < 0) )				return false;
	if( (x1 >= Width) && (x2 >= Width) )	return false;
	if( (y1 < 0) && (y2 < 0) )				return false;
	if( (y1 >= Height) && (y2 >= Height) )	return false;

	return true;
}

//■線描画
//32ビット専用
//ロックしてから使用する必要がある
void CDirectDrawSurface::DrawLine(
					int x1, int y1,			//始点
					int x2, int y2,			//終点
					RGBQUAD rgbq)			//色
{
	//ロックされていなければ失敗
	if(! Locked)	return;
	//描画する必要がなければ実行しない
	if(! LineClipping(x1, y1, x2, y2))	return;

	int	dx, dy;		//x, yの相対量
	int absx, absy;	//x, yの絶対量（距離）
	int ax, ay;		//x, yへの増加量(1ループ）
	int count;		//カウンタ
	int i;
	int w = SurfaceDesc.lPitch / 4;

	//直接描画
	RGBQUAD *p = (RGBQUAD*)SurfaceDesc.lpSurface;

	//線分情報初期化
	dx = x2 - x1;
	dy = y2 - y1;
	if(dx >= 0){
		absx = dx;
		ax = 1;
	}
	else{
		absx = -dx;
		ax = -1;
	}
	if(dy >= 0){
		absy = dy;
		ay = 1;
	}
	else{
		absy = -dy;
		ay = -1;
	}
	
	//線分の描画
	if(absx >= absy){
		count = absx / 2;
		for(i = 0; i < absx; i++){
			if( (x1 >= 0) && (x1 < Width) && (y1 >= 0) && (y1 < Height) ){
				p[x1 + y1 * w] = rgbq;
			}
			x1 += ax;
			count += absy;
			if(count >= absx){
				y1 += ay;
				count -= absx;
			}
		}
	}
	else{
		count = absy / 2;
		for(i = 0; i <= absy; i++){
			if( (x1 >= 0) && (x1 < Width) && (y1 >= 0) && (y1 < Height) ){
				p[x1 + y1 * w] = rgbq;
			}
			y1 += ay;
			count += absx;
			if(count >= absy){
				x1 += ax;
				count -= absy;
			}
		}
	}
}

//■アンチエイリアスライン描画
//32ビット専用
//ロックしてから使用する必要がある
void CDirectDrawSurface::DrawLine(
					double x1, double y1,	//始点
					double x2, double y2,	//終点
					RGBQUAD cl)				//色
{
	//ロックされていなければ失敗
	if(! Locked)	return;
	
	// 描画する必要がなければ実行しない
	if ( ! LineClipping( static_cast< int >( x1 ), static_cast< int >( y1 ), static_cast< int >( x2 ), static_cast< int >( y2 ) ) )
	{
		return;
	}

	//直接描画
	RGBQUAD *sp = (RGBQUAD*)SurfaceDesc.lpSurface;
	int w = SurfaceDesc.lPitch / 4;
	//相対量
	double	lx = x2 - x1;
	double	ly = y2 - y1;
	//距離
	double	len = sqrt(lx * lx + ly * ly);
	//角度
	double	c = atan2(ly, lx);
	//移動量
	double	ax = cos(c);
	double	ay = sin(c);

	double	dx = x1;
	double	dy = y1;

	for(int i = 0; i < len; i++, dx += ax, dy += ay){
		int x = int(dx);
		int y = int(dy);

		RGBQUAD *dp;	//転送先ピクセル
		double	tmp;	//比率

		//クリッピング
		if(dx < 0)			continue;
		if(dx >= Width)		continue;
		if(dy < 0)			continue;
		if(dy >= Height)	continue;

		if( (x >= 0) && (x < Width) && (y >= 0) && (y < Height) ){
			//x, y
			dp = sp + y * w + x;
			tmp = (1 - (dx - x)) * (1 - (dy - y)) / 1;
			dp->rgbRed   = static_cast< int >( cl.rgbRed   * tmp + dp->rgbRed   * ( 1.0 - tmp ) );
			dp->rgbGreen = static_cast< int >( cl.rgbGreen * tmp + dp->rgbGreen * ( 1.0 - tmp ) );
			dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * tmp + dp->rgbBlue  * ( 1.0 - tmp ) );
		}
		if( (x + 1 >= 0) && (x + 1 < Width) && (y >= 0) && (y < Height) ){
			//x + 1, y
			dp = sp + y * w + x + 1;
			tmp = (dx - x) * (1 - (dy - y)) / 1;
			dp->rgbRed   = static_cast< int >( cl.rgbRed   * tmp + dp->rgbRed   * ( 1.0 - tmp ) );
			dp->rgbGreen = static_cast< int >( cl.rgbGreen * tmp + dp->rgbGreen * ( 1.0 - tmp ) );
			dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * tmp + dp->rgbBlue  * ( 1.0 - tmp ) );
		}
		if( (x >= 0) && (x < Width) && (y + 1 >= 0) && (y + 1 < Height) ){
			//x, y + 1
			dp = sp + (y + 1) * w + x;
			tmp = (1 - (dx - x)) * (dy - y) / 1;
			dp->rgbRed   = static_cast< int >( cl.rgbRed   * tmp + dp->rgbRed   * ( 1.0 - tmp ) );
			dp->rgbGreen = static_cast< int >( cl.rgbGreen * tmp + dp->rgbGreen * ( 1.0 - tmp ) );
			dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * tmp + dp->rgbBlue  * ( 1.0 - tmp ) );
		}
		if( (x + 1 >= 0) && (x + 1 < Width) && (y + 1 >= 0) && (y + 1 < Height) ){
			//x + 1, y + 1
			dp = sp + (y + 1) * w + (x + 1);
			tmp = (dx - x) * (dy - y) / 1;
			dp->rgbRed   = static_cast< int >( cl.rgbRed   * tmp + dp->rgbRed   * ( 1.0 - tmp ) );
			dp->rgbGreen = static_cast< int >( cl.rgbGreen * tmp + dp->rgbGreen * ( 1.0 - tmp ) );
			dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * tmp + dp->rgbBlue  * ( 1.0 - tmp ) );
		}
	}
}

bool g_solid = false;
bool g_line = false;

double g_power = 1.0;
double g_power_min = 1.0;
double g_power_max = 5.0;
double g_power_plus = 0.1;
double g_power_rest = 0.8;
double g_power_plus_reset = -1.0;

double g_direction_fix_default = 0.0001;
double g_direction_fix_acceleration = 0.01;
double g_direction_random = 0.01;

void CDirectDrawSurface::DrawLineHumanTouch( double x1, double y1, double x2, double y2, RGBQUAD c )
{
	srand( CGameMain::GetInstange()->getMainLoop().GetNowTime() / 100 );

	int r = rand() % 255;

	RGBQUAD black = { 0, r, r, 20 };

	// くっきり
	if ( g_solid )
	{
		c.rgbReserved = 255;
	}

	// 面白みの無い絵に
	if ( g_line )
	{
		return DrawLine( x1, y1, x2, y2, c );
	}

	/*
	// 常に上から下の線を描く
	if ( y1 > y2 )
	{
		std::swap( x1, x2 );
		std::swap( y1, y2 );
	}
	*/

	int division = 200; // 一本の線を描画するために最大何個の円を描画するか
	double interval = 10.0; // 円と円の間隔の最大値 ( pixel )

	double power = 1.2; // 筆圧 ( pixel )
	double power_min = 1.0; // 最低筆圧 ( pixel )
	double power_max = 20.0; // 最大筆圧 ( pixel )
	double power_plus = 0.02; //
	double power_reset = 0.8; // 筆圧リセット時の倍率
	double power_plus_reset = -1.0;

	double direction = atan2( y2 - y1, x2 - x1 ); // 現在の筆の位置から終了点までの正確な方向 ( radian )
	double direction_fix_default = 0.0001; // 筆の方向の補正値の最小値 ( radian )
	double direction_fix = direction_fix_default; // 筆の方向の補正値 ( radian )
	double direction_fix_acceleration = 0.01; // 筆の方向の補正値の加速度 ( radian )
	double direction_random = 0.01; // 筆の方向のランダム性

	int color_plus = -2; // 色の変化

	power = g_power;
	power_min = g_power_min;
	power_max = g_power_max;
	power_plus = g_power_plus;
	power_reset = g_power_rest;
	power_plus_reset = g_power_plus_reset;

	direction_fix_default = g_direction_fix_default;
	direction_fix_acceleration = g_direction_fix_acceleration;
	direction_random = g_direction_random;

	direction_fix = direction_fix_default;


	double a = 0.f; // 筆圧のアレ
	double d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // 現在の筆の方向 ( radian )

	for ( int n = 0; n < division; n++ )
	{
		x1 += cos( d ) * min( power / 2, interval );
		y1 += sin( d ) * min( power / 2, interval );

		if ( rand() % 10 == 0 )
		{
			// DrawCircle( x1, y1, power + 3.0, black, true );
			DrawRing( x1, y1, power + 1.0, power, black, true );
		}
		else
		{
			DrawCircle( x1, y1, power, c, true );
		}
		

		a += 0.01f; // ( ( rand() % 100 / 100.f ) ) * 0.1f;

		power += ( 1.0 + a ) * 0.5 * power_plus;
		// power = a * power_plus;

		if ( power > power_max )
		{
			power = power_max;
			power_plus = -power_plus;
		}
		if ( power < power_min )
		{
			power = power_min;
			power_plus = -power_plus;
		}

		double lx = x2 - x1;
		double ly = y2 - y1;
		double len = sqrt( lx * lx + ly * ly );

		direction = atan2( y2 -y1, x2 - x1 );
		
		while ( direction < 0 )
		{
			direction += ( 2 * M_PI );
		}
		while ( d < 0 )
		{
			d += ( 2 * M_PI );
		}
		while ( direction >= 2 * M_PI )
		{
			direction -= 2 * M_PI;
		}
		while ( d >= 2 * M_PI )
		{
			d -= 2 * M_PI;
		}

		double dd = direction - d;

		while ( dd < 0 )
		{
			dd += ( 2 * M_PI );
		}

		if ( dd == 0 )
		{
			
		}
		else if ( dd <= M_PI )
		{
			d += direction_fix;
		}
		else if ( dd > M_PI )
		{
			d -= direction_fix;
		}
		else
		{
			break;
		}

		if ( len < 1.f )
		{
			break;
		}

		direction_fix += direction_fix_acceleration;

		if ( rand() % 100 < 10 )
		{
			direction_fix = direction_fix_default;
			d += ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random;
			
			power *= power_reset;
			power_plus *= power_plus_reset;

			c.rgbRed   = min( 255, max( 0, int( c.rgbRed )   + color_plus ) );
			c.rgbGreen = min( 255, max( 0, int( c.rgbGreen ) + color_plus ) );
			c.rgbBlue  = min( 255, max( 0, int( c.rgbBlue )  + color_plus ) );
			
			// c.rgbReserved = rand() % 256;
		}

		// c.rgbReserved = static_cast< int >( a * 100 ) % 256;
		

		/*
		if ( d < direction )
		{
			d += direction_fix;

			if ( d > direction ) d = direction;
		}
		if ( d > direction )
		{
			d -= direction_fix;

			if ( d < direction ) d = direction;
		}

		direction_fix += 0.001f;

		if ( len < 5.f )
		{
				direction_fix = 100;
		}
		*/
	}
}

//■単色三角形描画
//32ビット専用
//ロックしてから使用する必要がある
void CDirectDrawSurface::DrawPolygon(POINT *sp,		//POINT [3]
									 RGBQUAD cl)	//色
{
	//ロックされていなければ失敗
	if(! Locked){
		return;
	}

	int w = SurfaceDesc.lPitch / 4;
	int ws = Width - 1,
		hs = Height - 1;

	//並べ替えができるようにコピー
	POINT point[3];
	CopyMemory(point, sp, sizeof(POINT) * 3);

	//point[0].y < point[1].y < point[2].y
	//になるように並び替え
	if(point[0].y > point[1].y)		swap(point[0], point[1]);
	if(point[1].y > point[2].y)		swap(point[1], point[2]);
	if(point[0].y > point[1].y)		swap(point[0], point[1]);

	if(point[0].y == point[1].y){
		if(point[0].x > point[1].x){
			swap(point[0], point[1]);
		}
	}

	//クリッピング
	if(point[2].y < 0)	return;
	if(point[0].y >= hs) return;
	
	if( (point[0].x < 0) && (point[1].x < 0) && (point[2].x < 0) )	return;
	if( (point[0].x >= ws) && (point[1].x >= ws) && (point[2].x >= ws) ) return;

	//傾きを求める
	double mx[3] = {0, 0, 0};
	double smx, emx, smx2, emx2;

	//0..2		一番長い辺
	if(point[2].y - point[0].y != 0){
		mx[0] = (point[2].x - point[0].x) / (double)(point[2].y - point[0].y);
	}
	//0..1		途中までの辺
	if(point[1].y - point[0].y != 0){
		mx[1] = (point[1].x - point[0].x) / (double)(point[1].y - point[0].y);
	}
	//1..2		途中からの辺
	if(point[2].y - point[1].y != 0){
		mx[2] = (point[2].x - point[1].x) / (double)(point[2].y - point[1].y);
	}

	if( (mx[0] > mx[1]) && (point[0].y != point[1].y) ){
		//左向き
		smx = mx[1];
		emx = mx[0];
		smx2 = mx[2];
		emx2 = mx[0];
	}
	else{
		//右向き
		smx = mx[0];
		emx = mx[1];
		smx2 = mx[0];
		emx2 = mx[2];
	}

	//辺の長いほうからScanLineする
	double sx = point[0].x;		//x開始位置
	double ex;					//x終了位置
	int y = point[0].y;			//y初期化
	int dsx, dex;
	
	if(point[0].y == point[1].y){
		ex = point[1].x;
	}
	else{
		ex = sx;
	}

	//部分クリッピング
	if(point[0].y < 0){
		if(point[1].y < 0){
			//2点上にはみ出している
			if( (mx[0] > mx[1]) && (point[0].y != point[1].y) ){
				//右向き
				sx = point[1].x;
				sx += smx2 * -point[1].y;
				ex += emx2 * -point[0].y;
			}
			else{
				ex = point[1].x;
				sx += smx2 * -point[0].y;
				ex += emx2 * -point[1].y;
			}
		}
		else{
			//1点上にはみ出している
			sx += smx * -point[0].y;
			ex += emx * -point[0].y;
		}
		y = 0;
	}

	//描画
	RGBQUAD *p = (RGBQUAD*)SurfaceDesc.lpSurface;
	p += w * y;

	if( (point[0].y == point[1].y) && (point[0].y == point[2].y) ){
		//3点のyが同じ
		//0..1..2の順に並び替える
		if(point[0].x > point[1].x)		swap(point[0].x, point[1].x);
		if(point[1].x > point[2].x)		swap(point[1].x, point[2].x);
		if(point[0].x > point[1].x)		swap(point[0].x, point[1].x);

		dsx = point[0].x;
		dex = point[2].x;

		ScanLine(p, dsx, dex, cl);

		return;
	}

	//y座標クリッピング
	if(point[1].y > hs)		point[1].y = hs;
	if(point[2].y > hs)		point[2].y = hs;

	//中間点までのScanLine
	for(int i = y; i < point[1].y; i++){
		dsx = (int)sx;
		dex = (int)ex;

		ScanLine(p, dsx, dex, cl);
		p += w;

		sx += smx;
		ex += emx;

		y++;
	}
	
	//中間点からのScanLine
	for(int i = y; i <= point[2].y; i++){
		dsx = (int)sx;
		dex = (int)ex;

		ScanLine(p, dsx, dex, cl);
		p += w;

		sx += smx2;
		ex += emx2;

		y++;
	}
}
//■グローシェーディング用三角形描画
void CDirectDrawSurface::DrawPolygon(POINT		*sp,	//POINT [3]
									 RGBQUAD	*color)	//RGBQUAD [3]
{
	//ロックされていなければ失敗
	if(! Locked){
		return;
	}

	int w = SurfaceDesc.lPitch / 4;
	int ws = Width - 1,
		hs = Height - 1;

	RGBQUAD scl, ecl;	//スキャンライン用描画色

	//並べ替えができるようにコピー
	POINT point[3];
	RGBQUAD cl[3];
	CopyMemory(point, sp, sizeof(POINT) * 3);
	CopyMemory(cl, color, sizeof(RGBQUAD) * 3);

	//point[0].y < point[1].y < point[2].y
	//になるように並び替え
	if(point[0].y > point[1].y){
		swap(point[0], point[1]);
		swap(cl[0], cl[1]);
	}
	if(point[1].y > point[2].y){
		swap(point[1], point[2]);
		swap(cl[1], cl[2]);
	}
	if(point[0].y > point[1].y){
		swap(point[0], point[1]);
		swap(cl[0], cl[1]);
	}
	if(point[0].y == point[1].y){
		if(point[0].x > point[1].x){
			swap(point[0], point[1]);
			swap(cl[0], cl[1]);
		}
	}
	//クリッピング
	if(point[2].y < 0) return;
	if(point[0].y >= hs) return;
	if( (point[0].x < 0) && (point[1].x < 0) && (point[2].x < 0) ) return;
	if( (point[0].x >= ws) && (point[1].x >= ws) && (point[2].x >= ws) ) return;

	//x増加量
	double	mx[3] = {0, 0, 0};	//計算用
	double	smx, emx,			//途中までの辺用
			smx2, emx2;			//途中からの辺用
	//色増加量
	double	dr[3] = {0, 0, 0};	//R
	double	dg[3] = {0, 0, 0};	//G
	double	db[3] = {0, 0, 0};	//B
	double	sdr, sdg, sdb, 		//途中までの辺用
			edr, edg, edb;
	double	sdr2, sdg2, sdb2,	//途中からの辺用
			edr2, edg2, edb2;
								
	//0..2		一番長い辺
	if(point[2].y - point[0].y != 0){
		mx[0] = (point[2].x - point[0].x) / (double)(point[2].y - point[0].y);
		
		dr[0] = (cl[2].rgbRed - cl[0].rgbRed) / (double)(point[2].y - point[0].y);
		dg[0] = (cl[2].rgbGreen - cl[0].rgbGreen) / (double)(point[2].y - point[0].y);
		db[0] = (cl[2].rgbBlue - cl[0].rgbBlue) / (double)(point[2].y - point[0].y);
	}
	//0..1		途中までの辺
	if(point[1].y - point[0].y != 0){
		mx[1] = (point[1].x - point[0].x) / (double)(point[1].y - point[0].y);

		dr[1] = (cl[1].rgbRed - cl[0].rgbRed) / (double)(point[1].y - point[0].y);
		dg[1] = (cl[1].rgbGreen - cl[0].rgbGreen) / (double)(point[1].y - point[0].y);
		db[1] = (cl[1].rgbBlue - cl[0].rgbBlue) / (double)(point[1].y - point[0].y);
	}
	//1..2		途中からの辺
	if(point[2].y - point[1].y != 0){
		mx[2] = (point[2].x - point[1].x) / (double)(point[2].y - point[1].y);

		dr[2] = (cl[2].rgbRed - cl[1].rgbRed) / (double)(point[2].y - point[1].y);
		dg[2] = (cl[2].rgbGreen - cl[1].rgbGreen) / (double)(point[2].y - point[1].y);
		db[2] = (cl[2].rgbBlue - cl[1].rgbBlue) / (double)(point[2].y - point[1].y);
	}
	if( (mx[0] > mx[1]) && (point[0].y != point[1].y) ){
		//左向き
		smx = mx[1];
		emx = mx[0];
		smx2 = mx[2];
		emx2 = mx[0];

		sdr = dr[1];
		sdg = dg[1];
		sdb = db[1];
		edr = dr[0];
		edg = dg[0];
		edb = db[0];
		sdr2 = dr[2];
		sdg2 = dg[2];
		sdb2 = db[2];
		edr2 = dr[0];
		edg2 = dg[0];
		edb2 = db[0];
	}
	else{
		//右向き
		smx = mx[0];
		emx = mx[1];
		smx2 = mx[0];
		emx2 = mx[2];

		sdr = dr[0];
		sdg = dg[0];
		sdb = db[0];
		edr = dr[1];
		edg = dg[1];
		edb = db[1];
		sdr2 = dr[0];
		sdg2 = dg[0];
		sdb2 = db[0];
		edr2 = dr[2];
		edg2 = dg[2];
		edb2 = db[2];
	}
	
	//辺の長いほうからScanLineする
	double sx = point[0].x;		//x開始位置
	double ex = sx;				//x終了位置
	int y = point[0].y;			//y初期化
	int dsx, dex;

	//開始描画色
	double	sr, sg, sb;
	sr = cl[0].rgbRed;
	sg = cl[0].rgbGreen;
	sb = cl[0].rgbBlue;	
	//終了描画色
	double	er = sr,
			eg = sg,
			eb = sb;
	
	if(mx[1] == 0)	ex = point[1].x;	//辺0,1辺のx座標が同じ
	
	if(dr[1] == 0)	er = cl[1].rgbRed;
	if(dg[1] == 0)	eg = cl[1].rgbGreen;
	if(db[1] == 0)	eb = cl[1].rgbBlue;

	//部分クリッピング
	if(point[0].y < 0){
		if(point[1].y < 0){
			//2点上にはみ出している
			if( (mx[0] > mx[1]) && (point[0].y != point[1].y) ){
				//右向き
				sx = point[1].x;
				sx += smx2 * -point[1].y;
				ex += emx2 * -point[0].y;

				sr += sdr2 * -point[1].y;
				sg += sdg2 * -point[1].y;
				sb += sdb2 * -point[1].y;
				er += edr2 * -point[0].y;
				eg += edg2 * -point[0].y;
				eb += edb2 * -point[0].y;
			}
			else{
				ex = point[1].x;
				sx += smx2 * -point[0].y;
				ex += emx2 * -point[1].y;

				sr += sdr2 * -point[1].y;
				sg += sdg2 * -point[1].y;
				sb += sdb2 * -point[1].y;
				er += edr2 * -point[0].y;
				eg += edg2 * -point[0].y;
				eb += edb2 * -point[0].y;
			}
		}
		else{
			//1点上にはみ出している
			sx += smx * -point[0].y;
			ex += emx * -point[0].y;

			sr += sdr * -point[0].y;
			sg += sdg * -point[0].y;
			sb += sdb * -point[0].y;
			er += edr * -point[0].y;
			eg += edg * -point[0].y;
			eb += edb * -point[0].y;
		}
		y = 0;
	}

	//描画
	RGBQUAD *p = (RGBQUAD*)SurfaceDesc.lpSurface;
	p += w * y;

	if( (point[0].y == point[1].y) && (point[0].y == point[2].y) ){
		//3点のyが同じ
		//0..1..2の順に並び替える
		if(point[0].x > point[1].x)		swap(point[0].x, point[1].x);
		if(point[1].x > point[2].x)		swap(point[1].x, point[2].x);
		if(point[0].x > point[1].x)		swap(point[0].x, point[1].x);

		dsx = point[0].x;
		dex = point[2].x;

		ScanLine(p, dsx, dex, cl[0]);

		return;
	}
	//y座標クリッピング
	if(point[1].y > hs)		point[1].y = hs;
	if(point[2].y > hs)		point[2].y = hs;
	
	//中間点までのScanLine
	for(int i = y; i < point[1].y; i++){
		dsx = (int)sx;
		dex = (int)ex;

		scl.rgbRed = (BYTE)sr;
		scl.rgbGreen = (BYTE)sg;
		scl.rgbBlue = (BYTE)sb;

		ecl.rgbRed = (BYTE)er;
		ecl.rgbGreen = (BYTE)eg;
		ecl.rgbBlue = (BYTE)eb;

		ScanLine(p, dsx, dex, scl, ecl);
		p += w;

		sx += smx;
		ex += emx;

		sr += sdr;
		sg += sdg;
		sb += sdb;
		er += edr;
		eg += edg;
		eb += edb;

		y++;
	}
	//中間点からのScanLine
	for(int i = y; i <= point[2].y; i++){
		dsx = (int)sx;
		dex = (int)ex;

		scl.rgbRed = (BYTE)sr;
		scl.rgbGreen = (BYTE)sg;
		scl.rgbBlue = (BYTE)sb;

		ecl.rgbRed = (BYTE)er;
		ecl.rgbGreen = (BYTE)eg;
		ecl.rgbBlue = (BYTE)eb;

		ScanLine(p, dsx, dex, scl, ecl);
		p += w;

		sx += smx2;
		ex += emx2;

		sr += sdr2;
		sg += sdg2;
		sb += sdb2;
		er += edr2;
		eg += edg2;
		eb += edb2;

		y++;
	}
	wsprintf(DebugStr, "0 : %d, (%d, %d, %d)", point[0].y, cl[0].rgbRed, cl[0].rgbGreen, cl[0].rgbBlue);
	wsprintf(DebugStr2, "1 : %d, (%d, %d, %d)", point[1].y, cl[1].rgbRed, cl[1].rgbGreen, cl[1].rgbBlue);
	wsprintf(DebugStr3, "2 : %d, (%d, %d, %d)", point[2].y, cl[2].rgbRed, cl[2].rgbGreen, cl[2].rgbBlue);
}

void CDirectDrawSurface::DrawPolygonHumanTouch( POINT* p, RGBQUAD c )
{
	{
		int div = 50; // line + 1

		double x1 = p[ 0 ].x;
		double y1 = p[ 0 ].y;
		double x2 = p[ 0 ].x;
		double y2 = p[ 0 ].y;

		double dx1 = double( p[ 1 ].x - p[ 0 ].x ) / div;
		double dy1 = double( p[ 1 ].y - p[ 0 ].y ) / div;
		double dx2 = double( p[ 2 ].x - p[ 0 ].x ) / div;
		double dy2 = double( p[ 2 ].y - p[ 0 ].y ) / div;

		for ( int n = 0; n < div - 1; n++ )
		{
			x1 += dx1;
			y1 += dy1;
			x2 += dx2;
			y2 += dy2;

			DrawLineHumanTouch( x1, y1, x2, y2, c );
		}

	}
	/*
	{
		int div = 20; // line + 1

		double x1 = p[ 1 ].x;
		double y1 = p[ 1 ].y;
		double x2 = p[ 1 ].x;
		double y2 = p[ 1 ].y;

		double dx1 = double( p[ 0 ].x - p[ 1 ].x ) / div;
		double dy1 = double( p[ 0 ].y - p[ 1 ].y ) / div;
		double dx2 = double( p[ 2 ].x - p[ 1 ].x ) / div;
		double dy2 = double( p[ 2 ].y - p[ 1 ].y ) / div;

		for ( int n = 0; n < div - 1; n++ )
		{
			x1 += dx1;
			y1 += dy1;
			x2 += dx2;
			y2 += dy2;

			DrawLineHumanTouch( x1, y1, x2, y2, c );
		}
	}
	*/
}

//■ぼかし
void CDirectDrawSurface::Blur(RECT rc,				//ぼかす範囲
							  vector<POINT> &v)		//参照ピクセル配列
{
	if(! Locked)	return;

	int w = SurfaceDesc.lPitch / 4;
	int dw = rc.right - rc.left;
	int dh = rc.bottom - rc.top;

	int size = v.size();
	RGBQUAD *sp = (RGBQUAD*)SurfaceDesc.lpSurface;
	RGBQUAD *dp = sp + rc.top * w + rc.left;

	for(int y = rc.top; y < rc.bottom; y++){
		for(int x = rc.left; x < rc.right; x++){
			int r = 0, g = 0, b = 0;
			for(int i = 0; i < size; i++){
				sp = dp + v[i].y * w + v[i].x;
				r += sp->rgbRed;
				g += sp->rgbGreen;
				b += sp->rgbBlue;
			}
			dp->rgbRed = r / size;
			dp->rgbGreen = g / size;
			dp->rgbBlue = b / size;
			dp++;
		}
		dp += w - dw;
	}
}

//■ぼかし
void CDirectDrawSurface::Blur(	CDirectDrawSurface	*lpsrc,
								RECT				rc,
								vector<POINT>		&v)
{
	if(! Locked)						return;
	if(lpsrc == NULL)					return;
	if(! lpsrc->GetLocked())			return;
	if(Width != lpsrc->GetWidth())		return;
	if(Height != lpsrc->GetHeight())	return;
	if(v.size() <= 0)					return;

	int sw = lpsrc->GetSurfaceDesc()->lPitch / 4;	//転送元横ドット数
	int dw = SurfaceDesc.lPitch / 4;				//転送先横ドット数
	int w = rc.right - rc.left;						//ぼかし範囲
	int h = rc.bottom - rc.top;

	int size = v.size();
	RGBQUAD *sp = (RGBQUAD*)lpsrc->GetSurfaceDesc()->lpSurface;
	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;
	sp += rc.top * sw + rc.left;
	dp += rc.top * dw + rc.left;

	for(int y = rc.top; y < rc.bottom; y++){
		for(int x = rc.left; x < rc.right; x++){
			int s = 0;
			int r = 0, g = 0, b = 0;
			for(int i = 0; i < size; i++){
				if(x + v[i].x < 0)			continue;
				if(x + v[i].x >= Width)		continue;
				if(y + v[i].y < 0)			continue;
				if(y + v[i].y >= Height)	continue;

				RGBQUAD *p = sp + v[i].y * sw + v[i].x;
				r += p->rgbRed;
				g += p->rgbGreen;
				b += p->rgbBlue;
				s++;
			}
			if(s > 0){
				dp->rgbRed = r / s;
				dp->rgbGreen = g / s;
				dp->rgbBlue = b / s;
			}
			sp++;
			dp++;
		}
		sp += sw - w;
		dp += dw - w;
	}
}

//■円描画
void CDirectDrawSurface::DrawCircle(
							int		sx,		//始点
							int		sy,
							int		r,		//半径
							RGBQUAD	cl,		//色
							bool	a)		//アルファフラグ
{
	if( ! Locked ) return;

	int w = SurfaceDesc.lPitch / 4;
	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;
	
	r = abs(r);
	dp += (sy - r) * w + (sx - r);

	for(int y = sy - r; y < sy + r; y++){
		if( (y < 0) || (y >= Height) ){
			dp += w;
			continue;
		}
		int ly = y - sy;

		for(int x = sx - r; x < sx + r; x++, dp++){
			if( (x < 0) || (x >= Width) )	continue;
			
			int lx = x - sx;
			double len = sqrt(double(lx * lx + ly * ly));
			if(len > r)		continue;
			RGBQUAD tmp_cl = cl;
			if(a){			//アルファ合成
				double rate = cl.rgbReserved / double(255);
				cl.rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				cl.rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				cl.rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			if(len > r - 1){	//アンチエイリアス
				double rate = r - len;
				dp->rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				dp->rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			else{
				*dp = cl;
			}
			cl = tmp_cl;
		}
		dp += w - r * 2;
	}

}

//■円描画
void CDirectDrawSurface::DrawCircle( double sx, double sy, double r, RGBQUAD cl, bool a )
{
	if ( ! Locked ) return;

	int w = SurfaceDesc.lPitch / 4;
	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;
	
	r = abs( r );

	int left   = static_cast< int >( floor( sx - r ) );
	int top    = static_cast< int >( floor( sy - r ) );
	int right  = static_cast< int >( ceil( sx + r ) );
	int bottom = static_cast< int >( ceil( sy + r ) );

	dp += top * w + left;

	for ( int y = top; y < bottom; y++ )
	{
		if ( ( y < 0 ) || ( y >= Height ) )
		{
			dp += w;
			continue;
		}

		double ly = y - sy;

		for(int x = left; x < right; x++, dp++ )
		{
			if( (x < 0) || (x >= Width) )	continue;
			
			double lx = x - sx;
			double len = sqrt( lx * lx + ly * ly );

			if ( len > r )
			{
				continue;
			}

			RGBQUAD tmp_cl = cl;
			if(a){			//アルファ合成
				double rate = cl.rgbReserved / double(255);
				cl.rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				cl.rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				cl.rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			if(len > r - 1){	//アンチエイリアス
				double rate = r - len;
				dp->rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				dp->rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			else{
				*dp = cl;
			}
			cl = tmp_cl;
		}

		dp += w - ( right - left );
	}

}

//■輪描画
void CDirectDrawSurface::DrawRing(
							int		sx,		//始点
							int		sy,
							int		max_r,	//半径
							int		min_r,
							RGBQUAD	cl,		//色
							bool	a)		//アルファフラグ
{
	if( ! Locked ) return;

	int w = SurfaceDesc.lPitch / 4;
	RGBQUAD *dp = (RGBQUAD*)SurfaceDesc.lpSurface;

	max_r = abs(max_r);
	min_r = abs(min_r);
	dp += (sy - max_r) * w + (sx - max_r);

	for(int y = sy - max_r; y < sy + max_r; y++){
		if( (y < 0) || (y >= Height) ){
			dp += w;
			continue;
		}
		int ly = y - sy;

		for(int x = sx - max_r; x < sx + max_r; x++, dp++){
			if( (x < 0) || (x >= Width) )	continue;
			
			int lx = x - sx;
			double len = sqrt(double(lx * lx + ly * ly));
			if(len > max_r)		continue;
			if(len < min_r)		continue;
			RGBQUAD tmp_cl = cl;
			if(a){			//アルファ合成
				double rate = cl.rgbReserved / double(255);
				cl.rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				cl.rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				cl.rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			double	rate;
			bool	aa = false;
			if(len < min_r + 1){
				aa = true;
				rate = len - min_r;
			}
			if(len > max_r - 1){
				aa = true;
				rate = max_r - len;
			}
			if(aa){
				dp->rgbRed   = static_cast< int >( cl.rgbRed   * rate + dp->rgbRed   * ( 1.0 - rate ) );
				dp->rgbGreen = static_cast< int >( cl.rgbGreen * rate + dp->rgbGreen * ( 1.0 - rate ) );
				dp->rgbBlue  = static_cast< int >( cl.rgbBlue  * rate + dp->rgbBlue  * ( 1.0 - rate ) );
			}
			else{
				*dp = cl;
			}
			cl = tmp_cl;
		}
		dp += w - max_r * 2;
	}

}

//□水平線描画
void CDirectDrawSurface::ScanLine(RGBQUAD *prgbq,	//x=0, y=nのアドレス
								  int sx, int ex,	//始点，終点
								  RGBQUAD cl)		//色
{
	//swap
	if(sx > ex){
		swap(sx, ex);
	}
	//clipping
	if(sx >= Width)		return;
	if(ex < 0)			return;

	if(sx < 0)			sx = 0;
	if(ex >= Width)		ex = Width - 1;

	int len = ex - sx + 1;
	
	prgbq += sx;
	for(int i = 0; i < len; i++){
		*prgbq = cl;
		prgbq++;
	}
}
void CDirectDrawSurface::ScanLine(RGBQUAD *prgbq,	//x=0, y=nのアドレス
								  int sx, int ex,	//始点，終点
								  RGBQUAD scl,		//始点の色
								  RGBQUAD ecl)		//終点の色
{
	/*/swap
	if(sx > ex){
		swap(sx, ex);
		swap(scl, ecl);
	}
	/*/
	//clipping
	if(sx >= Width)		return;
	if(ex < 0)			return;

	int len = ex - sx + 1;		//線の長さ

	double r, g, b;		//計算用R,G,B
	r = scl.rgbRed;
	g = scl.rgbGreen;
	b = scl.rgbBlue;

	double dr, dg, db;	//増加量
	dr = (ecl.rgbRed - scl.rgbRed) / (double)len;
	dg = (ecl.rgbGreen - scl.rgbGreen) / (double)len;
	db = (ecl.rgbBlue - scl.rgbBlue) / (double)len;

	//clipping
	if(sx < 0){
		r += dr * -sx;
		g += dg * -sx;
		b += db * -sx;
		sx = 0;
	}
	if(ex >= Width){
		ex = Width - 1;
	}
	len = ex - sx + 1;

	prgbq += sx;
	RGBQUAD cl;
	for(int i = 0; i < len; i++){
		cl.rgbRed = (BYTE)r;
		cl.rgbGreen = (BYTE)g;
		cl.rgbBlue = (BYTE)b;

		*prgbq = cl;
		prgbq++;

		r += dr;
		g += dg;
		b += db;
	}
}

//□RGB(r, g, b)で作成されたCOLORREFのRとBを入れ替える
void CDirectDrawSurface::RGB_Swap(DWORD &cl)
{
	BYTE r = GetRValue(cl);
	BYTE g = GetGValue(cl);
	BYTE b = GetBValue(cl);

	cl = RGB(b, g, r);
}
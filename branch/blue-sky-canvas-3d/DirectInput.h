#ifndef DIRECT_INPUT_H
#define DIRECT_INPUT_H

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

/**
 * DirectInput のラッパークラス
 *
 */
class DirectInput
{
private:
	LPDIRECTINPUT7 direct_input_;

public:
	DirectInput( HINSTANCE );
	~DirectInput();

}; // class DirectInput

/*
//プロトタイプ宣言
BOOL	DI_Init( HINSTANCE );

//キーボードデバイス作成
LPDIRECTINPUTDEVICE7 DI_CreateKeyDevice(HWND);
//ジョイスティックデバイス作成
LPDIRECTINPUTDEVICE7 DI_CreateJoyDevice(HWND);
BOOL CALLBACK GetJoyDevice(LPDIDEVICEINSTANCE, LPVOID);

void	DI_Release(LPDIRECTINPUTDEVICE7&);
void	DI_UnInit();

extern	LPDIRECTINPUT7	lpDI;
*/

#endif // DIRECT_3D_H


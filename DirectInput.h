#ifndef DIRECT_INPUT_H
#define DIRECT_INPUT_H

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

/**
 * DirectInput �̃��b�p�[�N���X
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
//�v���g�^�C�v�錾
BOOL	DI_Init( HINSTANCE );

//�L�[�{�[�h�f�o�C�X�쐬
LPDIRECTINPUTDEVICE7 DI_CreateKeyDevice(HWND);
//�W���C�X�e�B�b�N�f�o�C�X�쐬
LPDIRECTINPUTDEVICE7 DI_CreateJoyDevice(HWND);
BOOL CALLBACK GetJoyDevice(LPDIDEVICEINSTANCE, LPVOID);

void	DI_Release(LPDIRECTINPUTDEVICE7&);
void	DI_UnInit();

extern	LPDIRECTINPUT7	lpDI;
*/

#endif // DIRECT_3D_H


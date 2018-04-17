#include "DirectInput.h"
#include <core/DirectX.h>

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

/**
 * �R���X�g���N�^
 */
DirectInput::DirectInput( HINSTANCE hinstance, HWND hwnd )
	: direct_input_( 0 )
	, mouse_device_( 0 )
{
	DIRECT_X_FAIL_CHECK( DirectInput8Create( hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast< void** >( & direct_input_ ), 0 ) );


	DIRECT_X_FAIL_CHECK( direct_input_->CreateDevice( GUID_SysMouse, & mouse_device_, 0 ) );
	DIRECT_X_FAIL_CHECK( mouse_device_->SetDataFormat( & c_dfDIMouse ) );
	DIRECT_X_FAIL_CHECK( mouse_device_->SetCooperativeLevel( hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) );
	
	DIPROPDWORD prop = { sizeof( DIPROPDWORD ), { sizeof( DIPROPHEADER ) } };
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = DIPROPAXISMODE_REL;

	DIRECT_X_FAIL_CHECK( mouse_device_->SetProperty( DIPROP_AXISMODE, & prop.diph ) );

	mouse_device_->Acquire();
}

DirectInput::~DirectInput()
{
	mouse_device_->Unacquire();
	mouse_device_->Release();
	direct_input_->Release();
}

void DirectInput::update()
{
	mouse_device_->Acquire();
	mouse_device_->GetDeviceState( sizeof( DIMOUSESTATE ), & mouse_state_ );
}

/*
//�}�N��
	//�������x��
#define	DI_COOPERATIVE	(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)



//���������@�L�[�{�[�h�p�f�o�C�X���쐬����@��������				
//�쐬���ꂽ�f�o�C�X�ւ̃|�C���^���Ԃ�
LPDIRECTINPUTDEVICE7 DI_CreateKeyDevice(HWND hWnd)
{
	LPDIRECTINPUTDEVICE7	lpKey = NULL;
	HRESULT					hr;
	
	if(lpDI == NULL)		return NULL;

	hr = lpDI->CreateDeviceEx(
		GUID_SysKeyboard,			//�쐬����f�o�C�X���w��
		IID_IDirectInputDevice7,	//�C���^�[�t�F�C�X�̎��ʎq
		(void **) &lpKey,			//�f�o�C�X���󂯎��A�h���X
		NULL);						//�ʏ�NULL
	if(hr != DI_OK){
		return NULL;
	}

	//�f�[�^�t�H�[�}�b�g�̐ݒ�
	hr = lpKey->SetDataFormat(&c_dfDIKeyboard);
	if(hr != DI_OK){
		return NULL;
	}

	//�������x���̐ݒ�
	hr = lpKey->SetCooperativeLevel(
			hWnd,	//�f�o�C�X�Ɋ֘A�t�����Ă���E�B���h�E�n���h��
			DI_COOPERATIVE);	//�������x��
	if(hr != DI_OK){
		return NULL;
	}

	//����
	return lpKey;
}

//���������@�W���C�X�e�B�b�N�p�f�o�C�X���쐬����@��������				
//�쐬���ꂽ�f�o�C�X�ւ̃|�C���^���Ԃ�
LPDIRECTINPUTDEVICE7 DI_CreateJoyDevice(
	HWND hWnd)		//�E�B���h�E�n���h��
{
	LPDIRECTINPUTDEVICE7	lpJoy = NULL;
	HRESULT					hr;

	//DirectInput�I�u�W�F�N�g���쐬�ς݂��`�F�b�N
	if(lpDI == NULL)		return NULL;

	//�W���C�X�e�B�b�N�̗�
	hr = lpDI->EnumDevices(
		DIDEVTYPE_JOYSTICK,		//�T���o���f�o�C�X�̎��
		(LPDIENUMDEVICESCALLBACK)GetJoyDevice,
								//�񋓎��ɌĂяo�����CALLBACK �֐�
		(LPVOID) &lpJoy,		//���������W���C�X�e�B�b�N�f�o�C�X���Ԃ�
		DIEDFL_ATTACHEDONLY);	//�񋓂͈̔͂��w�肷��t���O
	if((hr != DI_OK) || (lpJoy == NULL)){
		return NULL;
	}

	//�f�[�^�t�H�[�}�b�g�̐ݒ�
	hr = lpJoy->SetDataFormat(&c_dfDIJoystick);
	if(hr != DI_OK){
		return NULL;
	}

	//�������x���̐ݒ�
	lpJoy->SetCooperativeLevel(hWnd, DI_COOPERATIVE);
	if(hr != DI_OK){
		return NULL;
	}

	//�f�o�C�X�̐ݒ�
		//�����[�h�̐ݒ�
	DIPROPDWORD		diprop = {0};	//�����[�h�ݒ�p�ϐ�
			//�\���̂̃T�C�Y�ŏ���������
	diprop.diph.dwSize = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	
	diprop.diph.dwHow = DIPH_DEVICE;	//�f�o�C�X�ύX�̂��߂̃t���O
	diprop.diph.dwObj = 0;				//dwHow ��DIPH_DEVICE �̏ꍇ 0
	diprop.dwData = DIPROPAXISMODE_ABS;	//��Ύ����[�h
	lpJoy->SetProperty(DIPROP_AXISMODE, &diprop.diph);
		//�l�͈̔͂̐ݒ�
	DIPROPRANGE diprg = {0};	//���͈͐ݒ�p�ϐ�
	diprg.diph.dwSize = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwObj = DIJOFS_X;	//x���̐ݒ�
	diprg.diph.dwHow = DIPH_BYOFFSET;	//dwobj �ɂ�茈�܂��Ă���
	diprg.lMin = -1000;			//���̍ŏ��l
	diprg.lMax = 1000;			//���̍ő�l
	//x���̐ݒ�
	lpJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
	//y���̐ݒ�
	diprg.diph.dwObj = DIJOFS_Y;
	lpJoy->SetProperty(DIPROP_RANGE, &diprg.diph);

	//����
	return lpJoy;
}

//���������@�W���C�X�e�B�b�N�f�o�C�X�쐬�p�R�[���o�b�N�֐��@��������		
BOOL CALLBACK GetJoyDevice(
	LPDIDEVICEINSTANCE	lpDevInst,	//���������f�o�C�X���
	LPVOID		lpVoid)	//EnumDevices()�̑�3�������n�����
{
	LPDIRECTINPUTDEVICE7	lpDI_Joy = NULL;
	HRESULT					hr;

	//�f�o�C�X�̍쐬
	hr = lpDI->CreateDeviceEx(
		lpDevInst->guidInstance,
		IID_IDirectInputDevice7,
		(void **) &lpDI_Joy,
		NULL);
	if(hr != DI_OK){
	//�f�o�C�X���쐬�ł��Ȃ���Ύ���T��
		return DIENUM_CONTINUE;
	}
	//�쐬�����f�o�C�X��Ԃ�
	*(LPDIRECTINPUTDEVICE7 *)lpVoid = lpDI_Joy;
	//�񋓂��I������
	return DIENUM_STOP;
}

//���������@DirectInput�f�o�C�X�J���@��������
void DI_Release(LPDIRECTINPUTDEVICE7 &lpDevice)
{
	if(lpDevice != NULL){
		lpDevice->Release();
		lpDevice = NULL;
	}
}		

//���������@DirectInput�I�������@��������
//DirectInput�I�u�W�F�N�g�̊J��
void DI_UnInit()
{
	if(lpDI != NULL){
		lpDI->Release();
		lpDI = NULL;
	}
}
*/
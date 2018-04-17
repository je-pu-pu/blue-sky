#include "DirectInput.h"
#include <core/DirectX.h>

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

/**
 * コンストラクタ
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
//マクロ
	//協調レベル
#define	DI_COOPERATIVE	(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)



//■■■■　キーボード用デバイスを作成する　■■■■				
//作成されたデバイスへのポインタが返る
LPDIRECTINPUTDEVICE7 DI_CreateKeyDevice(HWND hWnd)
{
	LPDIRECTINPUTDEVICE7	lpKey = NULL;
	HRESULT					hr;
	
	if(lpDI == NULL)		return NULL;

	hr = lpDI->CreateDeviceEx(
		GUID_SysKeyboard,			//作成するデバイスを指定
		IID_IDirectInputDevice7,	//インターフェイスの識別子
		(void **) &lpKey,			//デバイスを受け取るアドレス
		NULL);						//通常NULL
	if(hr != DI_OK){
		return NULL;
	}

	//データフォーマットの設定
	hr = lpKey->SetDataFormat(&c_dfDIKeyboard);
	if(hr != DI_OK){
		return NULL;
	}

	//協調レベルの設定
	hr = lpKey->SetCooperativeLevel(
			hWnd,	//デバイスに関連付けられているウィンドウハンドル
			DI_COOPERATIVE);	//協調レベル
	if(hr != DI_OK){
		return NULL;
	}

	//成功
	return lpKey;
}

//■■■■　ジョイスティック用デバイスを作成する　■■■■				
//作成されたデバイスへのポインタが返る
LPDIRECTINPUTDEVICE7 DI_CreateJoyDevice(
	HWND hWnd)		//ウィンドウハンドル
{
	LPDIRECTINPUTDEVICE7	lpJoy = NULL;
	HRESULT					hr;

	//DirectInputオブジェクトが作成済みかチェック
	if(lpDI == NULL)		return NULL;

	//ジョイスティックの列挙
	hr = lpDI->EnumDevices(
		DIDEVTYPE_JOYSTICK,		//探し出すデバイスの種類
		(LPDIENUMDEVICESCALLBACK)GetJoyDevice,
								//列挙時に呼び出されるCALLBACK 関数
		(LPVOID) &lpJoy,		//見つかったジョイスティックデバイスが返る
		DIEDFL_ATTACHEDONLY);	//列挙の範囲を指定するフラグ
	if((hr != DI_OK) || (lpJoy == NULL)){
		return NULL;
	}

	//データフォーマットの設定
	hr = lpJoy->SetDataFormat(&c_dfDIJoystick);
	if(hr != DI_OK){
		return NULL;
	}

	//協調レベルの設定
	lpJoy->SetCooperativeLevel(hWnd, DI_COOPERATIVE);
	if(hr != DI_OK){
		return NULL;
	}

	//デバイスの設定
		//軸モードの設定
	DIPROPDWORD		diprop = {0};	//軸モード設定用変数
			//構造体のサイズで初期化する
	diprop.diph.dwSize = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	
	diprop.diph.dwHow = DIPH_DEVICE;	//デバイス変更のためのフラグ
	diprop.diph.dwObj = 0;				//dwHow がDIPH_DEVICE の場合 0
	diprop.dwData = DIPROPAXISMODE_ABS;	//絶対軸モード
	lpJoy->SetProperty(DIPROP_AXISMODE, &diprop.diph);
		//値の範囲の設定
	DIPROPRANGE diprg = {0};	//軸範囲設定用変数
	diprg.diph.dwSize = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwObj = DIJOFS_X;	//x軸の設定
	diprg.diph.dwHow = DIPH_BYOFFSET;	//dwobj により決まっている
	diprg.lMin = -1000;			//軸の最小値
	diprg.lMax = 1000;			//軸の最大値
	//x軸の設定
	lpJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
	//y軸の設定
	diprg.diph.dwObj = DIJOFS_Y;
	lpJoy->SetProperty(DIPROP_RANGE, &diprg.diph);

	//成功
	return lpJoy;
}

//■■■■　ジョイスティックデバイス作成用コールバック関数　■■■■		
BOOL CALLBACK GetJoyDevice(
	LPDIDEVICEINSTANCE	lpDevInst,	//発見したデバイス情報
	LPVOID		lpVoid)	//EnumDevices()の第3引数が渡される
{
	LPDIRECTINPUTDEVICE7	lpDI_Joy = NULL;
	HRESULT					hr;

	//デバイスの作成
	hr = lpDI->CreateDeviceEx(
		lpDevInst->guidInstance,
		IID_IDirectInputDevice7,
		(void **) &lpDI_Joy,
		NULL);
	if(hr != DI_OK){
	//デバイスが作成できなければ次を探す
		return DIENUM_CONTINUE;
	}
	//作成したデバイスを返す
	*(LPDIRECTINPUTDEVICE7 *)lpVoid = lpDI_Joy;
	//列挙を終了する
	return DIENUM_STOP;
}

//■■■■　DirectInputデバイス開放　■■■■
void DI_Release(LPDIRECTINPUTDEVICE7 &lpDevice)
{
	if(lpDevice != NULL){
		lpDevice->Release();
		lpDevice = NULL;
	}
}		

//■■■■　DirectInput終了処理　■■■■
//DirectInputオブジェクトの開放
void DI_UnInit()
{
	if(lpDI != NULL){
		lpDI->Release();
		lpDI = NULL;
	}
}
*/
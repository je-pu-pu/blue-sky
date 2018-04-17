#ifndef DIRECT_INPUT_H
#define DIRECT_INPUT_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

/**
 * DirectInput のラッパークラス
 *
 */
class DirectInput
{
private:
	LPDIRECTINPUT8 direct_input_;
	LPDIRECTINPUTDEVICE8 mouse_device_;
	DIMOUSESTATE mouse_state_;

public:
	DirectInput( HINSTANCE, HWND );
	~DirectInput();

	void update();
	const DIMOUSESTATE& get_mouse_state() const { return mouse_state_; }

}; // class DirectInput

#endif // DIRECT_3D_H

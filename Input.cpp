//**********************************************************
/**
 * Input の実装
 *
 * copyright (c) je-pu-pu all rights reserved
 *
 * @file		Input.cpp
 * @date		2006/09/12
 * @author		JE
 */
//**********************************************************

#include "Input.h"
#include <windows.h>

namespace blue_sky
{

Input::Input()
{
	for ( int n = 0; n < MAX_BUTTONS; n++ )
	{
		state_[ n ] = 0;
	}

	joy_info_.dwSize = sizeof( JOYINFOEX );
	joy_info_.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS;
}

/**
 * キーボードからの入力による状態の更新を行う
 */
void Input::update()
{
	for ( int n = 0; n < MAX_BUTTONS; n++ )
	{
		state_[ n ] <<= 1;
	}

	if ( GetAsyncKeyState( 'A' ) & 0x8000 ) state_[ LEFT  ] |= 1;
	if ( GetAsyncKeyState( 'D' ) & 0x8000 ) state_[ RIGHT ] |= 1;
	if ( GetAsyncKeyState( 'W' ) & 0x8000 ) state_[ UP    ] |= 1;
	if ( GetAsyncKeyState( 'S' ) & 0x8000 ) state_[ DOWN  ] |= 1;
	if ( GetAsyncKeyState( 'Q' ) & 0x8000 ) state_[ L     ] |= 1;
	if ( GetAsyncKeyState( 'E' ) & 0x8000 ) state_[ R     ] |= 1;

	if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) state_[ A ] |= 1;
	if ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) state_[ B ] |= 1;

	if ( GetAsyncKeyState( '1' ) & 0x8000 ) state_[ X ] |= 1;
	if ( GetAsyncKeyState( '2' ) & 0x8000 ) state_[ Y ] |= 1;

	if ( joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR )
	{
		if ( joy_info_.dwXpos <  0xFFFF / 4 * 1 ) state_[ LEFT  ] |= 1;
		if ( joy_info_.dwXpos >= 0xFFFF / 4 * 3 ) state_[ RIGHT ] |= 1;
		if ( joy_info_.dwYpos <  0xFFFF / 4 * 1 ) state_[ UP    ] |= 1;
		if ( joy_info_.dwYpos >= 0xFFFF / 4 * 3 ) state_[ DOWN  ] |= 1;
	
		if ( joy_info_.dwButtons & JOY_BUTTON1 ) state_[ A ] |= 1;
		if ( joy_info_.dwButtons & JOY_BUTTON3 ) state_[ B ] |= 1;
	
		if ( joy_info_.dwButtons & JOY_BUTTON5 ) state_[ L ] |= 1;
		if ( joy_info_.dwButtons & JOY_BUTTON6 ) state_[ R ] |= 1;
	}

	update_common();
}

/**
 * キーボード・ジョイスティック共通の更新処理を行う
 */
void Input::update_common()
{
	if ( release( DOWN  ) ) allow_pop( DOWN  );
	if ( release( UP    ) ) allow_pop( UP    );
	if ( release( RIGHT ) ) allow_pop( RIGHT );
	if ( release( LEFT  ) ) allow_pop( LEFT  );

	if ( push( DOWN  ) ) allow_push( DOWN  );
	if ( push( UP    ) ) allow_push( UP    );
	if ( push( RIGHT ) ) allow_push( RIGHT );
	if ( push( LEFT  ) ) allow_push( LEFT  );
}

} // namespace blue_sky
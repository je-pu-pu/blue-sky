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
#include <game/Config.h>
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

	joystick_enabled_ = ( joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR );
}

void Input::load_config( Config& config )
{
	key_code_[ LEFT  ] = config.get( "input.key.left",  'A' );
	key_code_[ RIGHT ] = config.get( "input.key.right", 'D' );
	key_code_[ UP    ] = config.get( "input.key.up",    'W' );
	key_code_[ DOWN  ] = config.get( "input.key.down",  'S' );
	
	key_code_[ A ] = config.get( "input.key.a", VK_LBUTTON );
	key_code_[ B ] = config.get( "input.key.b", VK_RBUTTON );
	key_code_[ X ] = config.get( "input.key.x", '1' );
	key_code_[ Y ] = config.get( "input.key.y", '2' );

	key_code_[ L ] = config.get( "input.key.l", 'Q' );
	key_code_[ R ] = config.get( "input.key.r", 'E' );

	joystick_code_[ A ] = 1 << ( config.get( "input.joystick.a", 1 ) - 1 );
	joystick_code_[ B ] = 1 << ( config.get( "input.joystick.b", 3 ) - 1 );
	
	joystick_code_[ X ] = 1 << ( config.get( "input.joystick.x", 2 ) - 1 );
	joystick_code_[ Y ] = 1 << ( config.get( "input.joystick.y", 4 ) - 1 );
	
	joystick_code_[ L ] = 1 << ( config.get( "input.joystick.l", 5 ) - 1 );
	joystick_code_[ R ] = 1 << ( config.get( "input.joystick.r", 6 ) - 1 );
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

	if ( GetAsyncKeyState( key_code_[ LEFT  ] ) & 0x8000 ) state_[ LEFT  ] |= 1;
	if ( GetAsyncKeyState( key_code_[ RIGHT ] ) & 0x8000 ) state_[ RIGHT ] |= 1;
	if ( GetAsyncKeyState( key_code_[ UP    ] ) & 0x8000 ) state_[ UP    ] |= 1;
	if ( GetAsyncKeyState( key_code_[ DOWN  ] ) & 0x8000 ) state_[ DOWN  ] |= 1;
	if ( GetAsyncKeyState( key_code_[ L     ] ) & 0x8000 ) state_[ L     ] |= 1;
	if ( GetAsyncKeyState( key_code_[ R     ] ) & 0x8000 ) state_[ R     ] |= 1;

	if ( GetAsyncKeyState( key_code_[ A ] ) & 0x8000 ) state_[ A ] |= 1;
	if ( GetAsyncKeyState( key_code_[ B ] ) & 0x8000 ) state_[ B ] |= 1;

	if ( GetAsyncKeyState( key_code_[ X ] ) & 0x8000 ) state_[ X ] |= 1;
	if ( GetAsyncKeyState( key_code_[ Y ] ) & 0x8000 ) state_[ Y ] |= 1;

	if ( joystick_enabled_ && joyGetPosEx( JOYSTICKID1, & joy_info_ ) == JOYERR_NOERROR )
	{
		if ( joy_info_.dwXpos <  0xFFFF / 4 * 1 ) state_[ LEFT  ] |= 1;
		if ( joy_info_.dwXpos >= 0xFFFF / 4 * 3 ) state_[ RIGHT ] |= 1;
		if ( joy_info_.dwYpos <  0xFFFF / 4 * 1 ) state_[ UP    ] |= 1;
		if ( joy_info_.dwYpos >= 0xFFFF / 4 * 3 ) state_[ DOWN  ] |= 1;
	
		if ( joy_info_.dwButtons & joystick_code_[ A ] ) state_[ A ] |= 1;
		if ( joy_info_.dwButtons & joystick_code_[ B ] ) state_[ B ] |= 1;

		if ( joy_info_.dwButtons & joystick_code_[ X ] ) state_[ X ] |= 1;
		if ( joy_info_.dwButtons & joystick_code_[ Y ] ) state_[ Y ] |= 1;
	
		if ( joy_info_.dwButtons & joystick_code_[ L ] ) state_[ L ] |= 1;
		if ( joy_info_.dwButtons & joystick_code_[ R ] ) state_[ R ] |= 1;
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
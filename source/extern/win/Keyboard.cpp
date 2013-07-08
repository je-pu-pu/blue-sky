// *********************************************************
// Keyboard.cpp
// copyright (c) JE all rights reserved
// 2003/2/4
// *********************************************************


#include "Keyboard.h"
#include <windows.h>
#include <iostream>

namespace win
{

using namespace driver;

std::map< int, int > Keyboard::key_map_;

Keyboard::Keyboard()
{
	if ( key_map_.empty() ) create_key_map();
}

Keyboard::~Keyboard()
{
}

void Keyboard::update()
{
	for ( int n = 0; n < MAX_KEY; n++ )
	{
		state_[ n ].update( ( GetAsyncKeyState( n ) & 0x8000 ) != 0 );
	}
}

//! キーの状態を返す
const Keyboard::State Keyboard::key( Keys key ) const
{
	return state_[ key_map_[ key ] ];
}

//! キーマップを作成する
void Keyboard::create_key_map()
{
	for ( int n = Key::KEY_A; n <= Key::KEY_Z; n++ )
	{
		key_map( n, 'A' + n - Key::KEY_A );
	}

	key_map( Key::KEY_ESC, VK_ESCAPE );
	key_map( Key::KEY_TAB, VK_TAB );
	key_map( Key::KEY_SHIFT, VK_SHIFT );
	key_map( Key::KEY_CTRL, VK_CONTROL );
	key_map( Key::KEY_ALT, VK_MENU );
	key_map( Key::KEY_SPACE, VK_SPACE );
	key_map( Key::KEY_ENTER, VK_RETURN );
	key_map( Key::KEY_BACK_SPACE, VK_BACK );
	key_map( Key::KEY_DELETE, VK_DELETE );
	key_map( Key::KEY_F1, VK_F1 );
	key_map( Key::KEY_F2, VK_F2 );
	key_map( Key::KEY_F3, VK_F3 );
	key_map( Key::KEY_F4, VK_F4 );
	key_map( Key::KEY_F5, VK_F5 );
	key_map( Key::KEY_F6, VK_F6 );
	key_map( Key::KEY_F7, VK_F7 );
	key_map( Key::KEY_F8, VK_F8 );
	key_map( Key::KEY_F9, VK_F9 );
	key_map( Key::KEY_F10, VK_F10 );
	key_map( Key::KEY_F11, VK_F11 );
	key_map( Key::KEY_F12, VK_F12 );
	key_map( Key::KEY_LEFT, VK_LEFT );
	key_map( Key::KEY_RIGHT, VK_RIGHT );
	key_map( Key::KEY_UP, VK_UP );
	key_map( Key::KEY_DOWN, VK_DOWN );
	key_map( Key::KEY_PAGE_UP, VK_PRIOR );
	key_map( Key::KEY_PAGE_DOWN, VK_NEXT );
	key_map( Key::KEY_HOME, VK_HOME );
	key_map( Key::KEY_END, VK_END );
	key_map( Key::KEY_INSERT, VK_INSERT );
}

void Keyboard::key_map( int key, int win_key )
{
	key_map_.insert( std::make_pair( key, win_key ) );
}

} // namespace win

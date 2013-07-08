// *********************************************************
// Keyboard.h
// copyright (c) JE all rights reserved
// 2003/2/4
// *********************************************************


#ifndef WIN_KEYBOARD_H
#define WIN_KEYBOARD_H

#include <driver/Keyboard.h>
#include <map>

namespace win
{

class Keyboard : public driver::Keyboard
{
public:
	static const int MAX_KEY = 256;
	static std::map< int, int > key_map_;
	static void create_key_map();
	static void key_map( int, int );

private:
	State state_[ MAX_KEY ];
public:
	Keyboard();
	virtual ~Keyboard();

	virtual void update();
	virtual const State key( Keys ) const;

}; // class Keyboard

} // namespace win

#endif // WIN_KEYBOARD_H

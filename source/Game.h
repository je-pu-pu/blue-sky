#ifndef GAME_H
#define GAME_H

#include <type/type.h>

/**
 * ’ŠÛƒQ[ƒ€ƒNƒ‰ƒX
 *
 * @todo game ‚ÉˆÚ“®‚·‚é
 */
class Game
{
public:

protected:
	Game() { }

public:
	static Game* get_instance();

	virtual ~Game() { }

	virtual bool update() = 0;

	virtual void on_key_down( char_t ) =0;
	virtual void on_function_key_down( int ) = 0;
	virtual void on_mouse_wheel( int ) = 0;
	virtual void on_resize() = 0;

	virtual int get_width() const = 0;
	virtual int get_height() const = 0;
};

#endif // GAME_H
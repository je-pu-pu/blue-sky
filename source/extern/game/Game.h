#pragma once

#include <type/type.h>

namespace game
{

/**
 * ’ŠÛƒQ[ƒ€ƒNƒ‰ƒX
 *
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
	virtual void on_special_key_down( int ) = 0;
	virtual void on_mouse_wheel( int ) = 0;
	virtual void on_resize() = 0;

	virtual int get_width() const = 0;
	virtual int get_height() const = 0;

	virtual bool is_show_cursor() const = 0;
};

} // namespace game

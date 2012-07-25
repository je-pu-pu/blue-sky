#ifndef GAME_H
#define GAME_H

#include "App.h"

class Game
{
public:

protected:
	Game() { }

public:
	static Game* getInstance();

	virtual ~Game() { }

	virtual bool update() = 0;

	inline App* get_app() const { return App::GetInstance(); }

	inline int get_width() const { return get_app()->get_width(); }
	inline int get_height() const { return get_app()->get_height(); }
};

#endif // GAME_H
#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "Game.h"

class Direct3D11;

class GameMain : public Game
{
protected:
	Direct3D11*	direct_3d_;		///< Direct3D

public:
	GameMain();
	~GameMain();

	bool update();
	void render();
};

inline Game* Game::getInstance() { static GameMain game_main; return & game_main; }

#endif // GAME_MAIN_H
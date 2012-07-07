#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "Game.h"

class Direct3D11;
class BulletPhysics;

class DirectInput;

namespace game
{
	class Config;
};

namespace blue_sky
{
	class Input;

	class ActiveObject;
	class ActiveObjectManager;
};

using namespace game;
using namespace blue_sky;

class GameMain : public Game
{
protected:
	Direct3D11*				direct_3d_;				///< Direct3D
	BulletPhysics*			physics_;				///< Bullet

	DirectInput*			direct_input_;			///< DirectInput
	Input*					input_;					///< Game Input

	Config*					config_;				///< Config
	Config*					save_data_;				///< Save Data

	ActiveObjectManager*	active_object_manager_;	///< ActiveObjectManager

	void render( const ActiveObject* );

public:
	GameMain();
	virtual ~GameMain();

	bool update();
	void render();
};

inline Game* Game::getInstance() { static GameMain game_main; return & game_main; }

#endif // GAME_MAIN_H
#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "Game.h"
#include <common/auto_ptr.h>

class Direct3D11;
class Direct3D11ConstantBuffer;
class Direct3D11BulletDebugDraw;

class DirectWrite;

class DirectInput;

class BulletPhysics;


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
using common::auto_ptr;

class GameMain : public Game
{
protected:
	auto_ptr< Direct3D11 >		direct_3d_;				///< Direct3D
	auto_ptr< BulletPhysics >	physics_;				///< Bullet

	auto_ptr< DirectInput >		direct_input_;			///< DirectInput
	auto_ptr< Input >			input_;					///< Game Input

	auto_ptr< Config >			config_;				///< Config
	auto_ptr< Config >			save_data_;				///< Save Data

	auto_ptr< ActiveObjectManager >			active_object_manager_;	///< ActiveObjectManager

	auto_ptr< Direct3D11ConstantBuffer >	game_constant_buffer_;
	auto_ptr< Direct3D11ConstantBuffer >	frame_constant_buffer_;
	auto_ptr< Direct3D11ConstantBuffer >	object_constant_buffer_;

	auto_ptr< DirectWrite >					direct_write_;
	auto_ptr< Direct3D11BulletDebugDraw >	bullet_debug_draw_;

	void render( const ActiveObject* );
	void render_line( const ActiveObject* );

public:
	GameMain();
	virtual ~GameMain();

	bool update();
	void render();
};

inline Game* Game::getInstance() { static GameMain game_main; return & game_main; }

#endif // GAME_MAIN_H
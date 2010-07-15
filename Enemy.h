#ifndef BLUE_SKY_ENEMY_H
#define BLUE_SKY_ENEMY_H

#include "ActiveObject.h"

namespace game
{

class Sound;
template< typename > class AABB;

} // namespace game

namespace blue_sky
{

class Input;
class Stage;
class GridCell;

/**
 * ìG
 *
 */
class Enemy : public ActiveObject
{
public:

private:

public:
	Enemy();
	~Enemy() { }

	/// çXêV
	void update();
	
}; // class Enemy

} // namespace blue_sky

#endif // BLUE_SKY_ENEMY_H

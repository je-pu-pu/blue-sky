#ifndef BLUE_SKY_ENEMY_H
#define BLUE_SKY_ENEMY_H

#include "ActiveObject.h"

namespace blue_sky
{

class Player;

/**
 * ìG
 *
 */
class Enemy : public ActiveObject
{
public:
	enum Mode
	{
		MODE_FIND = 0,
		MODE_CHASE,
		MODE_DETOUR
	};

private:
	const Player* player_;

	Mode mode_;
	int counter_;

protected:
	float get_collision_width() const { return 0.4f; }
	float get_collision_height() const { return 1.5f; }
	float get_collision_depth() const  { return 0.4f; }

	void on_collision_x( const GridCell& );
	void on_collision_y( const GridCell& );
	void on_collision_z( const GridCell& );

public:
	Enemy();
	~Enemy() { }

	void set_player( const Player* p ) { player_ = p; }

	/// çXêV
	void update();
	
}; // class Enemy

} // namespace blue_sky

#endif // BLUE_SKY_ENEMY_H

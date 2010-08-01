#ifndef BLUE_SKY_ROCKET_H
#define BLUE_SKY_ROCKET_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ���P�b�g
 *
 */
class Rocket : public ActiveObject
{
private:
	void on_collision_x( const GridCell& ) { }
	void on_collision_y( const GridCell& ) { }
	void on_collision_z( const GridCell& ) { }

public:
	Rocket();
	~Rocket() { }

	/// �X�V
	void update();

	void restart();
	
}; // class Rocket

} // namespace blue_sky

#endif // BLUE_SKY_ROCKET_H

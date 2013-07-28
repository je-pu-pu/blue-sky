#ifndef BLUE_SKY_LADDER_H
#define BLUE_SKY_LADDER_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ’òŽq
 *
 */
class Ladder : public ActiveObject
{
public:

private:
	bool is_hard() const { return false; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }

public:
	Ladder() { }
	~Ladder() { }
	
	float_t get_collision_width() const { return 0.4f; }
	float_t get_collision_height() const { return 5.f; }
	float_t get_collision_depth() const { return 0.2f; }

	void restart() { ActiveObject::restart(); }

}; // class Balloon

} // namespace blue_sky

#endif // BLUE_SKY_BALLOON_H

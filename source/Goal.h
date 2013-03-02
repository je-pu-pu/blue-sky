#ifndef BLUE_SKY_GOAL_H
#define BLUE_SKY_GOAL_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ÉSÅ[ÉãÇÃî‡
 *
 */
class Goal : public ActiveObject
{
public:


private:
	float get_collision_width() const { return 4.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 4.f; }

	void on_collide_with( ActiveObject* o ) { }

public:
	Goal();
	~Goal() { }

	void restart();

	/// çXêV
	void update();
	
}; // class Goal

} // namespace blue_sky

#endif // BLUE_SKY_GOAL_H

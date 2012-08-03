#ifndef BLUE_SKY_GOAL_H
#define BLUE_SKY_GOAL_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ƒS[ƒ‹‚Ì”à
 *
 */
class Goal : public ActiveObject
{
public:


private:

	float get_collision_width() const { return 4.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 4.f; }

public:
	Goal();
	~Goal() { }

	void restart();

	/// XV
	void update();
	
}; // class Goal

} // namespace blue_sky

#endif // BLUE_SKY_GOAL_H

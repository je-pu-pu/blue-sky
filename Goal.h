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
	void on_collision_x( const GridCell& ) { }
	void on_collision_y( const GridCell& ) { }
	void on_collision_z( const GridCell& ) { }

	float get_collision_width() const { return 0.f; }
	float get_collision_height() const { return 0.f; }
	float get_collision_depth() const { return 0.f; }

public:
	Goal();
	~Goal() { }

	/// çXêV
	void update();
	
}; // class Goal

} // namespace blue_sky

#endif // BLUE_SKY_GOAL_H

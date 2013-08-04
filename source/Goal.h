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
	float get_collision_width() const override { return 4.f; }
	float get_collision_height() const override { return 2.f; }
	float get_collision_depth() const override { return 4.f; }

	const Vector3& get_default_gravity() const override { return GravityZero; }

	void on_collide_with( GameObject* o ) override { }

public:
	Goal();
	~Goal() { }

	void restart();

	/// çXêV
	void update();
	
}; // class Goal

} // namespace blue_sky

#endif // BLUE_SKY_GOAL_H

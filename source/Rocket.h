#ifndef BLUE_SKY_ROCKET_H
#define BLUE_SKY_ROCKET_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ロケット
 *
 */
class Rocket : public ActiveObject
{
private:
	float get_collision_width() const { return 3.f; }
	float get_collision_height() const { return 3.f; }
	float get_collision_depth() const { return 3.f; }

public:
	Rocket();
	~Rocket() { }

	/// 更新
	void update();

	void restart();
	
}; // class Rocket

} // namespace blue_sky

#endif // BLUE_SKY_ROCKET_H

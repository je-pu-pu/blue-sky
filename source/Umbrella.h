#ifndef BLUE_SKY_UMBRELLA_H
#define BLUE_SKY_UMBRELLA_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * �P
 *
 */
class Umbrella : public ActiveObject
{
public:
	float flicker_;

private:
	float get_collision_width() const { return 2.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 2.f; }

public:
	Umbrella();
	~Umbrella() { }

	/// �X�V
	void update();

	void restart();
	
}; // class Umbrella

} // namespace blue_sky

#endif // BLUE_SKY_UMBRELLA_H

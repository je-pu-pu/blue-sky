#ifndef BLUE_SKY_ROBOT_H
#define BLUE_SKY_ROBOT_H

#include "ActiveObject.h"

namespace blue_sky
{

class Player;

/**
 * ìG
 *
 */
class Robot : public ActiveObject
{
public:

private:

protected:

public:
	Robot();
	~Robot() { }

	/// çXêV
	void update();
	
	float get_collision_width() const { return 4.f; }
	float get_collision_height() const { return 8.f; }
	float get_collision_depth() const  { return 2.f; }

}; // class Robot

} // namespace blue_sky

#endif // BLUE_SKY_ROBOT_H

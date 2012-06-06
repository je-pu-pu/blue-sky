#ifndef BLUE_SKY_POISON_H
#define BLUE_SKY_POISON_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ì≈ñÚ
 *
 */
class Poison : public ActiveObject
{
public:

private:
	void on_collision_x( const GridCell& ) { }
	void on_collision_y( const GridCell& ) { }
	void on_collision_z( const GridCell& ) { }

	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 1.f; }

public:
	Poison();
	~Poison() { }

	/// çXêV
	void update();

	void restart();
	
}; // class Poison

} // namespace blue_sky

#endif // BLUE_SKY_POISON_H

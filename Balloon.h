#ifndef BLUE_SKY_BALLOON_H
#define BLUE_SKY_BALLOON_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ïóëD
 *
 */
class Balloon : public ActiveObject
{
public:
	float flicker_;

private:
	void on_collision_x( const GridCell& ) { }
	void on_collision_y( const GridCell& ) { }
	void on_collision_z( const GridCell& ) { }

	float get_collision_width() const { return 0.4f; }
	float get_collision_height() const { return 0.4f; }
	float get_collision_depth() const { return 0.4f; }

public:
	Balloon();
	~Balloon() { }

	/// çXêV
	void update();
	
}; // class Balloon

} // namespace blue_sky

#endif // BLUE_SKY_BALLOON_H
